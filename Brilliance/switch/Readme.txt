版本更新：
1.更新双共享内存机制，可以swInit的同时发送交易
2.添加一个工具swShmcheckTool查看静态共享内存中配置文件，并写入到SWITCH_CHECK的环境变量指定目录，或者写入/tmp目录下
3.交易流量控制，添加一个配置文件在etc/config/SWTRAN_CTL.CFG,在其中的配置交易的最大数量要小于SWAPPS.CFG中TRANBEGIN_MAX的值
4.耗时统计，在报文头里面添加了三个字段

新增修改：
1.修改部分int为long，格式转换组号的长度问题。


20150326新增：
1.增加邓宇巍修改的部分
2.增加陈继辉修改的部分


20150401修改：
1.修改日志级别问题，修改文件名行号问题
2.修改当前置进程日志级别为0时主控退出问题
3.修改PtIvrXyk的日志部分

20150402修改：
1.修改日志中不打印进程号问题

20150408修改：
1.修改swConstant.h中iEXPRESSLEN的值
2.修改swapi.c中swSavetranhead函数中memcpy的大小

20150410修改：
1.添加swInit -f工具，清除静态共享内存
2.在swShutdown.c中添加清除静态共享内存和删除shmid.txt文件
3.修改swShmview.c
4.修改swShmapi.c，添加相关静态共享内存的函数

5.修改pt，使PtCreditCOmm日志打印出来
6.增加日志线程安全版本
7.修改Pt中相关msgrcv函数返回值的类型问题

20150413修改：
修改格式转换升级，调用TDF（递归）时出错的问题，交易结果返回“格式转换未知错误”
