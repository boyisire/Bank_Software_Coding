#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "HashIndex.h"

static const int HI_MAGIC = 12344321;

#define NIL_NODE -1

typedef struct _idxNode {
    int next;
    long data;
    char key[1];
} INode;

#define IDX_BUCKETS 10240
typedef struct _idxTab {
    int magic;
    int keySize;
    int maxRecords;
	int nodeSize;
    int bucket[IDX_BUCKETS];
	int free;
    char dummy[1];
} ITab;

static unsigned int _idx_hash(char * key);
static INode * _idx_node(ITab *tab, int nodeId);
static int _idx_newNode(ITab *tab);
static int _idx_findNode(ITab *tab, char *key);
static void _idx_freeNode(ITab *tab, int nodeId);

int idxSize(int keySize, int maxRecords)
{
    int ns = sizeof(INode) + keySize - 1;
    int sz = sizeof(ITab) + maxRecords * ns - 1;
	return sz;
}

void idxInit(void *indexArea, int keySize, int maxRecords)
{
		int i;
    ITab *tab = (ITab *)indexArea;
	tab->magic = HI_MAGIC;
    tab->keySize = keySize;
    tab->maxRecords = maxRecords;
    tab->nodeSize = sizeof(INode) + keySize - 1;
    for (i = 0; i < IDX_BUCKETS; i++) {
        tab->bucket[i] = NIL_NODE;
    }
    INode *n1 = (INode *)(tab->dummy);
    for (i = 1; i < maxRecords; i++) {
        INode *n2 = (INode *)(tab->dummy + tab->nodeSize * i);
		n1->next = i;
		n1 = n2;
	}
	n1->next = NIL_NODE;
    tab->free = 0;
}

int idxInsert(void *indexArea, char *key, long data)
{
    if (indexArea == NULL) {
        return -1;
    }
    ITab *tab = (ITab *)indexArea;
	if (tab->magic != HI_MAGIC) {
        return -HI_MAGIC;
	}
    if (strlen(key) > tab->keySize) {
        return -__LINE__;
    }

	INode * n;

    int id = _idx_findNode(tab, key);
    if (id == NIL_NODE) {
        unsigned int idx = _idx_hash(key) % IDX_BUCKETS;
        id = _idx_newNode(tab);
		if (id == NIL_NODE) {
            return -__LINE__;
		}
		n = _idx_node(tab, id);
        strcpy(n->key, key);
	    n->next = tab->bucket[idx];
        tab->bucket[idx] = id;
    } else {
		n = _idx_node(tab, id);
	}
    n->data = data;
	return 0;
}

long idxSelect(void *indexArea, char *key)
{
    if (indexArea == NULL) {
        return -1;
    }
    ITab *tab = (ITab *)indexArea;
	if (tab->magic != HI_MAGIC) {
        return -HI_MAGIC;
	}

    int id = _idx_findNode(tab, key);
    if (id == NIL_NODE) {
	    return -2;
    }
    INode *n = _idx_node(tab, id);
    return n->data;
}

void idxClear(void *indexArea)
{
		int i;
    if (indexArea == NULL) {
        return;
    }
    ITab * tab = (ITab *)indexArea;
	if (tab->magic != HI_MAGIC) {
        return;
	}

    for (i = 0; i < IDX_BUCKETS; i++) {
        int id = tab->bucket[i];
        while (id != NIL_NODE) {
			int id2 = id;
            INode * t = _idx_node(tab, id);
			id = t->next;
            _idx_freeNode(tab, id2);
        }
        tab->bucket[i] = NIL_NODE;
    }
}

static unsigned int _idx_hash(char * key)
{
		int i;
    unsigned int h = 0;
    for (i = 0; key[i] != 0; i++) {
        h *= 16777619;
        h ^= (unsigned int)key[i];
    }   
    return h;
}

int _idx_newNode(ITab *tab)
{
    if (tab->free == NIL_NODE) {
		return NIL_NODE;
	}
	int id = tab->free;
    INode * n = _idx_node(tab, id);
	tab->free = n->next;
    return id;
}

void _idx_freeNode(ITab *tab, int nodeId)
{
    INode * n = _idx_node(tab, nodeId);
    n->next = tab->free;
	tab->free = nodeId;
}

int _idx_findNode(ITab *tab, char *key)
{
    unsigned int idx = _idx_hash(key) % IDX_BUCKETS;
    int id = tab->bucket[idx];
    if (id == NIL_NODE) {
        return NIL_NODE;
    }

    while (id != NIL_NODE) {
        INode *p = _idx_node(tab, id);
        if (strcmp(p->key, key) == 0) {
            return id;
        }
        id = p->next;
    }
    return NIL_NODE;
}

INode * _idx_node(ITab *tab, int nodeId)
{
     INode *n = (INode *)(tab->dummy + tab->nodeSize * nodeId);
	 return n;
}
