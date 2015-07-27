#!bin/bash

#删除历史日终日志文件
find $HISDAYENDPATH/log -mtime +5|xargs rm -f

#删除历史日终数据文件
find $HISDAYENDPATH/dmp -mtime +5|xargs rm -f
