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
name = "�³�����ƽ̨����̨����ϵͳ"

[�����ڴ����]
type = 1
function = "sysshmmng.mu"

[�������]
type = 1
function = "systask.mu"

[�������]
type = 1
function = "sysmail.mu"

[�˿ڹ���]
type = 1
function = "sysport.mu"

[������ˮ����]
type = 1
function = "syslog.mu"

[Saf����]
type = 1
function = "syssaf.mu"

[���������]
type = 2
function = "swBatch"
#parameter = "20011127 140000"
#parmdesc = "������ͳ��ʱ��(yyyymmdd);������ͳ��ʱ��(hhmmss);"
#parmlen = "8;6;"
iocode = "swbatch"
win_type = "1"

[�˳�]
type = 9
