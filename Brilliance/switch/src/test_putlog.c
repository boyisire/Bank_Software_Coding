#include "switch.h"

#define TEST_COUNT 1024*1024
long aoff[TEST_COUNT];
char filename[TEST_COUNT][16];
int main()
{
	short len;
	long offset;
	char pack[1024*2];
	int i;
	int rt;
        strcpy(agDebugfile, "test_log");
	strcpy(pack, "helle world");
	swVdebug(1, "start putlog....");
	for(i=0; i< TEST_COUNT; i++)
	{
		if(i%2)
		{
			rt = swPutlog(pack, 1024*2, &aoff[i], filename[i]);
		}
		else
		{
			rt = swPutlog(pack, 1024, &aoff[i], filename[i]);
		}
		if(rt != 0)  { printf("error\n"); exit(-1);}
	}
	swVdebug(1, "start getlog....");
	for(i=0; i< TEST_COUNT; i++)
	{
		memset(pack, 0x00, sizeof(pack));
		if(i%2)
		{
			if ( 0 != swGetlog(filename[i], 1024*2, aoff[i], pack) )
			{
				printf("error\n");
				exit(-1);
			}
		}
		else
		{
			if ( 0 != swGetlog(filename[i], 1024, aoff[i], pack) )
			{
				printf("error\n");
				exit(-1);
			}
		
		}
		if(pack[0] != 'h')
		{
			printf("error\n");
			exit(-1);
		}
	}
	swVdebug(1, "end ....");
	return 0;
}
