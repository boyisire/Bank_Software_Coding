/*add by zcd 20141220*/
/*
 *  
 *
 *  Edit History:
 *
 *    2009/06/30 - Created by Brilliance Tech.
 *
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "swxml.h"

#include "switch.h"
#include "swUsrfun.h"

#define XML_ATTR_ID	"_id_"
#define XMLIsAbsolutePath(a) (a[0]=='/')

static int strtoupper(char *aBuf);
static int strtolower(char *aBuf);
int GetEncoding(char *aEncoding);
char encoding[][256]={"gb2312","GBK","gb18030","UTF-8"};
int encflag=-1;
/*
 * XMLFree: Free the all related memory for XML context
 */
void XMLFree()
{
  if (ctx.doc)
    xmlFreeDoc(ctx.doc);
  xmlCleanupParser();
  memset(&ctx, 0x00, sizeof(XMLContext));
  swVdebug(4,"XMLFree() success");  
}

/*
 * XMLInit: Init the XML context with a root element
 */

 /*delete by zcd 20141218
int XMLInit(char *root)
{
  char buf[101];

  sprintf(buf, "<?xml version=\"1.0\" encoding=\"utf8\"?><%s/>", root);
  return XMLInitFromBuf(buf);
}
***endo f delete by zcd 20141218*/

/*add by zcd 20141218*/
int XMLInit(char *aVersion,char *aEncoding,char *root)
{
  char buf[101];
   if(NULL == aVersion || NULL == aEncoding) return(11);
   /*判断数据长度是否超出限制*/
   if(strlen(aVersion) > 128 || strlen(aEncoding) > 128)
      return(18);
  sprintf(buf, "<?xml version=\"%s\" encoding=\"%s\"?><%s/>", aVersion,aEncoding, root);
  
  GetEncoding(aEncoding);
  
  swVdebug(4,"XMLInit success,encoding=[%s]",encoding[encflag]); 
  return XMLInitFromBuf(buf);
}
/*end of add by zcd 20141218*/

/*
 * XMLInitFromBuf: Init the XML context from a string buffer
 */
int XMLInitFromBuf(char *buf)
{
  xmlDocPtr doc = NULL;
  xmlNodePtr node = NULL;

 if (ctx.doc != NULL) {
    return 3;
  }

  memset(&ctx, 0x00, sizeof(XMLContext));
  if ((doc = xmlParseDoc(BAD_CAST buf)) == NULL) {
	swVdebug(1,"xmlParseDoc() return null"); 
/*    debug(0,__FILE__,__LINE__, "xmlParseDoc() return null");*/
    xmlCleanupParser();
    return -1;
  }

  if ((node = xmlDocGetRootElement(doc)) == NULL) {
	swVdebug(1,"xmlDocGetRootElement() return null"); 
/*    debug(0,__FILE__,__LINE__, "xmlDocGetRootElement() return null");*/
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return -1;
  }

  ctx.doc = doc;
  ctx.node = node;
  rootdoc = doc;
  rootnode = node;

  return 0;
}

/*
 * XMLInitFromFile: Init the XML context from a file
 */
int XMLInitFromFile(char *file)
{
  xmlDocPtr doc = NULL;
  xmlNodePtr node = NULL;

  memset(&ctx, 0, sizeof(XMLContext));

  /*if ((doc = xmlParseFile(file)) == NULL) */
  if ((doc = xmlReadFile(file, NULL, XML_PARSE_NOBLANKS)) == NULL) {
	swVdebug(1,"xmlParseFile() return null");  
    xmlCleanupParser();
    return -1;
  }

  if ((node = xmlDocGetRootElement(doc)) == NULL) {
    swVdebug(1,"xmlDocGetRootElement() return null");
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return -1;
  }

  ctx.doc = doc;
  ctx.node = node;

//add by lyg 2012-6-14 11:22
  rootdoc = doc;
  rootnode = node;  
  return 0;
}

/*
 * XMLDumpToBuf: Dump the XML document to a string buffer
 */

/*int XMLDumpToBuf( char *buf, int size, int fmt) delete 2008.5.14*/
int XMLDumpToBuf(char *buf, int fmt)
{
  xmlChar *dumpBuf = NULL;
  int dumpLen = 0;

  if (ctx.doc == NULL) {
   swVdebug(1,"[%d]doc can not be null\n",__LINE__);
    return -1;
  }

  xmlDocDumpFormatMemory(ctx.doc, &dumpBuf, &dumpLen, fmt);
  if (dumpBuf == NULL || dumpLen == 0) {
    swVdebug(1,"xmlDocDumpFormatMemory() return fail\n");
    if (dumpBuf)
      xmlFree(dumpBuf);
    return -1;
  }

/* delete at: 2008.5.14
  if (size < dumpLen+1)
  {
    swVdebug(1,"buffer size is not enough to dump");
    xmlFree(dumpBuf);
    return -1;
  }
*/
  memcpy(buf, dumpBuf, dumpLen);
  buf[dumpLen] = 0;
  /*add by pc 20110716 内存没释放*/
  if (dumpBuf)
		xmlFree(dumpBuf);
  /*add by pc 20110716 内存没释放*/
  return dumpLen;
}

/*
 * XMLDumpToFile: Dump the XML document to a file
 */
int XMLDumpToFile(char *file, int fmt)
{

  if (ctx.doc == NULL) {
    swVdebug(1,"[%d]doc can not be null\n",__LINE__);
    return -1;
  }

  if (xmlSaveFormatFile(file, ctx.doc, fmt) < 0) {
    swVdebug(1,"xmlSaveFormatFile() return fail");
    return -1;
  }

  return 0;
}

/*
 * XMLGetNode: 
 */
xmlNodePtr _XMLGetNode(char *xpath, int id)
{
  xmlChar *xpath_buf = NULL;
  xmlNodePtr node = NULL;
  xmlXPathContextPtr xpath_ctx = NULL;
  xmlXPathObjectPtr xpath_obj = NULL;
  char attr[101];

  char *fp = NULL;

  if (ctx.doc == NULL) {
    swVdebug(1,"[%d]doc can not be null\n",__LINE__);
    return NULL;
  }
  if (ctx.node == NULL) {
    swVdebug(1,"current node can not be null\n");
    return NULL;
  }

  if (xpath == NULL || strlen(xpath) == 0)
    return ctx.node;
  if (XMLIsAbsolutePath(xpath))
    xpath_buf = xmlStrdup(BAD_CAST xpath);
  else {
    xpath_buf = xmlGetNodePath(ctx.node);
    if (xpath_buf == NULL) {
      swVdebug(1,"xmlGetNodePath() return null");
      return NULL;
    }
    xpath_buf = xmlStrcat(xpath_buf, BAD_CAST "/");
    if (xpath_buf == NULL) {
      swVdebug(1,"xmlStrcat() return null");
      return NULL;
    }
    xpath_buf = xmlStrcat(xpath_buf, BAD_CAST xpath);
    if (xpath_buf == NULL) {
      swVdebug(1,"xmlStrcat() return null");
      return NULL;
    }
  }
  if (id > 0) {
    sprintf(attr, "[attribute::%s='%d']", XML_ATTR_ID, id);
    xpath_buf = xmlStrcat(xpath_buf, BAD_CAST attr);
    if (xpath_buf == NULL) {
      swVdebug(1,"xmlStrcat() return null");
      return NULL;
    }
  }
  xpath_ctx = xmlXPathNewContext(ctx.doc);
  if (xpath_ctx == NULL) {
    swVdebug(1,"xmlXPathNewContext() return null");
    xmlFree(xpath_buf);
    return NULL;
  }
  xpath_obj = xmlXPathEval(xpath_buf, xpath_ctx);
  if (xpath_obj == NULL) {
    swVdebug(1,"xmlXPathEval(%s) return null\n", xpath_buf);
    xmlFree(xpath_buf);
    xmlXPathFreeContext(xpath_ctx);
    return NULL;
  }
  if (xpath_obj->type != XPATH_NODESET) {
    swVdebug(1,"XPath(%s): type is not NODESET\n", xpath_buf);
    xmlFree(xpath_buf);
    xmlXPathFreeContext(xpath_ctx);
    xmlXPathFreeObject(xpath_obj);
    return NULL;
  }
  if (xpath_obj->nodesetval->nodeNr == 0) {
    swVdebug(1,"XPath(%s): none of nodes be found\n", xpath_buf);
    xmlFree(xpath_buf);
    xmlXPathFreeContext(xpath_ctx);
    xmlXPathFreeObject(xpath_obj);
    return NULL;
  }

  node = xpath_obj->nodesetval->nodeTab[0];
  xmlFree(xpath_buf);
  xmlXPathFreeContext(xpath_ctx);
  xmlXPathFreeObject(xpath_obj);
  return node;
}

/*
 * XMLGetNode2: 
 */
xmlNodePtr _XMLGetNode2(char *xpath, int id)
{
  xmlChar *xpath_buf = NULL;
  xmlNodePtr node = NULL;
  xmlXPathContextPtr xpath_ctx = NULL;
  xmlXPathObjectPtr xpath_obj = NULL;
  char attr[101];
  int i = 0;
  char nodeName[100];

  char *fp = NULL;

  if (ctx.doc == NULL) {
    swVdebug(1,"[%d]doc can not be null\n",__LINE__);
    return NULL;
  }
  if (ctx.node == NULL) {
    swVdebug(1,"current node can not be null\n");
    return NULL;
  }

  if (xpath == NULL || strlen(xpath) == 0)
    return ctx.node;
  if (XMLIsAbsolutePath(xpath))
    xpath_buf = xmlStrdup(BAD_CAST xpath);
  else {
    xpath_buf = xmlGetNodePath(ctx.node);
    if (xpath_buf == NULL) {
      swVdebug(1,"xmlGetNodePath() return null");
      return NULL;
    }
    xpath_buf = xmlStrcat(xpath_buf, BAD_CAST "/");
    if (xpath_buf == NULL) {
      swVdebug(1,"xmlStrcat() return null");
      return NULL;
    }
    xpath_buf = xmlStrcat(xpath_buf, BAD_CAST xpath);
    if (xpath_buf == NULL) {
      swVdebug(1,"xmlStrcat() return null");
      return NULL;
    }
  }

  xpath_ctx = xmlXPathNewContext(ctx.doc);
  if (xpath_ctx == NULL) {
    swVdebug(1,"xmlXPathNewContext() return null");
    xmlFree(xpath_buf);
    return NULL;
  }
  xpath_obj = xmlXPathEval(xpath_buf, xpath_ctx);
  if (xpath_obj == NULL) {
    swVdebug(1,"xmlXPathEval(%s) return null", xpath_buf);
    xmlFree(xpath_buf);
    xmlXPathFreeContext(xpath_ctx);
    return NULL;
  }
  if (xpath_obj->type != XPATH_NODESET) {
    swVdebug(1,"XPath(%s): type is not NODESET", xpath_buf);
    xmlFree(xpath_buf);
    xmlXPathFreeContext(xpath_ctx);
    xmlXPathFreeObject(xpath_obj);
    return NULL;
  }
  if (xpath_obj->nodesetval->nodeNr == 0) {
    swVdebug(1,"XPath(%s): none of nodes be found", xpath_buf);
    xmlFree(xpath_buf);
    xmlXPathFreeContext(xpath_ctx);
    xmlXPathFreeObject(xpath_obj);
    return NULL;
  }

  node = xpath_obj->nodesetval->nodeTab[0];
  xmlFree(xpath_buf);
  xmlXPathFreeContext(xpath_ctx);
  xmlXPathFreeObject(xpath_obj);
  memset(nodeName, 0x00, sizeof(nodeName));
  strcpy(nodeName, node->name);
  i = 0;
  if (id > 0) {
    while (NULL != node) {
      if ((!strcmp(node->name, nodeName))) {
        i++;
      }
      if (i == id)
        break;
      node = node->next;
    }
    if (i != id) {
      swVdebug(1,"XPath(%s)(%d): none of nodes be found", xpath_buf,id);
      return NULL;
    }
  }

  return node;
}

/*
 * XMLGetChildNode:
 */
xmlNodePtr XMLGetChildNode(xmlNodePtr parent, char *childname, int id)
{
  xmlNodePtr node = NULL;
  xmlChar *buf = NULL;
  char sid[101];

  sprintf(sid, "%d", id);
  node = parent->children;
  while (node) {
    if (xmlStrcmp(node->name, BAD_CAST childname) == 0) {
      if (id == 0)
        return node;
      buf = xmlGetProp(node, BAD_CAST XML_ATTR_ID);
      if (buf == NULL) {
        xmlFree(buf);
        if (id == 1)
          return node;
        return NULL;
      }
      if (xmlStrcmp(buf, BAD_CAST sid) == 0) {
        xmlFree(buf);
        return node;
      }
      xmlFree(buf);
    }
    node = node->next;
  }
  return NULL;
}

/*
 * XMLLocate:
 */
int XMLLocate(char *xpath, int id)
{
	
  xmlNodePtr node = NULL;

  node = XMLGetNode(xpath, id);
  /*modify by zcd 20141224
  if (node == NULL)
    return -2;*/
  if (node == NULL)
    return 2;
  ctx.node = node;
  return 0;
}

/*
 * XMLGetNodeText:
 */

/*int XMLGetNodeText( xmlNodePtr node, char *buf, int size)delete at: 2008.5.14 */
int XMLGetNodeText(xmlNodePtr node, char *buf)
{
  xmlChar *result = NULL;

  /*
     swVdebug(2,"ycctest: XML_CHAR_ENCODING_8859_1:[%d], \
     XML_CHAR_ENCODING_UTF8[%d]", XML_CHAR_ENCODING_8859_1, XML_CHAR_ENCODING_UTF8);
     swVdebug(2, "ycctest: type:%d", node->type);
   */
  if ((result = xmlNodeGetContent(node)) == NULL) {
    swVdebug(1,"xmlNodeGetContent(%s) return null", node->name);
    return -1;
  }

  /* 
     swVdebug(2, "ycctest: node result:");
     swDebughex(result, 6);
   */
  if (ctx.doc->charset == XML_CHAR_ENCODING_UTF8) {
  
    if(isEqualLength(result)){
       swVdebug(5,"func _UTF8Toisolat1 ");
      
       if (_UTF8Toisolat1(&result) == -1) {
         xmlFree(result);
         return -1;
       }
    }
  }

  else if (ctx.doc->charset != XML_CHAR_ENCODING_8859_1) {
    swVdebug(1,"illegal charset(%d)", ctx.doc->charset);
    /*
       swVdebug(2,"ycctest: illegal charset(%d)", ctx.doc->charset);
     */
    xmlFree(result);
    return -1;
  }

  /* 
     swVdebug(2,"ycctest: buf(%s)", (char*)result);
   */
  strcpy(buf, (char *) result);
  xmlFree(result);
  return 0;
}

/*
 * XMLGetText:
 */

/*int XMLGetText( char *xpath, char *buf, int size, int id) delete at: 2008.5.14*/
int XMLGetText(char *xpath, char *buf, int id)
{
  xmlNodePtr node = NULL;

  node = XMLGetNode(xpath, id);
  /*modify by zcd 20141224
  if (node == NULL)
    return -2;*/
  if (node == NULL)
    return 2;
  return XMLGetNodeText(node, buf);
}

/*
 * XMLGetText2:
 */
int XMLGetText2(char *xpath, char *buf, int id)
{
  xmlNodePtr node = NULL;

  node = XMLGetNode2(xpath, id);
  /*modify by zcd 20141224
  if (node == NULL)
    return -2;*/
  if (node == NULL)
    return 2;
  return XMLGetNodeText(node, buf);
}

/*
 * XMLSetNodeText:
 */
int XMLSetNodeText(xmlNodePtr node, char *buf)
{
  /*
     swVdebug(2, "ycctest: pack charset:[%d]", ctx.doc->charset);
   */
  if (ctx.doc->charset == XML_CHAR_ENCODING_UTF8) {
    xmlChar *tmpbuf = NULL;

    tmpbuf = xmlStrdup((xmlChar *) buf);
  
  
    if(isEqualLength(tmpbuf)){
        swVdebug(5,"func _isolat1ToUTF8()");   
        if (_isolat1ToUTF8(&tmpbuf) == -1) {
        swVdebug(1,"isolat1ToUTF8() return fail");
        xmlFree(tmpbuf);
        return -1;
        }
    }

    xmlNodeSetContent(node, tmpbuf);
    xmlFree(tmpbuf);
  }
  else {
    xmlNodeSetContent(node, BAD_CAST buf);
  }

  return 0;
}

/*
 * XMLSetText:
 */
int XMLSetText(char *xpath, char *buf, int id)
{
  xmlNodePtr node = NULL;

  node = XMLGetNode(xpath, id);
  /*modify by zcd 20141224
  if (node == NULL)
    return -2;*/
  if (node == NULL)
    return 2;
  return XMLSetNodeText(node, buf);
}

/*
 * XMLGetNodeProp:
 */
int XMLGetNodeProp(xmlNodePtr node, char *attr, char *buf)
{
  xmlChar *result = NULL;

  if ((result = xmlGetProp(node, (xmlChar *) attr)) == NULL) {
    swVdebug(1,"xmlNodeGetContent(%s) return null", node->name);
    return -1;
  }

  if (ctx.doc->charset == XML_CHAR_ENCODING_UTF8) {
   
      /*add by zcd 20150114*/
  
      if(isEqualLength(result)){
    
      swVdebug(5,"func _UTF8Toisolat1()");
      
      if (_UTF8Toisolat1(&result) == -1) {
        xmlFree(result);
        return -1;
      }
    }
  }
  else if (ctx.doc->charset != XML_CHAR_ENCODING_8859_1) {
    swVdebug(1,"illegal charset(%d)", ctx.doc->charset);
    xmlFree(result);
    return -1;
  }
  strcpy(buf, (char *) result);
  xmlFree(result);
  return 0;
}

/*
 * XMLGetProp:
 */
int XMLGetProp(char *xpath, char *attr, char *buf, int id)
{
  xmlNodePtr node = NULL;

  node = XMLGetNode(xpath, id);
  /*modify by zcd 20141224
  if (node == NULL)
    return -2;*/
  if (node == NULL)
    return 2;
  /*return XMLGetNodeProp( node, attr, buf, size); */
  return XMLGetNodeProp(node, attr, buf);
}

/*
 * XMLSetNodeProp:
 */
int XMLSetNodeProp(xmlNodePtr node, char *attr, char *buf)
{
  if (ctx.doc->charset == XML_CHAR_ENCODING_UTF8) {
    xmlChar *tmpbuf = NULL;

    tmpbuf = xmlStrdup((xmlChar *) buf);
	/*add by zcd 20150114*/
 	if(isEqualLength(tmpbuf)){
      swVdebug(5,"func _isolat1ToUTF8 ");
      if (_isolat1ToUTF8(&tmpbuf) == -1) {
        swVdebug(1,"isolat1ToUTF8() return fail");
        xmlFree(tmpbuf);
        return -1;
      }
  
    }
    xmlSetProp(node, BAD_CAST attr, tmpbuf);
    xmlFree(tmpbuf);
  }
  else
    xmlSetProp(node, BAD_CAST attr, BAD_CAST buf);

  return 0;
}

/*
 * XMLSetProp:
 */
int XMLSetProp(char *xpath, char *attr, char *buf, int id)
{
  xmlNodePtr node = NULL;

  node = XMLGetNode(xpath, id);
  /*modify by zcd 20141224
  if (node == NULL)
    return -2;*/
  if (node == NULL)
    return 2;
  return XMLSetNodeProp(node, attr, buf);
}

/*
 * _XMLPutNode:
 */
int _XMLPutNode(char *xpath, char *text, int id)
{
  char *s = NULL, *xpath_buf = NULL;
  char sid[101], alTmpPath[101];
  char alXpath[256], alPath[101];
  char *alTmp = NULL;
  int i = 0, ilRc = 0;
  xmlNodePtr parent = NULL, child = NULL, node = NULL;
  int ilLen = 0, ilNum = 0;
  char s1[256];

  if (ctx.doc == NULL) {
    swVdebug(1,"[%d]doc can not be null\n",__LINE__);
    return -1;
  }

  memset(alXpath, 0x00, sizeof(alXpath));
  memset(alPath, 0x00, sizeof(alPath));
  memset(alTmpPath, 0x00, sizeof(alTmpPath));

  xpath_buf = (char *) xmlStrdup(BAD_CAST xpath);
  if (XMLIsAbsolutePath(xpath)) {
    s = strtok(xpath_buf + 1, "/");
    if (s == NULL) {
      swVdebug(1,"[%s] is invalid xpath", xpath);
      xmlFree((xmlChar *) xpath_buf);
      return -1;
    }
    parent = xmlDocGetRootElement(ctx.doc);
    if (xmlStrcmp(parent->name, BAD_CAST s) != 0) {
      swVdebug(1,"root element [%s] error", s);
      xmlFree((xmlChar *) xpath_buf);
      return -1;
    }
    s = strtok(NULL, "/");
  }
  else {
    parent = ctx.node;
    s = strtok(xpath_buf, "/");
  }

  while (s) {
    /*
       for(i=0;(s[i]!='/')&&(i<strlen(s));i++)
       s1[i+1] = s[i];
     */
    memset(s1, 0x00, sizeof(s1));
    memmove(s1 + 1, s, strlen(s));
    s1[0] = '/';
    s1[strlen(s) + 1] = 0x0;
    ilNum = 0;
    _swParsexml1(s1, (char *) alPath, &ilNum);
    child = XMLGetChildNode(parent, s, ilNum);
    if (child == NULL) {
      child = xmlNewChild(parent, NULL, BAD_CAST alPath, NULL);
      if (child == NULL) {
        swVdebug(1,"xmlNewChild(%s) return null", s);
        xmlFree((xmlChar *) xpath_buf);
        return -1;
      }      
    }
    parent = child;
    s = strtok(NULL, "/");
  }
  xmlFree((xmlChar *) xpath_buf);
  id = ilNum;

  if (ilNum > 0) {
    sprintf(sid, "%d", ilNum);
    if (XMLSetNodeProp(parent, XML_ATTR_ID, sid) == -1)
      return -1;
  }

  return XMLSetNodeText(parent, text);
}
 

/*
 * _UTF8Toisolat1:
 */
int _UTF8Toisolat1(xmlChar ** pbuf)
{
  /* modified by ycc Mon Jun 28 17:26:09 CST 2010
     return EncConvert("UTF-8", "ISO8859-1", pbuf);
   */
  if(encflag!=-1 && encflag!=3)
    return EncConvert("UTF-8", encoding[encflag], pbuf);
  else 
    return EncConvert("UTF-8", "GBK", pbuf);
}

/*
 * isolat1ToUTF8: 
 */
int _isolat1ToUTF8(xmlChar ** pbuf)
{
  /* modified by ycc Mon Jun 28 17:26:09 CST 2010
     return EncConvert("ISO8859-1", "UTF-8", pbuf);
   */
  if(encflag!=-1 && encflag!=3)
    return EncConvert(encoding[encflag], "UTF-8", pbuf);
  else
    return EncConvert("GBK", "UTF-8", pbuf);

}

/*
 * EncConvert:
 */
int EncConvert(char *from, char *to, xmlChar ** pbuf)
{
  iconv_t cd = NULL;
  xmlChar *result = NULL;
  int ret = 0, i_len = 0, o_len = 0;

  if ((cd = iconv_open(to, from)) == (iconv_t) - 1) {
    swVdebug(1,"iconv_open(%s,%s) return -1:%s", from, to,strerror(errno));
    return -1;
  }

  i_len = strlen((char *) (*pbuf)) + 1;
  o_len = i_len * 4;
  result = xmlMalloc(o_len);
  ret = _EncConvert(cd, (unsigned char *) result, &o_len, (unsigned char *) (*pbuf), &i_len);
  if (ret == -1) {
    swVdebug(1, "iconv fail -1 ");
    xmlFree(result);
    iconv_close(cd);
    return -1;
  }

  xmlFree(*pbuf);
  *pbuf = result;
  iconv_close(cd);
  result[o_len] = 0x00;

  return 0;
}

int _EncConvert(iconv_t cd, unsigned char *out, int *outlen, unsigned char *in, int *inlen)
{
  size_t icv_inlen = *inlen, icv_outlen = *outlen;
  const char *icv_in = (const char *) in;
  char *icv_out = (char *) out;
  int ret = 0;

  /*
     swVdebug(2, "ycctest: bufin len:%d, bufout len:%d", icv_inlen, icv_outlen);
  swDebughex(icv_in, icv_inlen);
   */
  ret = iconv(cd, (char **) &icv_in, &icv_inlen, &icv_out, &icv_outlen);

/* modified by ycc Mon Jun 25 17:24:24 CST 2010
  if (in != NULL)
*/
  if (icv_in != NULL) {
    *inlen -= icv_inlen;
    *outlen -= icv_outlen;
  }
  else {
    *inlen = 0;
    *outlen = 0;
  }
  if ((icv_inlen != 0) || (ret == -1))
    return -1;

/* modified by ycc
  if ((icv_inlen != 0) || (ret == -1)) {
    if(icv_inlen != 0) {
      swVdebug(2, "ycctest: icv_inlen[%d]",  icv_inlen);
    }
    else {
      switch(errno){
        case EILSEQ:
          swVdebug(1, "ycctest:An invalid multibyte sequence has been encountered in the input");
        case EINVAL:
          swVdebug(1,"An incomplete multibyte sequence has been encountered in the input");
        case E2BIG:
          swVdebug(1,"here is not sufficient room at icv_out");

      }
    }
    return -1;
  }
*/
  return 0;
}

int _swParsexml1(char *aXpath, char *aPath, int *iNum)
{
  int i = 0, j = 0;
  char alTmp[101];
  char alBuf[101];

  if (aXpath[0] == '\0') {
#ifdef _XML_DEBUG
    printf("S0450: _swParsexml1() 输入的XPATH为空!\n");
#endif
    aXpath[0] = 0;
    return (0);
  }

  if (aXpath[0] != '/') {
#ifdef _XML_debug
    printf("S0460: _swParsexml1() 所输入的XPATH不正确,请检查!\n");
#endif
    return (0);
  }

  if (strlen(aXpath) == 1) {
    aXpath[0] = 0;
    aPath[0] = 0;
    return (0);
  }

  for (i = 1; i < strlen(aXpath); i++) {
    if (aXpath[i] == '/')
      break;
  }
  if (i > 1) {
    memcpy(aPath, aXpath + 1, i - 1);
    aPath[i - 1] = 0;
    /* 找出相对路径的值 */
    if (aPath[i - 2] == ']') {
      for (j = i - 2; j > 0; j--) {
        if (aXpath[j] == '[')
          break;
      }
      if (i - 2 > j) {
        memset(alBuf, 0x00, sizeof(alBuf));
        memcpy(alBuf, aPath + j, i - j - 2);
        alBuf[i - j - 2] = 0;
        *iNum = atoi(alBuf);
        if (*iNum <= 0)
          *iNum = 0;
      }
      aPath[j - 1] = 0;
    }
  }
  if (strlen(aXpath) > i) {
    memcpy(alTmp, aXpath + i, strlen(aXpath) - i);
    alTmp[strlen(aXpath) - i] = 0;
    strcpy(aXpath, alTmp);
  }
  else
    aXpath[0] = 0;

  return (0);
}

xmlNodePtr XMLGetNode(char *xpath, int id)
{
  char alPath[101];
  int ilNum = 0;
  char *alTmp = NULL;
  int ilRc = 0;
  xmlNodePtr node = NULL;
  char alXpath[101];
  char *fp = NULL;

  memset(alXpath, 0x00, sizeof(alXpath));
  if (xpath[0] == '/') {
	
    ctx.doc = rootdoc;
    ctx.node = rootnode;
    fp = xpath + 1;
    fp = strstr(fp, "/");
    if (fp == NULL)
      return rootnode;
    memmove(xpath, fp, strlen(fp) + 1);
    strcpy(alXpath, xpath);
  }
  else {
  	
    alXpath[0] = '/';
    strcpy(alXpath + 1, xpath);
  }

  ilNum = 0;
  _swParsexml1(alXpath, alPath, &ilNum);

  node = _XMLGetNode(alPath, ilNum);
  ctx.node = node;
  while ((alTmp = strchr(alXpath, '/')) != NULL) {
  	
    ilNum = 0;
    ilRc = _swParsexml1((char *) alXpath, (char *) alPath, &ilNum);
    if (ilRc) {
      return (NULL);
    }
    node = _XMLGetNode(alPath, ilNum);
    ctx.node = node;
  }
  return ctx.node;

}

xmlNodePtr XMLGetNode2(char *xpath, int id)
{
  char alPath[101];
  int ilNum = 0;
  char *alTmp = NULL;
  int ilRc = 0;
  xmlNodePtr node = NULL;
  char alXpath[101];
  char *fp = NULL;

  memset(alXpath, 0x00, sizeof(alXpath));
  if (xpath[0] == '/') {
    ctx.doc = rootdoc;
    ctx.node = rootnode;
    fp = xpath + 1;
    fp = strstr(fp, "/");
    if (fp == NULL)
      return rootnode;
    memmove(xpath, fp, strlen(fp) + 1);
    strcpy(alXpath, xpath);
  }
  else {
    alXpath[0] = '/';
    strcpy(alXpath + 1, xpath);
  }

  ilNum = 0;
  _swParsexml1(alXpath, alPath, &ilNum);
  node = _XMLGetNode2(alPath, ilNum);
  ctx.node = node;
  while ((alTmp = strchr(alXpath, '/')) != NULL) {
    ilNum = 0;
    ilRc = _swParsexml1((char *) alXpath, (char *) alPath, &ilNum);
    if (ilRc) {
      return (NULL);
    }
    node = _XMLGetNode2(alPath, ilNum);
    ctx.node = node;
  }

  return ctx.node;

}

/*
 * XMLPutNode:
 */
int XMLPutNode(char *xpath, char *text, int id)
{
  
  swVdebug(5, "XMLPutNode.xpath[%s], text:[%s]", xpath, text);
   
  char *alTmp1 = NULL, *alTmp2 = NULL;
  int len = 0;

  len = strlen(xpath);
  for (; len > 0; len--) {
    if (xpath[len] == '/')
      break;
  }
  alTmp1 = xpath + len + 1;

  alTmp2 = strstr(xpath, "[");
  if (alTmp2 == NULL) {
    return _XMLPutNode(xpath, text, id);
  }
  else if (alTmp2 > alTmp1) {
    return _XMLPutNode(xpath, text, id);
  }
  else {
    xpath[len] = '\0';
    XMLLocate(xpath, id);
    
    return _XMLPutNode(alTmp1, text, id);
  }
}

void StrToUTF8(char *alXpathTmp)
{
  char *buf2;
  xmlChar *tmpbuf = NULL;

  buf2 = alXpathTmp;
  tmpbuf = xmlStrdup((xmlChar *) buf2);
  swVdebug(3,"encflag_test=%d",encflag);
  if(encflag!=-1 && encflag!=3)
    EncConvert(encoding[encflag],"UTF-8", &tmpbuf);
  else
    EncConvert("GBK", "UTF-8", &tmpbuf);
  strcpy(alXpathTmp, tmpbuf);
  /*add by pc 20110716 内存没释放*/
  xmlFree(tmpbuf);
  /*add by pc 20110716 内存没释放*/
  return;
}


/*add by zcd 20141218*/
int isChineseChar( const char ch)
{
        return (ch & 0x80);
}
/*end of add by zcd 20141218*/



 /*add by zcd 20150116*/
int isEqualLength(xmlChar *str)
{
    int clen = strlen(str);
    int i,flag=0;
    for(i=0;i<clen;i++)
    {
        if((unsigned char)str[i]>0 && (unsigned char)str[i]<=127)
            flag=0;
        else{
            flag=1;
            break;
        }
    }
    if(flag==1)
        return 1;
    else 
        return 0;
}
/*end of add by zcd 20150116*/

 static int strtoupper(char *aBuf)
 {
    while (*aBuf != '\0')
    {
      *aBuf = toupper(*aBuf);
      aBuf++;
    }
    return 0;
 }

static int strtolower(char *aBuf)
{
  while (*aBuf != '\0'){
    *aBuf = tolower(*aBuf);
    aBuf++;
  }
  return 0;
}


int GetEncoding(char *aEncoding)
{
  int i;
  char alTmp[127];
  memset(alTmp,0x00,sizeof(alTmp));
  strcpy(alTmp,aEncoding);
  for(i=0;i<4;i++)
  {
    if(0==strncmp(alTmp,encoding[i],strlen(encoding[i]))) {
        encflag=i;
        break;
    }
    int result=strtoupper(alTmp);
    if(0==strncmp(alTmp,encoding[i],strlen(encoding[i]))   ){
       encflag=i;
       break;
    }
    strtolower(alTmp);
    if(0==strncmp(alTmp,encoding[i],strlen(encoding[i])) ){
      encflag=i;
      break;
    }
  }
  return 0;
}

