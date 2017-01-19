#!/usr/bin/sh

#加载公共函数
. ./ShellFunc.lib

if [ $# -lt 1 ]
then
	echo "格式:$0 数据库用户名 数据库密码 数据库连接串 表名"
	exit
else
	DBUSER="$1"
	DBPASS="$2"
	DBLINK="$3"
	TabName="$4"
fi

DB_ConnStr="${DBUSER}/${DBPASS}@${DBLINK}"
SQL_Str1="select to_char(sysdate,'DD') from dual;"
SQL_Str2="select to_char(sysdate,'YYYYMM') from dual;"

SQL_DT1=`Fun_DB "$DBUSER/${DBPASS}@${DBLINK}" "${SQL_Str1}"`
SQL_DT2=`Fun_DB "$DBUSER/${DBPASS}@${DBLINK}" "${SQL_Str2}"`


if [ ${SQL_DT1} == "01" ]
then
	SQL_Str3="Rename ${TabName} to ${TabName}_${SQL_DT2};"
else
	echo "ZZZZZZZ"
fi
echo $SQL_DT1
echo $SQL_DT2
echo $SQL_Str3


echo $SQL_DT1
