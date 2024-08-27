#include <iostream>
#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"

#include <cstring>

// Stage - I
void printBlockAllocationMap(unsigned char *buffer)
{
  Disk::readBlock(buffer, 0);
  for (int i = 0; i < 9; i++)
  {
    std::cout << i << " th position has value " << (int)buffer[i] << std::endl;
  }
}

void stage2 () {
  // stage - II

  // creating objects to read relation and attribute catalogs
  RecBuffer relCatBuffer(RELCAT_BLOCK);
  RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

  HeadInfo relCatHeader;
  HeadInfo attrCatHeader;

  // loading the headers from the respective class
  relCatBuffer.getHeader(&relCatHeader);

  Attribute attrRow[RELCAT_NO_ATTRS];
  relCatBuffer.getRecord(attrRow, 1);

  // changing attribute name

  for (int i = 0; i < relCatHeader.numEntries; i++)
  {
    Attribute relCatRecord[RELCAT_NO_ATTRS];

    relCatBuffer.getRecord(relCatRecord, i);

    std::cout << "Relation: " << relCatRecord[RELCAT_REL_NAME_INDEX].sVal << std::endl;

    // RecBuffer attrCatBuffer(ATTRCAT_BLOCK);
    // attrCatBuffer.getHeader(&attrCatHeader);

    // int attrNums = relCatRecord[RELCAT_NO_ATTRIBUTES_INDEX].nVal;
    // int attrEntries = attrCatHeader.numEntries;

    int headerNum = ATTRCAT_BLOCK;

    while (headerNum != -1)
    {
      RecBuffer attrCatBuffer(headerNum);
      attrCatBuffer.getHeader(&attrCatHeader);

      for (int j = 0; j < attrCatHeader.numEntries; j++)
      {
        Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
        attrCatBuffer.getRecord(attrCatRecord, j);

        const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";

        if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, relCatRecord[RELCAT_REL_NAME_INDEX].sVal) == 0)
        {
          std::cout << "  " << attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal << ": " << attrType << std::endl;
        }
      }

      headerNum = attrCatHeader.rblock;
    }
    std::cout << std::endl;
  }
}

void stage3 () {
  // Note: relId is 0,1 for rel, attr tables respectively

  for (int relId=0; relId<3; relId++) {
    RelCatEntry RelCatBuf;
    RelCacheTable::getRelCatEntry(relId, &RelCatBuf);

    std::cout << "Relation: " << RelCatBuf.relName << std::endl;

    for (int i=0; i<RelCatBuf.numAttrs; i++) {
      AttrCatEntry AttrCatBuf;
      AttrCacheTable::getAttrCatEntry(relId, i, &AttrCatBuf);
      const char *attrType = AttrCatBuf.attrType == NUMBER ? "NUM" : "STR";
      std::cout << "  " << AttrCatBuf.attrName << ": " << attrType << std::endl;
    }

    std::cout << std::endl;
  }
}

int main(int argc, char *argv[])
{
  /* Initialize the Run Copy of Disk */
  Disk disk_run;
  StaticBuffer buffer;
  OpenRelTable cache;

  // unsigned char buffer[BLOCK_SIZE];
  // Disk::readBlock(buffer, 7000);
  // char message[] = "hello";
  // memcpy(buffer + 20, message, 6);
  // Disk::writeBlock(buffer, 7000);

  // unsigned char buffer2[BLOCK_SIZE];
  // char message2[16];
  // Disk::readBlock(buffer2, 7000);
  // memcpy(message2, buffer2+20, 6);
  // std::cout << message2;
  // std::cout << "\n";

  // stage - I
  // printBlockAllocationMap (buffer);

  // stage2();
  // stage3();

  // return 0;
  // StaticBuffer buffer;
  // OpenRelTable cache;

  
  return FrontendInterface::handleFrontend(argc, argv);
}
