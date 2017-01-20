

/* 
function name :swisShmNonAttachProc 
input: shmid
function: check if share memory has no process attached,
          and if YES, release the share memory
函数功能：释放指定shmid的共享内存
参数    ：shmid 共享内存id
返回值  ：-1 ： 发生错误
          -2 :  不可以删除
           0 ： 释放成功
作者    ：王浩
时间    ：20150312
*/
int   swReleaseZeroAttachedShm(int shmid)
{
    struct shmid_ds buf;
    int ilRc;

    ilRc = shmctl(shmid, IPC_STAT, &buf);
    if(ilRc < 0)
    {
        swVdebug(0,"读取shmid[%d]的状态错误！\n",shmid);
        return -1;
    }

    if(0 == buf.shm_nattch)
    {
        ilRc = shmctl(shmid, IPC_RMID, NULL);
        if(ilRc < 0)
        {
            swVdebug(0,"删除指定的shmid[%d]所指定的共享内存错误！", shmid);
            return -1;
        }
    }
    else
    {
        swVdebug(0,"指定的shmid[%d]所指向的共享内存还有[%d]个进程链接，不能释放!",shmid, buf.shm_nattch);
        return -2;
    }


    return 0;
}


