#include "BlockBuffer.h"

#include <cstdlib>
#include <cstring>

#include <iostream>

BlockBuffer::BlockBuffer(int blockNum)
{
  this->blockNum = blockNum;
}

// record buffer is intialized just by calling the parent(blockBuffer) constructor
RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum) {}

int BlockBuffer::getHeader(struct HeadInfo *head)
{
  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }

  // std::cout << this->blockNum << " is the block num\n";

  // unsigned char buffer[BLOCK_SIZE];
  // Disk::readBlock(buffer, blockNum);

  memcpy(&head->blockType, bufferPtr, 4);
  memcpy(&head->pblock, bufferPtr + 4, 4);
  memcpy(&head->lblock, bufferPtr + 8, 4);
  memcpy(&head->rblock, bufferPtr + 12, 4);
  memcpy(&head->numEntries, bufferPtr + 16, 4);
  memcpy(&head->numAttrs, bufferPtr + 20, 4);
  memcpy(&head->numSlots, bufferPtr + 24, 4);
  memcpy(&head->reserved, bufferPtr + 28, 4);

  return SUCCESS;
}

int RecBuffer::getRecord(union Attribute *rec, int slotNum)
{
  struct HeadInfo head;
  this->getHeader(&head);

  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }

  int attrCount = head.numAttrs;
  int slotCount = head.numSlots;

  // if input slotNum is not in the permitted range return E_OUTOFBOUND.
  if (slotNum < 0 || slotNum > slotCount) return E_OUTOFBOUND;

  // reading block to buffer
  // unsigned char buffer[BLOCK_SIZE];
  // Disk::readBlock(buffer, blockNum);

  int recordSize = attrCount * ATTR_SIZE;
  int slotMapSize = slotCount;

  // load the record(row) into rec array
  memcpy(rec, bufferPtr + HEADER_SIZE + slotMapSize + slotNum * recordSize, recordSize);

  return SUCCESS;
}

// int RecBuffer::setRecord(union Attribute *rec, int slotNum) {
//   unsigned char buffer[BLOCK_SIZE];
// }

int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char **buffPtr)
{
  // check whether the block is already present in the buffer using StaticBuffer.getBufferNum()
  int bufferNum = StaticBuffer::getBufferNum(this->blockNum);

  if (bufferNum != E_BLOCKNOTINBUFFER) { // if present
    // and increment the timestamps of all other occupied buffers in BufferMetaInfo.
    for (int i=0; i<BUFFER_CAPACITY; i++) {
      StaticBuffer::metainfo[i].timeStamp++;
    }

    // set the timestamp of the corresponding buffer to 0
    StaticBuffer::metainfo[bufferNum].timeStamp = 0;
  }

  else
  {
    // get a free buffer using StaticBuffer.getFreeBuffer()
    bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);

    if (bufferNum == E_OUTOFBOUND)
    {
      return E_OUTOFBOUND;
    }
    // std::cout << "block not in buffer " << this->blockNum <<std::endl;

    Disk::readBlock(StaticBuffer::blocks[bufferNum], this->blockNum);
  }

  // store the pointer to this buffer (blocks[bufferNum]) in *buffPtr
  *buffPtr = StaticBuffer::blocks[bufferNum];

  return SUCCESS;
}



// STAGE - 4


/* used to get the slotmap from a record block
NOTE: this function expects the caller to allocate memory for `*slotMap`
*/
int RecBuffer::getSlotMap(unsigned char *slotMap) {
  unsigned char *bufferPtr;

  // get the starting address of the buffer containing the block using loadBlockAndGetBufferPtr().
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }

  struct HeadInfo head;
  // get the header of the block using getHeader() function
  getHeader(&head);

  int slotCount = head.numSlots;

  // get a pointer to the beginning of the slotmap in memory by offsetting HEADER_SIZE
  unsigned char *slotMapInBuffer = bufferPtr + HEADER_SIZE;

  // copy the values from `slotMapInBuffer` to `slotMap` (size is `slotCount`)
  memcpy (slotMap, slotMapInBuffer, slotCount);

  return SUCCESS;
}


int compareAttrs(Attribute attr1, Attribute attr2, int attrType) {
  
  // if (op == NUMBER) {
  //   double val1 = attr1.nVal;
  //   double val2 = attr2.nVal;
  //   std::cout << "records: " << attr1.nVal << " " << attr2.nVal << std::endl;

  //   switch (op) {
  //     case EQ : return val1 == val2 ? 1 : 0; break;
  //     case LE : return val1 <= val2 ? 1 : 0; break;
  //     case LT : return val1 <  val2 ? 1 : 0; break;
  //     case GE : return val1 >= val2 ? 1 : 0; break;
  //     case GT : return val1 >  val2 ? 1 : 0; break;
  //     case NE : return val1 != val2 ? 1 : 0; break;
  //   }
  // }
  // else {
  //   int res = strcmp (attr1.sVal, attr2.sVal);
  //   switch (op) {
  //     case EQ : return res == 0 ? 1 : 0; break;
  //     case LE : return res <= 0 ? 1 : 0; break;
  //     case LT : return res <  0 ? 1 : 0; break;
  //     case GE : return res >= 0 ? 1 : 0; break;
  //     case GT : return res >  0 ? 1 : 0; break;
  //     case NE : return res != 0 ? 1 : 0; break;
  //   }
  // }

  double diff = attrType == STRING ? strcmp(attr1.sVal, attr2.sVal) : attr1.nVal - attr2.nVal;
  /*
  if diff > 0 then return 1
  if diff < 0 then return -1
  if diff = 0 then return 0
  */
  if (diff > 0) return 1;
  if (diff < 0) return -1;

  return 0;
}



// STAGE - 6

int RecBuffer::setRecord(union Attribute *rec, int slotNum) {
    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
    int ret = loadBlockAndGetBufferPtr(&bufferPtr);

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.
    if (ret != SUCCESS) return ret;

    /* get the header of the block using the getHeader() function */
    HeadInfo head;
    getHeader(&head);

    // get number of attributes in the block.
    int attrCount = head.numAttrs;

    // get the number of slots in the block.
    int slotCount = head.numSlots;

    // if input slotNum is not in the permitted range return E_OUTOFBOUND.
    if (slotNum < 0 || slotNum > slotCount) return E_OUTOFBOUND;

    /* offset bufferPtr to point to the beginning of the record at required
       slot. the block contains the header, the slotmap, followed by all
       the records. so, for example,
       record at slot x will be at bufferPtr + HEADER_SIZE + (x*recordSize)
       copy the record from `rec` to buffer using memcpy
       (hint: a record will be of size ATTR_SIZE * numAttrs)
    */
    int recordSize = attrCount * ATTR_SIZE;
    bufferPtr += HEADER_SIZE + slotCount + recordSize*slotNum;
    memcpy(bufferPtr, rec, recordSize);

    // update dirty bit using setDirtyBit()
    StaticBuffer::setDirtyBit(blockNum);

    /* (the above function call should not fail since the block is already
       in buffer and the blockNum is valid. If the call does fail, there
       exists some other issue in the code) */

    return SUCCESS;
}



// STAGE - 7

int BlockBuffer::setHeader(struct HeadInfo *head){

    unsigned char *bufferPtr;
    // get the starting address of the buffer containing the block using
    // loadBlockAndGetBufferPtr(&bufferPtr).
    int ret = loadBlockAndGetBufferPtr(&bufferPtr);

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
    if (ret != SUCCESS) {
        // return the value returned by the call.
        return ret;
    }

    // cast bufferPtr to type HeadInfo*
    struct HeadInfo *bufferHeader = (struct HeadInfo *)bufferPtr;

    // copy the fields of the HeadInfo pointed to by head (except reserved) to
    // the header of the block (pointed to by bufferHeader)
    //(hint: bufferHeader->numSlots = head->numSlots )
    bufferHeader->blockType = head->blockType;
    bufferHeader->lblock = head->lblock;
    bufferHeader->numAttrs = head->numAttrs;
    bufferHeader->numEntries = head->numEntries;
    bufferHeader->numSlots = head->numSlots;
    bufferHeader->pblock = head->pblock;
    bufferHeader->rblock = head->rblock;
    bufferHeader->lblock = head->lblock;
    // bufferHeader

    // update dirty bit by calling StaticBuffer::setDirtyBit()
    ret = StaticBuffer::setDirtyBit(blockNum);

    // if setDirtyBit() failed, return the error code
    if (ret != SUCCESS) return ret;

    return SUCCESS;
}


int BlockBuffer::setBlockType(int blockType){

    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
    int ret = loadBlockAndGetBufferPtr(&bufferPtr);

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.
    if (ret != SUCCESS) return ret;

    // store the input block type in the first 4 bytes of the buffer.
    // (hint: cast bufferPtr to int32_t* and then assign it)
    // *((int32_t *)bufferPtr) = blockType;
    *((int32_t *)bufferPtr) = blockType;

    // update the StaticBuffer::blockAllocMap entry corresponding to the
    // object's block number to `blockType`.
    StaticBuffer::blockAllocMap[blockNum] = blockType;
    

    // update dirty bit by calling StaticBuffer::setDirtyBit()
    ret = StaticBuffer::setDirtyBit(blockNum);

    // if setDirtyBit() failed
        // return the returned value from the call
    if (ret != SUCCESS) return ret;

    return SUCCESS;
}


int BlockBuffer::getFreeBlock(int blockType){
    // iterate through the StaticBuffer::blockAllocMap and find the block number
    // of a free block in the disk.
    int freeBlock;
    for (freeBlock = 0; freeBlock < DISK_BLOCKS; freeBlock++) {
      if (StaticBuffer::blockAllocMap[freeBlock] == UNUSED_BLK) break;
      // std:: cout << freeBlock << " " << (int)StaticBuffer::blockAllocMap[freeBlock] << std::endl;
    }

    // if no block is free, return E_DISKFULL.
    if (freeBlock == DISK_BLOCKS) return E_DISKFULL;


    // set the object's blockNum to the block number of the free block.
    blockNum = freeBlock;

    // find a free buffer using StaticBuffer::getFreeBuffer() .
    int bufferIndex = StaticBuffer::getFreeBuffer(blockNum);

    // initialize the header of the block passing a struct HeadInfo with values
    // pblock: -1, lblock: -1, rblock: -1, numEntries: 0, numAttrs: 0, numSlots: 0
    // to the setHeader() function.
    HeadInfo head;
    head.pblock = head.lblock = head.rblock = -1;
    head.numEntries = head.numAttrs = head.numSlots = 0;
    head.blockType = blockType;
    setHeader(&head);
    StaticBuffer::blockAllocMap[blockNum] = blockType;
    // update the block type of the block to the input block type using setBlockType().
    // setBlockType(REC);

    // return block number of the free block.
    return blockNum;
}


BlockBuffer::BlockBuffer(char blockType){
    // allocate a block on the disk and a buffer in memory to hold the new block of
    // given type using getFreeBlock function and get the return error codes if any.
    int blockTypeInt = UNUSED_BLK;
    if (blockType == 'R') blockTypeInt = REC;
    // else if (blockType ==)
    int freeBlock = getFreeBlock(blockTypeInt);
    // if (freeBlock < 0) return; // Error - Blocks Full

    // set the blockNum field of the object to that of the allocated block
    // number if the method returned a valid block number,
    // otherwise set the error code returned as the block number.
    blockNum = freeBlock;

    // (The caller must check if the constructor allocatted block successfully
    // by checking the value of block number field.)
}


RecBuffer::RecBuffer() : BlockBuffer('R'){}
// call parent non-default constructor with 'R' denoting record block.


int RecBuffer::setSlotMap(unsigned char *slotMap) {
    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block using
       loadBlockAndGetBufferPtr(&bufferPtr). */
    int ret = loadBlockAndGetBufferPtr(&bufferPtr);

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.
    if (ret != SUCCESS) return ret;

    // get the header of the block using the getHeader() function
    HeadInfo head;
    getHeader(&head);

    int numSlots = head.numSlots /* the number of slots in the block */;

    // the slotmap starts at bufferPtr + HEADER_SIZE. Copy the contents of the
    // argument `slotMap` to the buffer replacing the existing slotmap.
    // Note that size of slotmap is `numSlots`
    memcpy(bufferPtr + HEADER_SIZE, slotMap, numSlots);

    // update dirty bit using StaticBuffer::setDirtyBit
    ret = StaticBuffer::setDirtyBit(blockNum);

    // if setDirtyBit failed, return the value returned by the call
    if (ret != SUCCESS) return ret;

    // return SUCCESS
    return SUCCESS;
}


int BlockBuffer::getBlockNum(){

    //return corresponding block number.
    return blockNum;
}



// STAGE - 8

void BlockBuffer::releaseBlock(){

    // if blockNum is INVALID_BLOCKNUM (-1), or it is invalidated already, do nothing
    if (blockNum == INVALID_BLOCKNUM) return;

    /* get the buffer number of the buffer assigned to the block
        using StaticBuffer::getBufferNum().
        (this function return E_BLOCKNOTINBUFFER if the block is not
        currently loaded in the buffer)
    */
    int bufferIndex = StaticBuffer::getBufferNum(blockNum);

    // if the block is present in the buffer, free the buffer
    // by setting the free flag of its StaticBuffer::tableMetaInfo entry
    // to true.
    if (bufferIndex >= 0) {
      StaticBuffer::metainfo[bufferIndex].free = true;
    }

    // free the block in disk by setting the data type of the entry
    // corresponding to the block number in StaticBuffer::blockAllocMap
    // to UNUSED_BLK.
    StaticBuffer::blockAllocMap[blockNum] = UNUSED_BLK;

    // set the object's blockNum to INVALID_BLOCK (-1)
    blockNum = INVALID_BLOCKNUM;
}