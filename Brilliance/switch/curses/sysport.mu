[attribute]
name = "端口管理"

[查看端口状态]
type = 3	#Funcation Call
function = "swListport()"

[强置端口状态UP]
type = 3	#Funcation Call
function = "swConup()"
iocode = "swconup"
win_type = "1"

[强置端口状态DOWN]
type = 3	#Funcation Call
function = "swCondown()"
iocode = "swcondown"
win_type = "1"

[置端口状态]
type = 3	#Funcation Call
function = "swConset()"
iocode = "swconset"
win_type = "1"
