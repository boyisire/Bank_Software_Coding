do=`ps -ef |grep bmqGrp_rcv_comm|grep -v grep |awk -F " " '{print $2}'`
kill -9 $do

do=`ps -ef |grep bmqGrp_rcv_front|grep -v grep |awk -F " " '{print $2}'`
kill -9 $do
