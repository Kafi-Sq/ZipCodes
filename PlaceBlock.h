#ifndef PLACEBLOCK_H
#define PLACEBLOCK_H

#include <optional>

#include "BlockBuffer.h"
#include "Place.h"

class BlockBuffer;

class PlaceBlock {
   private:
    int size = -1;
    std::string highestKey = "";

   public:
    BlockHeader blockHeader;
    std::vector<Place> placeBlock;

    PlaceBlock() = default;

    bool unpack(BlockBuffer& bBuf, LengthIndicatedBuffer<BlockFileHeader>& lBuf);
    void merge(PlaceBlock& fromPB);
    std::string getHighestKey();
    std::optional<Place> getRecord(const std::string& key);
    void print();
    void sort();
    int getSize();
};

#endif