#include "sqlca.h"

/*º¯ÊýÉùÃ÷*/
extern int DB_Connect(char *DB_User, char *DB_Pass, char *DB_Link);
extern void DB_Commit();
extern void DB_Rollback();
extern int DB_IsExistTable(char *DB_User, char *DB_Pass, char *DB_Link,char *DB_Table);
extern int DB_IsExistTabPart(char *DB_User, char *DB_Pass, char *DB_Link,char *DB_Table,char *DB_TablePart);
extern int DB_SysDate_Count(char *DB_User, char *DB_Pass, char *DB_Link,int DayNum);
extern int DB_SqlExec(char *Buf);
extern int DB_GetTabSpaceName(char *DB_User, char *DB_Pass, char *DB_Link,char *DB_Table);
