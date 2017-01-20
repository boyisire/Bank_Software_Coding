[attribute]
name = "任务管理"

[显示任务]
type = 3	#Funcation Call
function = "swListtask()"

[启动任务]
type = 3	#Funcation Call
function = "swStarttask()"
#parmdesc = "请输入要启动的任务名称;请输入您的姓名;年龄;"
#parmlen = "20;10;4;"
iocode = "start_task"
win_type = "1"

[停止任务]
type = 3	#Funcation Call
function = "swStoptask()"
#parmdesc = "请输入要关闭的任务名称;"
#parmlen = "20;"
iocode = "stop_task"
win_type = "1"

