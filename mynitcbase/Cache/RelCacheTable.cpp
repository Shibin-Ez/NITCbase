#include "RelCacheTable.h"

#include <cstring>

// #include <stdio.h>

RelCacheEntry *RelCacheTable::relCache[MAX_OPEN];

int RelCacheTable::getRelCatEntry(int relId, RelCatEntry *relCatBuf)
{
  if (relId < 0 || relId >= MAX_OPEN)
  {
    return E_OUTOFBOUND;
  }

  // if there's no entry at the rel-id
  if (relCache[relId] == nullptr)
  {
    return E_RELNOTOPEN;
  }

  // copy the value to the relCatBuf argument
  *relCatBuf = relCache[relId]->relCatEntry;

  return SUCCESS;
}

void RelCacheTable::recordToRelCatEntry(union Attribute record[RELCAT_NO_ATTRS], RelCatEntry *relCatEntry)
{
  strcpy(relCatEntry->relName, record[RELCAT_REL_NAME_INDEX].sVal);
  relCatEntry->numAttrs = (int)record[RELCAT_NO_ATTRIBUTES_INDEX].nVal;

  /* fill the rest of the relCatEntry struct with the values at
      RELCAT_NO_RECORDS_INDEX,
      RELCAT_FIRST_BLOCK_INDEX,
      RELCAT_LAST_BLOCK_INDEX,
      RELCAT_NO_SLOTS_PER_BLOCK_INDEX
  */

  relCatEntry->firstBlk = (int)record[RELCAT_FIRST_BLOCK_INDEX].nVal;
  relCatEntry->lastBlk = (int)record[RELCAT_LAST_BLOCK_INDEX].nVal;
  relCatEntry->numRecs = (int)record[RELCAT_NO_RECORDS_INDEX].nVal;
  relCatEntry->numSlotsPerBlk = (int)record[RELCAT_NO_SLOTS_PER_BLOCK_INDEX].nVal;
}



// STAGE - 4



/* will return the searchIndex for the relation corresponding to `relId
NOTE: this function expects the caller to allocate memory for `*searchIndex`
*/
int RelCacheTable::getSearchIndex(int relId, RecId* searchIndex) {
  // check if 0 <= relId < MAX_OPEN and return E_OUTOFBOUND otherwise
  if (relId < 0 || relId >= MAX_OPEN) return E_OUTOFBOUND;

  // check if relCache[relId] == nullptr and return E_RELNOTOPEN if true
  if (relCache[relId] == nullptr) return E_RELNOTOPEN;

  // copy the searchIndex field of the Relation Cache entry corresponding
  //   to input relId to the searchIndex variable.
  *searchIndex = relCache[relId]->searchIndex;

  return SUCCESS;
}

// sets the searchIndex for the relation corresponding to relId
int RelCacheTable::setSearchIndex(int relId, RecId* searchIndex) {

  // check if 0 <= relId < MAX_OPEN and return E_OUTOFBOUND otherwise
  if (relId < 0 || relId >= MAX_OPEN) return E_OUTOFBOUND;

  // check if relCache[relId] == nullptr and return E_RELNOTOPEN if true
  if (relCache[relId] == nullptr) return E_RELNOTOPEN;

  // update the searchIndex value in the relCache for the relId to the searchIndex argument
  relCache[relId]->searchIndex = *searchIndex;

  return SUCCESS;
}

int RelCacheTable::resetSearchIndex(int relId) {
  // use setSearchIndex to set the search index to {-1, -1}
  relCache[relId]->searchIndex.block = -1;
  relCache[relId]->searchIndex.slot = -1;
  return SUCCESS; 
}



// STAGE - 7

int RelCacheTable::setRelCatEntry(int relId, RelCatEntry *relCatBuf) {

  if(relId < 0 || relId >= MAX_OPEN) {
    return E_OUTOFBOUND;
  }

  if(relCache[relId] == nullptr) {
    return E_RELNOTOPEN;
  }

  // copy the relCatBuf to the corresponding Relation Catalog entry in
  // the Relation Cache Table.
  relCache[relId]->relCatEntry = *relCatBuf;

  // set the dirty flag of the corresponding Relation Cache entry in
  // the Relation Cache Table.
  relCache[relId]->dirty = true;

  return SUCCESS;
}


void RelCacheTable::relCatEntryToRecord(RelCatEntry *relCatEntry, union Attribute rec[RELCAT_NO_ATTRS]) {
  strcpy(rec[RELCAT_REL_NAME_INDEX].sVal, relCatEntry->relName);
  rec[RELCAT_NO_ATTRIBUTES_INDEX].nVal = relCatEntry->numAttrs;
  rec[RELCAT_FIRST_BLOCK_INDEX].nVal = relCatEntry->firstBlk;
  rec[RELCAT_LAST_BLOCK_INDEX].nVal = relCatEntry->lastBlk;
  rec[RELCAT_NO_RECORDS_INDEX].nVal = relCatEntry->numRecs;
  rec[RELCAT_NO_SLOTS_PER_BLOCK_INDEX].nVal = relCatEntry->numSlotsPerBlk;
}