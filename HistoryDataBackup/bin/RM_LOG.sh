#!bin/bash

#ɾ����ʷ������־�ļ�
find $HISDAYENDPATH/log -mtime +5|xargs rm -f

#ɾ����ʷ���������ļ�
find $HISDAYENDPATH/dmp -mtime +5|xargs rm -f
