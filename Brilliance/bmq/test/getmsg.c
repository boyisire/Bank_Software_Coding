#include "bmq.h"

struct igMail
{
    int ilSourq;
    int ilDesq;
};
//#define MAXLEN 40000
#define MAXLEN 65536
int main(int argc, char **argv)
{
    int ilRc, interval ;
    long max_count, i;
    unsigned int ilDesq,ilSourq,ilGrpid,ilQid,ilPrior,ilMsglen;
    struct timeval etime, stime;

    long llType,llClass;
    char alBufget[MAXLEN];

    if(argc != 3)
    {
        printf("usage: get source target\n");
        exit(0);
    }

    ilDesq = atoi(argv[1]);
    max_count = atoi(argv[2]);

//   fork();

   // printf("in atoi ilDesq is :%d\n",ilDesq);
    ilRc = bmqOpen(ilDesq);
    if(ilRc)
    {
        printf("open error!\n");
        exit(0);
    }
    //printf("befor get ilDesq is:%d\n",ilDesq);
        llType = 12;
        llClass = 12;
        ilGrpid = 0;
        ilQid = 0;
        gettimeofday(&stime, NULL);
        printf("\n.................................\n");
        printf("Start time=%ld\n",(stime.tv_sec*1000) + (stime.tv_usec/1000));
        for(i = 0; i< max_count; i++ )
	{

        	ilRc = bmqGetw(&ilGrpid,&ilQid,&ilPrior,&llType,&llClass,alBufget,&ilMsglen, 0);
                if(i == 0 )
		{
                	gettimeofday(&stime, NULL);
                	printf("Start time=%ld\n",(stime.tv_sec*1000) + (stime.tv_usec/1000));
		}
        	if(ilRc)
        	{	
            		printf("get error!\n");
            		printf("ilRc is:%d\n",ilRc);
            		bmqClose();
        	}
       }
       gettimeofday(&etime, NULL);
       printf("end time=%ld\n",(etime.tv_sec*1000) + (etime.tv_usec/1000) );
       printf("total time = %ld", (etime.tv_sec*1000) + (etime.tv_usec/1000) - stime.tv_sec*1000 - stime.tv_usec/1000 );
       fflush(stdout);
       bmqClose();
       return 0;
}

