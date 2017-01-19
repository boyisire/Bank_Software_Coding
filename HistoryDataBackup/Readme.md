## 程序设计
>数据移值工具
	通过参数控制：
	-DayEnd	日终（通过读表进行处理）
	-h
	-...

### 主要目录
+ etc	配置文件
+ bin	可执行程序
+ src	源代码
+ inc	头文件
+ sql	常用SQL
+ log	日志文件

### src 主要文件：
+ <1>DT_Main.c          主程序
+ <2>DT_Proc_DayEnd.sqc	日终操作处理模块
+ <3>DT_Func_Db.sqc	    常用数据库操作库
+ <4>DT_Proc_Mon.c	    监控处理模块
+ <5>DT_Func_Pub.c	    常用公共函数库

### 头文件
+ DayEnd.h
+ DT_Db_Pub.h
+ DT_Pub.h
