#include "OpenRelTable.h"

#include <cstring>

#include <stdlib.h>
// #include <iostream>

OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];

OpenRelTable::OpenRelTable() {

  // initialize relCache and attrCache with nullptr
  for (int i = 0; i < MAX_OPEN; ++i) {
    RelCacheTable::relCache[i] = nullptr;
    AttrCacheTable::attrCache[i] = nullptr;
  }

  /************ Setting up Relation Cache entries ************/
  // (we need to populate relation cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Relation Cache Table****/
  RecBuffer relCatBlock(RELCAT_BLOCK);

  Attribute relCatRecord[RELCAT_NO_ATTRS];
  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_RELCAT);

  struct RelCacheEntry relCacheEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  relCacheEntry.recId.block = RELCAT_BLOCK;
  relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;

  // allocate this on the heap because we want it to persist outside this function
  RelCacheTable::relCache[RELCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry;

  /**** setting up Attribute Catalog relation in the Relation Cache Table ****/

  // set up the relation cache entry for the attribute catalog similarly
  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_ATTRCAT);
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  relCacheEntry.recId.block = RELCAT_BLOCK;
  relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;

  // from the record at RELCAT_SLOTNUM_FOR_ATTRCAT

  // set the value at RelCacheTable::relCache[ATTRCAT_RELID]
  RelCacheTable::relCache[ATTRCAT_RELID] = new RelCacheEntry;
  *(RelCacheTable::relCache[ATTRCAT_RELID]) = relCacheEntry;


  // Exercise - Students cache
  // relCatBlock.getRecord(relCatRecord, 2);
  // RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  // relCacheEntry.recId.block = RELCAT_BLOCK;
  // relCacheEntry.recId.slot = 2;

  // RelCacheTable::relCache[2] = new RelCacheEntry;
  // *(RelCacheTable::relCache[2]) = relCacheEntry;


  /************ Setting up Attribute cache entries ************/
  // (we need to populate attribute cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Attribute Cache Table ****/
  RecBuffer attrCatBlock(ATTRCAT_BLOCK);

  Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

  // iterate through all the attributes of the relation catalog and create a linked
  struct AttrCacheEntry *attrCacheEntry = new AttrCacheEntry;
  struct AttrCacheEntry *temp = attrCacheEntry;
  for (int i=0; i<RELCAT_NO_ATTRS; i++) {
    // list of AttrCacheEntry (slots 0 to 5)
    attrCatBlock.getRecord(attrCatRecord, i);

    // for each of the entries, set
    AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &temp->attrCatEntry);
    temp->recId.block = ATTRCAT_BLOCK;
    temp->recId.slot = i;

    if (i < RELCAT_NO_ATTRS - 1) temp->next = new AttrCacheEntry;
    else temp->next = nullptr;

    temp = temp->next;
  }
  

  // set the next field in the last entry to nullptr

  AttrCacheTable::attrCache[RELCAT_RELID] = attrCacheEntry;

  /**** setting up Attribute Catalog relation in the Attribute Cache Table ****/

  // set up the attributes of the attribute cache similarly.
  attrCacheEntry = new AttrCacheEntry;
  temp = attrCacheEntry;

  // read slots 6-11 from attrCatBlock and initialise recId appropriately
  for (int i=RELCAT_NO_ATTRS; i<RELCAT_NO_ATTRS+ATTRCAT_NO_ATTRS; i++) {
    // list of AttrCacheEntry (slots 0 to 5)
    attrCatBlock.getRecord(attrCatRecord, i);

    // for each of the entries, set
    AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &temp->attrCatEntry);
    temp->recId.block = ATTRCAT_BLOCK;
    temp->recId.slot = i;

    if (i < RELCAT_NO_ATTRS+ATTRCAT_NO_ATTRS - 1) temp->next = new AttrCacheEntry;
    else temp->next = nullptr;

    temp = temp->next;
  }

  AttrCacheTable::attrCache[ATTRCAT_RELID] = attrCacheEntry;

  // Exercise - Students Table.
  // attrCacheEntry = new AttrCacheEntry;
  // temp = attrCacheEntry;

  // // read slots 12-15 from attrCatBlock and initialise recId appropriately
  // for (int i=12; i<16; i++) {
  //   // list of AttrCacheEntry (slots 0 to 5)
  //   attrCatBlock.getRecord(attrCatRecord, i);

  //   // for each of the entries, set
  //   AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &temp->attrCatEntry);
  //   temp->recId.block = ATTRCAT_BLOCK;
  //   temp->recId.slot = i;

  //   if (i < 16 - 1) temp->next = new AttrCacheEntry;
  //   else temp->next = nullptr;

  //   temp = temp->next;
  // }

  // // set the value at AttrCacheTable::attrCache[2]
  // AttrCacheTable::attrCache[2] = attrCacheEntry;




  /************ Setting up tableMetaInfo entries ************/

  // in the tableMetaInfo array
  //   set free = false for RELCAT_RELID and ATTRCAT_RELID
  tableMetaInfo[RELCAT_RELID].free = false;
  tableMetaInfo[ATTRCAT_RELID].free = false;

  for (int i=2; i<MAX_OPEN; i++) {
    tableMetaInfo[i].free = true;
    // bool val = tableMetaInfo[i].free;
    // if (val == true) std::cout << "true ";
    // else std::cout << "false";
  }
  // std::cout << std::endl;

  //   set relname for RELCAT_RELID and ATTRCAT_RELID
  strcpy(tableMetaInfo[RELCAT_RELID].relName, RELCAT_RELNAME);
  strcpy(tableMetaInfo[ATTRCAT_RELID].relName, ATTRCAT_RELNAME);
}

void freeCacheLL (AttrCacheEntry* head) {
  if (head == nullptr) return;
  freeCacheLL(head->next);
  delete head;
}

OpenRelTable::~OpenRelTable() {
  // close all open relations (from rel-id = 2 onwards. Why?)
  for (int i = 2; i < MAX_OPEN; ++i) {
    if (!tableMetaInfo[i].free) {
      OpenRelTable::closeRel(i); // we will implement this function later
    }
  }

  // free all the memory that you allocated in the constructor
  for (int i=2; i<MAX_OPEN; i++) {
    if (RelCacheTable::relCache[i] != nullptr) delete RelCacheTable::relCache[i];
  }

  for (int i=2; i<MAX_OPEN; i++) freeCacheLL (AttrCacheTable::attrCache[i]);
  

  // STAGE - 8

  if (RelCacheTable::relCache[ATTRCAT_RELID]->dirty == true /* RelCatEntry of the ATTRCAT_RELID-th RelCacheEntry has been modified */) {

        /* Get the Relation Catalog entry from RelCacheTable::relCache
        Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */
        RelCatEntry relCatEntry = RelCacheTable::relCache[ATTRCAT_RELID]->relCatEntry;
        Attribute rec[RELCAT_NO_ATTRS];
        RelCacheTable::relCatEntryToRecord(&relCatEntry, rec);

        // declaring an object of RecBuffer class to write back to the buffer
        RecBuffer relCatBlock(RELCAT_BLOCK);

        // Write back to the buffer using relCatBlock.setRecord() with recId.slot
        relCatBlock.setRecord(rec, ATTRCAT_RELID);
    }
    // free the memory dynamically allocated to this RelCacheEntry
    delete RelCacheTable::relCache[ATTRCAT_RELID];



    //releasing the relation cache entry of the relation catalog

    if(RelCacheTable::relCache[RELCAT_RELID]->dirty == true /* RelCatEntry of the RELCAT_RELID-th RelCacheEntry has been modified */) {

        /* Get the Relation Catalog entry from RelCacheTable::relCache
        Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */
        RelCatEntry relCatEntry = RelCacheTable::relCache[RELCAT_RELID]->relCatEntry;
        Attribute rec[RELCAT_NO_ATTRS];
        RelCacheTable::relCatEntryToRecord(&relCatEntry, rec);

        // declaring an object of RecBuffer class to write back to the buffer
        RecBuffer relCatBlock(RELCAT_BLOCK);

        // Write back to the buffer using relCatBlock.setRecord() with recId.slot
        relCatBlock.setRecord(rec, RELCAT_RELID);
    }
    // free the memory dynamically allocated for this RelCacheEntry
    delete RelCacheTable::relCache[RELCAT_RELID];

    // free the memory allocated for the attribute cache entries of the
    // relation catalog and the attribute catalog
    freeCacheLL (AttrCacheTable::attrCache[RELCAT_RELID]);
    freeCacheLL (AttrCacheTable::attrCache[ATTRCAT_RELID]);
}




// STAGE - 4

/* This function will open a relation having name `relName`.
Since we are currently only working with the relation and attribute catalog, we
will just hardcode it. In subsequent stages, we will loop through all the relations
and open the appropriate one.
*/
int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {

  // if relname is RELCAT_RELNAME, return RELCAT_RELID
  // if (strcmp(relName, RELCAT_RELNAME) == 0) return RELCAT_RELID;

  // if relname is ATTRCAT_RELNAME, return ATTRCAT_RELID
  // if (strcmp(relName, ATTRCAT_RELNAME) == 0) return ATTRCAT_RELID;

  // if relName matches relName in cache[12];
  int index = -1;
  for (int i=0; i<MAX_OPEN; i++) {
    if (RelCacheTable::relCache[i] == nullptr) continue;
    if (strcmp(RelCacheTable::relCache[i]->relCatEntry.relName, relName) == 0) {
      index = i;
    }
  }

  if (index != -1) return index;

  return E_RELNOTOPEN;
}




// STAGE - 5

int OpenRelTable::getFreeOpenRelTableEntry() {

  /* traverse through the tableMetaInfo array,
    find a free entry in the Open Relation Table.*/
  for (int i=0; i<MAX_OPEN; i++) {
    if (tableMetaInfo[i].free == true) {
      return i;
    }
  }

  // if found return the relation id, else return E_CACHEFULL.
  return E_CACHEFULL;
}


int OpenRelTable::openRel(char relName[ATTR_SIZE]) {
  /* the relation `relName` already has an entry in the Open Relation Table */
  // (checked using OpenRelTable::getRelId())
  int res = getRelId(relName);
  
  if(res >= 0 && res < MAX_OPEN){    

    // return that relation id;
    return res;
  }

  /* find a free slot in the Open Relation Table
     using OpenRelTable::getFreeOpenRelTableEntry(). */
  int freeSlot = getFreeOpenRelTableEntry();

  if (freeSlot == E_CACHEFULL){
    return E_CACHEFULL;
  }

  // let relId be used to store the free slot.
  int relId = freeSlot;

  /****** Setting up Relation Cache entry for the relation ******/

  /* search for the entry with relation name, relName, in the Relation Catalog using
      BlockAccess::linearSearch().
      Care should be taken to reset the searchIndex of the relation RELCAT_RELID
      before calling linearSearch().*/
  RelCacheTable::resetSearchIndex(RELCAT_RELID);
  Attribute relNameAttr;
  strcpy(relNameAttr.sVal, relName);

  // relcatRecId stores the rec-id of the relation `relName` in the Relation Catalog.
  RecId relcatRecId = BlockAccess::linearSearch(RELCAT_RELID, RELCAT_ATTR_RELNAME, relNameAttr, EQ);

  if (relcatRecId.block == -1 && relcatRecId.slot == -1) {
    // (the relation is not found in the Relation Catalog.)
    return E_RELNOTEXIST;
  }

  /* read the record entry corresponding to relcatRecId and create a relCacheEntry
      on it using RecBuffer::getRecord() and RelCacheTable::recordToRelCatEntry().
      update the recId field of this Relation Cache entry to relcatRecId.
      use the Relation Cache entry to set the relId-th entry of the RelCacheTable.
    NOTE: make sure to allocate memory for the RelCacheEntry using malloc()
  */
  union Attribute rec[RELCAT_NO_ATTRS];
  RecBuffer buffer(RELCAT_BLOCK);
  buffer.getRecord(rec, relcatRecId.slot);
  // RelCacheEntry relCacheEntry;
  RelCatEntry relCatEntry;
  RelCacheTable::recordToRelCatEntry(rec, &relCatEntry);
  RelCacheTable::relCache[relId] = new RelCacheEntry;
  RelCacheTable::relCache[relId]->recId = relcatRecId;
  RelCacheTable::relCache[relId]->relCatEntry = relCatEntry;
  // std::cout << rec[RELCAT_REL_NAME_INDEX].sVal << " is relName\n";


  /****** Setting up Attribute Cache entry for the relation ******/

  // let listHead be used to hold the head of the linked list of attrCache entries.
  struct AttrCacheEntry *listHead = new AttrCacheEntry;
  struct AttrCacheEntry* temp = listHead;
  /*iterate over all the entries in the Attribute Catalog corresponding to each
  attribute of the relation relName by multiple calls of BlockAccess::linearSearch()
  care should be taken to reset the searchIndex of the relation, ATTRCAT_RELID,
  corresponding to Attribute Catalog before the first call to linearSearch().*/
  RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
  while (true) {
      // std::cout << "got inside while\n";
      /* let attrcatRecId store a valid record id an entry of the relation, relName,
      in the Attribute Catalog.*/
      Attribute relNameAttr;
      strcpy(relNameAttr.sVal, relName);
      RecId attrcatRecId = BlockAccess::linearSearch(ATTRCAT_RELID, ATTRCAT_ATTR_RELNAME, relNameAttr, EQ);

      // std::cout << attrcatRecId.block << " " << attrcatRecId.slot << std::endl;
      if (attrcatRecId.block == -1) break;
      

      /* read the record entry corresponding to attrcatRecId and create an
      Attribute Cache entry on it using RecBuffer::getRecord() and
      AttrCacheTable::recordToAttrCatEntry().
      update the recId field of this Attribute Cache entry to attrcatRecId.
      add the Attribute Cache entry to the linked list of listHead .*/
      // NOTE: make sure to allocate memory for the AttrCacheEntry using malloc()
      union Attribute rec[ATTRCAT_NO_ATTRS];
      RecBuffer buffer(attrcatRecId.block);
      buffer.getRecord(rec, attrcatRecId.slot);
      AttrCatEntry attrCatEntry;
      AttrCacheTable::recordToAttrCatEntry(rec, &attrCatEntry);
      temp->next = new AttrCacheEntry;
      temp = temp->next;
      temp->attrCatEntry = attrCatEntry;
      temp->recId = attrcatRecId;
  }
  temp->next = nullptr;
  temp = listHead;
  if (temp) {
    listHead = listHead->next;
    delete temp;
  }

  // set the relIdth entry of the AttrCacheTable to listHead.
  AttrCacheTable::attrCache[relId] = listHead;

  /****** Setting up metadata in the Open Relation Table for the relation******/

  // update the relIdth entry of the tableMetaInfo with free as false and
  tableMetaInfo[relId].free = false;
  // relName as the input.
  strcpy(tableMetaInfo[relId].relName, relName);

  return relId;
}



int OpenRelTable::closeRel(int relId) {
  if (relId == RELCAT_RELID || relId == ATTRCAT_RELID) {
    return E_NOTPERMITTED;
  }

  if (relId < 0 || relId >= MAX_OPEN) {
    return E_OUTOFBOUND;
  }

  if (tableMetaInfo[relId].free == true) {
    return E_RELNOTOPEN;
  }



  /****** Releasing the Relation Cache entry of the relation ******/

  if (RelCacheTable::relCache[relId]->dirty == true)
  {

    /* Get the Relation Catalog entry from RelCacheTable::relCache
    Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */
    RelCatEntry relCatEntry = RelCacheTable::relCache[relId]->relCatEntry;
    Attribute rec[RELCAT_NO_ATTRS];
    RelCacheTable::relCatEntryToRecord(&relCatEntry, rec);


    // declaring an object of RecBuffer class to write back to the buffer
    RecBuffer relCatBlock(RELCAT_BLOCK);

    // Write back to the buffer using relCatBlock.setRecord() with recId.slot
    RecId recId = RelCacheTable::relCache[relId]->recId;
    relCatBlock.setRecord(rec, recId.slot);
  }

  /****** Releasing the Attribute Cache entry of the relation ******/

  // for all the entries in the linked list of the relIdth Attribute Cache entry.
  for (AttrCacheEntry *p = AttrCacheTable::attrCache[relId]; p != nullptr; p = p->next)
  {
      if (p->dirty == true)
      {
          /* Get the Attribute Catalog entry from attrCache
            Then convert it to a record using AttrCacheTable::attrCatEntryToRecord().
            Write back that entry by instantiating RecBuffer class. Use recId
            member field and recBuffer.setRecord() */
            union Attribute record[ATTRCAT_NO_ATTRS];
            AttrCacheTable::attrCatEntryToRecord(&(p->attrCatEntry), record);
      }

      // free the memory dynamically alloted to this entry in Attribute
      // Cache linked list and assign nullptr to that entry
  }



  /****** Set the Open Relation Table entry of the relation as free ******/



  // free the memory allocated in the relation and attribute caches which was
  // allocated in the OpenRelTable::openRel() function
  if (RelCacheTable::relCache[relId] != nullptr) delete RelCacheTable::relCache[relId];
  freeCacheLL(AttrCacheTable::attrCache[relId]);
  // update `tableMetaInfo` to set `relId` as a free slot
  tableMetaInfo[relId].free = true;

  // update `relCache` and `attrCache` to set the entry at `relId` to nullptr
  RelCacheTable::relCache[relId] = nullptr;
  AttrCacheTable::attrCache[relId] = nullptr;

  return SUCCESS;
}
