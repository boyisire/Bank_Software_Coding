#!/usr/bin/sh

. ./ShellFunc.lib
if [ $# -lt 1 ]
then
	echo "∏Ò Ω:$0 MONERRO"
	exit
else
	Str_MonErro="$1"
fi
SQL_STR="select ID||'|'|| TABNAME||'|'|| TABNAMEDESC||'|'|| FROMUSER||'|'|| FROMPASS||'|'|| FROMLINK||'|'|| TOUSER||'|'|| TOPASS||'|'|| TOLINK||'|'|| OPERFLAG||'|'|| KEEPDATE||'|'|| CREATEDATE||'|'|| DATECLOUMN||'|'|| DATEFORMAT||'|'|| EXTENDSIZE||'|'|| EXECSHELL||'|'|| MONFLAG||'|'|| MONERRO||'|'|| MONCODE||'|'|| MONDESC||'|'|| STATUS||'|'|| OPERDATE||'|'|| NOTES from ch_dayend_cfg where MONERRO='${Str_MonErro}';"

Str=`Fun_DB "chmap3/qdglpt@arptdbs" "${SQL_STR}"`
echo $Str|awk -F "|" '{printf "[%s]\n",$2}'
echo $Str|awk -F "|" '{printf "TAB_ID=%s\n",$1}'
echo $Str|awk -F "|" '{printf "TAB_TABNAME=%s\n",$2}'
echo $Str|awk -F "|" '{printf "TAB_TABNAMEDESC=%s\n",$3}'
echo $Str|awk -F "|" '{printf "TAB_FROMUSER=%s\n",$4}'
echo $Str|awk -F "|" '{printf "TAB_FROMPASS=%s\n",$5}'
echo $Str|awk -F "|" '{printf "TAB_FROMLINK=%s\n",$6}'
echo $Str|awk -F "|" '{printf "TAB_TOUSER=%s\n",$7}'
echo $Str|awk -F "|" '{printf "TAB_TOPASS=%s\n",$8}'
echo $Str|awk -F "|" '{printf "TAB_TOLINK=%s\n",$9}'
echo $Str|awk -F "|" '{printf "TAB_OPERFLAG=%s\n",$10}'
echo $Str|awk -F "|" '{printf "TAB_KEEPDATE=%s\n",$11}'
echo $Str|awk -F "|" '{printf "TAB_CREATEDATE=%s\n",$12}'
echo $Str|awk -F "|" '{printf "TAB_DATECLOUMN=%s\n",$13}'
echo $Str|awk -F "|" '{printf "TAB_DATEFORMAT=%s\n",$14}'
echo $Str|awk -F "|" '{printf "TAB_EXTENDSIZE=%s\n",$15}'
echo $Str|awk -F "|" '{printf "TAB_EXECSHELL=%s\n",$16}'
echo $Str|awk -F "|" '{printf "TAB_MONFLAG=%s\n",$17}'
echo $Str|awk -F "|" '{printf "TAB_MONERRO=%s\n",$18}'
echo $Str|awk -F "|" '{printf "TAB_MONCODE=%s\n",$19}'
echo $Str|awk -F "|" '{printf "TAB_MONDESC=%s\n",$20}'
echo $Str|awk -F "|" '{printf "TAB_STATUS=%s\n",$21}'
echo $Str|awk -F "|" '{printf "TAB_OPERDATE=%s\n",$22}'
