#include "AttrCacheTable.h"

#include <cstring>
#include <iostream>


AttrCacheEntry* AttrCacheTable::attrCache[MAX_OPEN];


int AttrCacheTable::getAttrCatEntry(int relId, int attrOffset, AttrCatEntry* attrCatBuf) {
  // check if 0 <= relId < MAX_OPEN and return E_OUTOFBOUND otherwise
  if (relId < 0 || relId > MAX_OPEN) {
    return E_OUTOFBOUND;
  }

  // check if attrCache[relId] == nullptr and return E_RELNOTOPEN if true
  if (attrCache[relId] == nullptr) {
    return E_RELNOTOPEN;
  }

  // traverse the linked list of attribute cache entries
  for (AttrCacheEntry* entry = attrCache[relId]; entry != nullptr; entry = entry->next) {
    if (entry->attrCatEntry.offset == attrOffset) {

      // copy entry->attrCatEntry to *attrCatBuf and return SUCCESS;
      *attrCatBuf = entry->attrCatEntry;
      return SUCCESS;
    }
  }

  // there is no attribute at this offset
  return E_ATTRNOTEXIST;
}


void AttrCacheTable::recordToAttrCatEntry(union Attribute record[ATTRCAT_NO_ATTRS],
                                          AttrCatEntry* attrCatEntry) {
  strcpy(attrCatEntry->relName, record[ATTRCAT_REL_NAME_INDEX].sVal);

  // copy the rest of the fields in the record to the attrCacheEntry struct
  strcpy(attrCatEntry->attrName, record[ATTRCAT_ATTR_NAME_INDEX].sVal);
  attrCatEntry->attrType = (int)record[ATTRCAT_ATTR_TYPE_INDEX].nVal;
  attrCatEntry->offset = (int)record[ATTRCAT_OFFSET_INDEX].nVal;
  attrCatEntry->primaryFlag = (bool)record[ATTRCAT_PRIMARY_FLAG_INDEX].nVal;
  attrCatEntry->rootBlock = (int)record[ATTRCAT_ROOT_BLOCK_INDEX].nVal;
}



// Stage - 4


/* returns the attribute with name `attrName` for the relation corresponding to relId
NOTE: this function expects the caller to allocate memory for `*attrCatBuf`
*/
int AttrCacheTable::getAttrCatEntry(int relId, char attrName[ATTR_SIZE], AttrCatEntry* attrCatBuf) {

  // check that relId is valid and corresponds to an open relation
  if (relId < 0 || relId >= MAX_OPEN) return E_OUTOFBOUND;
  if (attrCache[relId] == nullptr) return E_RELNOTOPEN;

  // iterate over the entries in the attribute cache and set attrCatBuf to the entry that
  //    matches attrName
  AttrCacheEntry *temp;
  for (temp = attrCache[relId]; temp != nullptr; temp = temp->next) {
    if (strcmp(temp->attrCatEntry.attrName, attrName) == 0) {
      *attrCatBuf = temp->attrCatEntry;
      return SUCCESS;
    }
  }

  // no attribute with name attrName for the relation
  return E_ATTRNOTEXIST;
}



// STAGE- 10

int AttrCacheTable::getSearchIndex(int relId, char attrName[ATTR_SIZE], IndexId *searchIndex) {

  if( relId < 0 || relId >= MAX_OPEN ) return E_OUTOFBOUND;
  if(attrCache[relId] == nullptr) return E_RELNOTOPEN;

  for(AttrCacheEntry *p = attrCache[relId]; p != nullptr; p = p->next)
  {
    if (strcmp(p->attrCatEntry.attrName, attrName) == 0)
    {
      //copy the searchIndex field of the corresponding Attribute Cache entry
      //in the Attribute Cache Table to input searchIndex variable.
      searchIndex->block = p->searchIndex.block;
      searchIndex->index = p->searchIndex.index;

      return SUCCESS;
    }
  }

  return E_ATTRNOTEXIST;
}


int AttrCacheTable::getSearchIndex(int relId, int attrOffset, IndexId *searchIndex) {

  if( relId < 0 || relId >= MAX_OPEN ) return E_OUTOFBOUND;
  if(attrCache[relId] == nullptr) return E_RELNOTOPEN;

  for(AttrCacheEntry *p = attrCache[relId]; p != nullptr; p = p->next)
  {
    if (p->attrCatEntry.offset == attrOffset)
    {
      //copy the searchIndex field of the corresponding Attribute Cache entry
      //in the Attribute Cache Table to input searchIndex variable.
      searchIndex->block = p->searchIndex.block;
      searchIndex->index = p->searchIndex.index;

      return SUCCESS;
    }
  }

  return E_ATTRNOTEXIST;
}


int AttrCacheTable::setSearchIndex(int relId, char attrName[ATTR_SIZE], IndexId *searchIndex) {

  if( relId < 0 || relId >= MAX_OPEN ) return E_OUTOFBOUND;
  if(attrCache[relId] == nullptr) return E_RELNOTOPEN;

  for(AttrCacheEntry *p = attrCache[relId]; p != nullptr; p = p->next)
  {
    if (strcmp(p->attrCatEntry.attrName, attrName) == 0)
    {
      // copy the input searchIndex variable to the searchIndex field of the
      //corresponding Attribute Cache entry in the Attribute Cache Table.
      p->searchIndex.block = searchIndex->block;
      p->searchIndex.index = searchIndex->index;

      return SUCCESS;
    }
  }

  return E_ATTRNOTEXIST;
}


int AttrCacheTable::setSearchIndex(int relId, int attrOffset, IndexId *searchIndex) {

  if( relId < 0 || relId >= MAX_OPEN ) return E_OUTOFBOUND;
  if(attrCache[relId] == nullptr) return E_RELNOTOPEN;

  for(AttrCacheEntry *p = attrCache[relId]; p != nullptr; p = p->next)
  {
    if (p->attrCatEntry.offset == attrOffset)
    {
      // copy the input searchIndex variable to the searchIndex field of the
      //corresponding Attribute Cache entry in the Attribute Cache Table.
      p->searchIndex.block = searchIndex->block;
      p->searchIndex.index = searchIndex->index;

      return SUCCESS;
    }
  }

  return E_ATTRNOTEXIST;
}

int AttrCacheTable::resetSearchIndex(int relId, char attrName[ATTR_SIZE]) {

  // declare an IndexId having value {-1, -1}
  IndexId resetedIndex;
  resetedIndex.block = resetedIndex.index = -1;

  // set the search index to {-1, -1} using AttrCacheTable::setSearchIndex
  int retVal = setSearchIndex(relId, attrName, &resetedIndex);

  // return the value returned by setSearchIndex
  return retVal;
}

int AttrCacheTable::resetSearchIndex(int relId, int attrType) {

  // declare an IndexId having value {-1, -1}
  IndexId resetedIndex;
  resetedIndex.block = resetedIndex.index = -1;

  // set the search index to {-1, -1} using AttrCacheTable::setSearchIndex
  int retVal = setSearchIndex(relId, attrType, &resetedIndex);

  // return the value returned by setSearchIndex
  return retVal;
}



// STAGE - 11

int AttrCacheTable::setAttrCatEntry(int relId, int attrOffset, AttrCatEntry *attrCatBuf) {

  if(relId < 0 || relId >= MAX_OPEN) return E_OUTOFBOUND;
  if(attrCache[relId] == nullptr) return E_RELNOTOPEN;

  for(AttrCacheEntry *p = attrCache[relId]; p != nullptr; p = p->next)
  {
    if(p->attrCatEntry.offset == attrOffset)
    {
      // copy the attrCatBuf to the corresponding Attribute Catalog entry in
      // the Attribute Cache Table.
      p->attrCatEntry = *attrCatBuf;

      // set the dirty flag of the corresponding Attribute Cache entry in the
      // Attribute Cache Table.
      p->dirty = true;

      return SUCCESS;
    }
  }

  return E_ATTRNOTEXIST;
}

int AttrCacheTable::setAttrCatEntry(int relId, char attrName[ATTR_SIZE], AttrCatEntry *attrCatBuf) {

  if(relId < 0 || relId >= MAX_OPEN) return E_OUTOFBOUND;
  if(attrCache[relId] == nullptr) return E_RELNOTOPEN;

  for(AttrCacheEntry *p = attrCache[relId]; p != nullptr; p = p->next)
  {
    if(strcmp(p->attrCatEntry.attrName, attrName) == 0)
    {
      // copy the attrCatBuf to the corresponding Attribute Catalog entry in
      // the Attribute Cache Table.
      p->attrCatEntry = *attrCatBuf;

      // set the dirty flag of the corresponding Attribute Cache entry in the
      // Attribute Cache Table.
      p->dirty = true;

      return SUCCESS;
    }
  }

  return E_ATTRNOTEXIST;
}


void AttrCacheTable::attrCatEntryToRecord(AttrCatEntry *attrCatEntry, union Attribute record[]) {
  strcpy(record[ATTRCAT_REL_NAME_INDEX].sVal, attrCatEntry->relName);
  strcpy(record[ATTRCAT_ATTR_NAME_INDEX].sVal, attrCatEntry->attrName);
  record[ATTRCAT_ATTR_TYPE_INDEX].nVal = attrCatEntry->attrType;
  record[ATTRCAT_PRIMARY_FLAG_INDEX].nVal = attrCatEntry->primaryFlag;
  record[ATTRCAT_ROOT_BLOCK_INDEX].nVal = attrCatEntry->rootBlock;
  record[ATTRCAT_OFFSET_INDEX].nVal = attrCatEntry->offset;
}