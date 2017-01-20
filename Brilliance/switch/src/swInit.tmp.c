

/* 
function name :swisShmNonAttachProc 
input: shmid
function: check if share memory has no process attached,
          and if YES, release the share memory
�������ܣ��ͷ�ָ��shmid�Ĺ����ڴ�
����    ��shmid �����ڴ�id
����ֵ  ��-1 �� ��������
          -2 :  ������ɾ��
           0 �� �ͷųɹ�
����    ������
ʱ��    ��20150312
*/
int   swReleaseZeroAttachedShm(int shmid)
{
    struct shmid_ds buf;
    int ilRc;

    ilRc = shmctl(shmid, IPC_STAT, &buf);
    if(ilRc < 0)
    {
        swVdebug(0,"��ȡshmid[%d]��״̬����\n",shmid);
        return -1;
    }

    if(0 == buf.shm_nattch)
    {
        ilRc = shmctl(shmid, IPC_RMID, NULL);
        if(ilRc < 0)
        {
            swVdebug(0,"ɾ��ָ����shmid[%d]��ָ���Ĺ����ڴ����", shmid);
            return -1;
        }
    }
    else
    {
        swVdebug(0,"ָ����shmid[%d]��ָ��Ĺ����ڴ滹��[%d]���������ӣ������ͷ�!",shmid, buf.shm_nattch);
        return -2;
    }


    return 0;
}


