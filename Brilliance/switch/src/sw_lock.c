/*-------------------------------------------------------------------------
 *
 * sw_lock.c
 *	   Hardware-dependent implementation of spinlocks.
 *
 *
 *
 *
 *-------------------------------------------------------------------------
*/
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include "sw_lock.h"
#include <unistd.h>
#define Max(x, y)                               ((x) > (y) ? (x) : (y))
#define Min(x, y)                               ((x) < (y) ? (x) : (y))


unsigned long wait_count=0;
unsigned long lock_count=0;

#define MAX_RANDOM_VALUE  0x7FFFFFFF
slock_t		dummy_spinlock;

static int	spins_per_delay = DEFAULT_SPINS_PER_DELAY;

void sw_usleep(long microsec)
{
        if (microsec > 0)
        {
                struct timeval delay;

                delay.tv_sec = microsec / 1000000L;
                delay.tv_usec = microsec % 1000000L;
                (void) select(0, NULL, NULL, NULL, &delay);
        }
}


/*
 * s_lock_stuck() - complain about a stuck spinlock
 */
static void
s_lock_stuck(volatile slock_t *lock, const char *file, int line)
{
#if defined(S_LOCK_TEST)
	fprintf(stderr,
			"\nStuck spinlock (%p) detected at %s:%d.\n",
			lock, file, line);
	exit(1);
#else
	printf("stuck spinlock (%p) detected at %s:%d",
		 lock, file, line);
#endif
}


/*
 * s_lock(lock) - platform-independent portion of waiting for a spinlock.
 */
int
s_lock(volatile slock_t *lock, const char *file, int line)
{
#define MIN_SPINS_PER_DELAY 10
#define MAX_SPINS_PER_DELAY 1000
#define NUM_DELAYS			1000
#define MIN_DELAY_USEC		1000L
#define MAX_DELAY_USEC		1000000L

	int			spins = 0;
	int			delays = 0;
	int			cur_delay = 0;
    lock_count++;
	while (TAS_SPIN(lock))
	{       
        wait_count++;
		/* CPU-specific delay each time through the loop */
		SPIN_DELAY();

		/* Block the process every spins_per_delay tries */
		if (++spins >= spins_per_delay)
		{
			if (++delays > NUM_DELAYS)
				s_lock_stuck(lock, file, line);

			if (cur_delay == 0) /* first time to delay? */
				cur_delay = MIN_DELAY_USEC;

			sw_usleep(cur_delay);

#if defined(S_LOCK_TEST)
			fprintf(stdout, "*");
			fflush(stdout);
#endif

			/* increase delay by a random fraction between 1X and 2X */
			cur_delay += (int) (cur_delay *
					  ((double) random() / (double) MAX_RANDOM_VALUE) + 0.5);
			/* wrap back to minimum delay when max is exceeded */
			if (cur_delay > MAX_DELAY_USEC)
				cur_delay = MIN_DELAY_USEC;

			spins = 0;
		}
	}

	if (cur_delay == 0)
	{
		/* we never had to delay */
		if (spins_per_delay < MAX_SPINS_PER_DELAY)
			spins_per_delay = Min(spins_per_delay + 100, MAX_SPINS_PER_DELAY);
	}
	else
	{
		if (spins_per_delay > MIN_SPINS_PER_DELAY)
			spins_per_delay = Max(spins_per_delay - 1, MIN_SPINS_PER_DELAY);
	}
	return delays;
}


/*
 * Set local copy of spins_per_delay during backend startup.
 *
 * NB: this has to be pretty fast as it is called while holding a spinlock
 */
void
set_spins_per_delay(int shared_spins_per_delay)
{
	spins_per_delay = shared_spins_per_delay;
}

/*
 * Update shared estimate of spins_per_delay during backend exit.
 *
 * NB: this has to be pretty fast as it is called while holding a spinlock
 */
int
update_spins_per_delay(int shared_spins_per_delay)
{
	return (shared_spins_per_delay * 15 + spins_per_delay) / 16;
}




#ifdef HAVE_SPINLOCKS			/* skip spinlocks if requested */


#if defined(__GNUC__)



#if defined(__m68k__) && !defined(__linux__)
/* really means: extern int tas(slock_t* **lock); */
static void
tas_dummy()
{
	__asm__		__volatile__(
#if defined(__NetBSD__) && defined(__ELF__)
/* no underscore for label and % for registers */
										 "\
.global		tas 				\n\
tas:							\n\
			movel	%sp@(0x4),%a0	\n\
			tas 	%a0@		\n\
			beq 	_success	\n\
			moveq	#-128,%d0	\n\
			rts 				\n\
_success:						\n\
			moveq	#0,%d0		\n\
			rts 				\n"
#else
										 "\
.global		_tas				\n\
_tas:							\n\
			movel	sp@(0x4),a0	\n\
			tas 	a0@			\n\
			beq 	_success	\n\
			moveq 	#-128,d0	\n\
			rts					\n\
_success:						\n\
			moveq 	#0,d0		\n\
			rts					\n"
#endif   /* __NetBSD__ && __ELF__ */
	);
}
#endif   /* __m68k__ && !__linux__ */
#endif   /* not __GNUC__ */
#endif   /* HAVE_SPINLOCKS */



/*****************************************************************************/
#if defined(S_LOCK_TEST)

/*
 * test program for verifying a port's spinlock support.
 */

struct test_lock_struct
{
	char		pad1;
	slock_t		lock;
	char		pad2;
};

volatile struct test_lock_struct test_lock;

int
main()
{
	srandom((unsigned int) time(NULL));

	test_lock.pad1 = test_lock.pad2 = 0x44;

	S_INIT_LOCK(&test_lock.lock);

	if (test_lock.pad1 != 0x44 || test_lock.pad2 != 0x44)
	{
		printf("S_LOCK_TEST: failed, declared datatype is wrong size\n");
		return 1;
	}

	if (!S_LOCK_FREE(&test_lock.lock))
	{
		printf("S_LOCK_TEST: failed, lock not initialized\n");
		return 1;
	}

	S_LOCK(&test_lock.lock);

	if (test_lock.pad1 != 0x44 || test_lock.pad2 != 0x44)
	{
		printf("S_LOCK_TEST: failed, declared datatype is wrong size\n");
		return 1;
	}

	if (S_LOCK_FREE(&test_lock.lock))
	{
		printf("S_LOCK_TEST: failed, lock not locked\n");
		return 1;
	}

	S_UNLOCK(&test_lock.lock);

	if (test_lock.pad1 != 0x44 || test_lock.pad2 != 0x44)
	{
		printf("S_LOCK_TEST: failed, declared datatype is wrong size\n");
		return 1;
	}

	if (!S_LOCK_FREE(&test_lock.lock))
	{
		printf("S_LOCK_TEST: failed, lock not unlocked\n");
		return 1;
	}

	S_LOCK(&test_lock.lock);

	if (test_lock.pad1 != 0x44 || test_lock.pad2 != 0x44)
	{
		printf("S_LOCK_TEST: failed, declared datatype is wrong size\n");
		return 1;
	}

	if (S_LOCK_FREE(&test_lock.lock))
	{
		printf("S_LOCK_TEST: failed, lock not re-locked\n");
		return 1;
	}

	printf("S_LOCK_TEST: this will print %d stars and then\n", NUM_DELAYS);
	printf("             exit with a 'stuck spinlock' message\n");
	printf("             if S_LOCK() and TAS() are working.\n");
	fflush(stdout);

	s_lock(&test_lock.lock, __FILE__, __LINE__);

	printf("S_LOCK_TEST: failed, lock not locked\n");
	return 1;
}


#endif   /* S_LOCK_TEST */
