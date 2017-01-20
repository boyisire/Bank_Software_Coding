#ifndef __HASH_TABLE_H
#define __HASH_TABLE_H

int idxSize(int keySize, int maxRecords);

void idxInit(void *indexArea, int keySize, int maxRecords);

int idxInsert(void *indexArea, char *key, long data);

long idxSelect(void *indexArea, char *key);

void idxClear(void *indexArea);

#endif
