#ifndef BLOCKBUFFER_H
#define BLOCKBUFFER_H

#include <vector>

#include "LengthIndicatedBuffer.h"
#include "Place.h"
#include "PlaceBlock.h"

class PlaceBlock;

class BlockBuffer {
   private:
    bool read(std::istream &file);

   public:
    static const int noSuccessor = -1;
    std::vector<char> buffer;
    BlockFileHeader header;
    BlockHeader blockHeader;
    int unpackedRecords = 0;
    int curr = 0;
    int length = 0;

    int RBN;

    void processBlockHeader();
    void setBlockHeader(BlockHeader blockHeader);

   public:
    BlockBuffer() = default;
    BlockBuffer(BlockFileHeader &header);
    bool read(std::istream &file, int RBN);
    void write(std::ostream &file, int RBN);
    bool unpack(LengthIndicatedBuffer<BlockFileHeader> &lBuf);
    void pack(PlaceBlock pb, LengthIndicatedBuffer<BlockFileHeader> &lBuf);
    void pack(LengthIndicatedBuffer<BlockFileHeader> &lBuf);

    void setPrecedingBlock(int RBN);
    void setSucceedingBlock(int RBN);
    void clear();

    void init(BlockFileHeader &header);
};

#endif