###########################################################
# Menu Items Type:
#	0 --- SUB_MENU
#	1 --- POP_MENU
#	2 --- Execute Unix Shell Commands
#	3 --- Call Routines
#	8 --- Clear screen
#	9 --- Exit menu
###########################################################

[ATTRIBUTE]
name = "新晨交换平台主控台管理系统"

[共享内存管理]
type = 1
function = "sysshmmng.mu"

[任务管理]
type = 1
function = "systask.mu"

[邮箱管理]
type = 1
function = "sysmail.mu"

[端口管理]
type = 1
function = "sysport.mu"

[交易流水管理]
type = 1
function = "syslog.mu"

[Saf管理]
type = 1
function = "syssaf.mu"

[批处理管理]
type = 2
function = "swBatch"
#parameter = "20011127 140000"
#parmdesc = "请输入统计时间(yyyymmdd);请输入统计时间(hhmmss);"
#parmlen = "8;6;"
iocode = "swbatch"
win_type = "1"

[退出]
type = 9
