#include "StaticBuffer.h"

#include <cstring>

unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];
unsigned char StaticBuffer::blockAllocMap[DISK_BLOCKS];


StaticBuffer::StaticBuffer() {
  // copy blockAllocMap blocks from disk to buffer (using readblock() of disk)
  // blocks 0 to 3
  unsigned char block[BLOCK_SIZE];
  for (int blockNum = 0; blockNum < 4; blockNum++)
  {
      Disk::readBlock(block, blockNum);
      memcpy(blockAllocMap + blockNum * BLOCK_SIZE, block, BLOCK_SIZE);
  }

  // initialise all blocks as free

  for (int bufferIndex=0; bufferIndex<BUFFER_CAPACITY; bufferIndex++) {
    metainfo[bufferIndex].free = true;
    metainfo[bufferIndex].dirty = false;
    metainfo[bufferIndex].timeStamp = -1;
    metainfo[bufferIndex].blockNum = -1;
  }
}


StaticBuffer::~StaticBuffer() {
  // copy blockAllocMap blocks from buffer to disk(using writeblock() of disk)
  for (int blockNum = 0; blockNum < 4; blockNum++)
  {
      Disk::writeBlock(blockAllocMap + blockNum * BLOCK_SIZE, blockNum);
  }

  /*iterate through all the buffer blocks,
    write back blocks with metainfo as free=false,dirty=true
    using Disk::writeBlock()
    */
  for (int bufferIndex=0; bufferIndex<BUFFER_CAPACITY; bufferIndex++) {
    if (metainfo[bufferIndex].dirty == true && metainfo[bufferIndex].free == false) {
      Disk::writeBlock(blocks[bufferIndex], metainfo[bufferIndex].blockNum);
    }
  }
}


int StaticBuffer::getFreeBuffer(int blockNum) {
  if (blockNum < 0 || blockNum > DISK_BLOCKS) {
    return E_OUTOFBOUND;
  }
  int allocatedBuffer = -1;

  // increase the timeStamp in metaInfo of all occupied buffers.
  for (int bufferIndex=0; bufferIndex<BUFFER_CAPACITY; bufferIndex++) {
    if (metainfo[bufferIndex].free == true) {
      metainfo[bufferIndex].timeStamp++;
    }
  }

  // let bufferNum be used to store the buffer number of the free/freed buffer.
  int bufferNum;

  // iterate through all the blocks in the StaticBuffer
  for (int bufferIndex=0; bufferIndex<BUFFER_CAPACITY; bufferIndex++) {

    // find the first free block in the buffer (check metainfo)
    if (metainfo[bufferIndex].free == true) {
      
      // assign allocatedBuffer = index of the free block
      allocatedBuffer = bufferIndex;
      break;
    }
  }

  // if a free buffer is not available,
  if (allocatedBuffer == -1) {
    // find the buffer with the largest timestamp
    int max = -2;
    for (int bufferIndex=0; bufferIndex<BUFFER_CAPACITY; bufferIndex++) {
      if (metainfo[bufferIndex].timeStamp > max) {
        max = metainfo[bufferIndex].timeStamp;
        allocatedBuffer = bufferIndex;
      }
    }

    // IF IT IS DIRTY, write back to the disk using Disk::writeBlock()
    if (metainfo[allocatedBuffer].dirty == true) {
      Disk::writeBlock(blocks[allocatedBuffer], metainfo[allocatedBuffer].blockNum);
    }

    // update the metaInfo entry corresponding to bufferNum with
    // free:false, dirty:false, blockNum:the input block number, timeStamp:0.
    metainfo[allocatedBuffer].dirty = false;
    metainfo[allocatedBuffer].timeStamp = 0;
  }

  metainfo[allocatedBuffer].free = false;
  metainfo[allocatedBuffer].blockNum = blockNum;

  return allocatedBuffer;
}


int StaticBuffer::getBufferNum(int blockNum) {
  // Check if blockNum is valid (between zero and DISK_BLOCKS) and return E_OUTOFBOUND if not valid.
  if (blockNum < 0 || blockNum > DISK_BLOCKS) {
    return E_OUTOFBOUND;
  }

  // find and return the bufferIndex which corresponds to blockNum (check metainfo)
  for (int bufferIndex=0; bufferIndex<BUFFER_CAPACITY; bufferIndex++) {
    if (metainfo[bufferIndex].blockNum == blockNum) return bufferIndex;
  }

  // if block is not in the buffer
  return E_BLOCKNOTINBUFFER;
}


int StaticBuffer::setDirtyBit(int blockNum){
    // find the buffer index corresponding to the block using getBufferNum().
    int bufferIndex = getBufferNum(blockNum);

    // if block is not present in the buffer (bufferNum = E_BLOCKNOTINBUFFER)
    //     return E_BLOCKNOTINBUFFER
    if (bufferIndex == E_BLOCKNOTINBUFFER) return E_BLOCKNOTINBUFFER;

    // if blockNum is out of bound (bufferNum = E_OUTOFBOUND)
    //     return E_OUTOFBOUND
    if (bufferIndex == E_OUTOFBOUND) return E_OUTOFBOUND;

    // else
    //     (the bufferNum is valid)
    //     set the dirty bit of that buffer to true in metainfo
    metainfo[bufferIndex].dirty = true;

    return SUCCESS;
}



// STAGE - 10

int StaticBuffer::getStaticBlockType(int blockNum){
    // Check if blockNum is valid (non zero and less than number of disk blocks)
    // and return E_OUTOFBOUND if not valid.
    if (blockNum < 0 || blockNum >= DISK_BLOCKS) return E_OUTOFBOUND;

    // Access the entry in block allocation map corresponding to the blockNum argument
    int type = (int) blockAllocMap[blockNum];

    // and return the block type after type casting to integer.
    return type;
}