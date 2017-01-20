/*add by zcd 20141220*/
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/parserInternals.h>
#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/debugXML.h>
#include <libxml/xmlerror.h>
#include <libxml/xinclude.h>
#include <libxml/catalog.h>
#include <libxml/DOCBparser.h>
#include <libxml/globals.h>

struct swxml_context
{
  xmlDocPtr	doc; 
  xmlNodePtr	node; 
};
typedef struct swxml_context   XMLContext;
typedef struct swxml_context*  XMLContextPtr;

void XMLFree();

/*delete by zcd 20141218
int XMLInit( char*);
***end of delete by zcd 20141218*/
 
/*add by zcd 20141218*/
int XMLInit(char *aVersion,char *aEncoding,char *root);
/*end of add by zcd 20141218*/

int XMLInitFromBuf( char *);
int XMLInitFromFile( char *);

int XMLDumpToBuf( char *, int);
int XMLDumpToFile( char *, int);

xmlNodePtr XMLGetNode( char *, int);
xmlNodePtr XMLGetNode2( char *, int);
xmlNodePtr XMLGetChildNode(xmlNodePtr, char *, int);
int XMLLocate( char *, int);

int XMLGetNodeText( xmlNodePtr, char *);
int XMLGetText( char *, char *, int);
int XMLGetText2( char *, char *, int);
int XMLSetNodeText( xmlNodePtr, char *);
int XMLSetText( char *, char *, int);

int XMLGetNodeProp( xmlNodePtr, char *, char *);
int XMLGetProp( char *, char *, char *, int);
int XMLSetNodeProp( xmlNodePtr, char *, char *);
int XMLSetProp( char *, char *, char *, int);

int XMLPutNode( char *, char *, int);

/* 全局变量 */
XMLContext ctx; 
xmlDocPtr	rootdoc; 
xmlNodePtr	rootnode; 

extern char cgDebugflag;

/*delete by zcd 20141218
extern char agDebugfile[256];

***end of delete by zcd 20141218*/
 
/* XML 路径长度和buff */
#define XMLPATHLEN 256
char XMLPATH[XMLPATHLEN];

int _isolat1ToUTF8(xmlChar ** pbuf);
int _UTF8Toisolat1(xmlChar ** pbuf);
int _swParsexml1(char *aXpath, char *aPath, int *iNum);
int EncConvert(char *from, char *to, xmlChar ** pbuf);
int _EncConvert(iconv_t cd, unsigned char *out, int *outlen, unsigned char *in, int *inlen);
int isChineseChar( const char ch);
