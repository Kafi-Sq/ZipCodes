#include "PlaceBlock.h"

#include <algorithm>

#include "PrimaryKey.h"

bool PlaceBlock::unpack(BlockBuffer &bBuf, LengthIndicatedBuffer<BlockFileHeader> &lBuf) {
    blockHeader = bBuf.blockHeader;
    while (bBuf.unpack(lBuf)) {
        Place p;
        p.unpack(lBuf);
        placeBlock.push_back(p);
    }
    return true;
}

int PlaceBlock::getSize() {
    if (size < 0) {
        size = 0;
    }
    for (auto rec : placeBlock) {
        size += rec.getSize();
    }
    return size;
}

std::string PlaceBlock::getHighestKey() {
    std::string max = "";
    for (auto rec : placeBlock) {
        if (CompareStr(max, rec.getZipCode())) {
            max = rec.getZipCode();
        }
    }
    std::cout << max << std::endl;
    return max;
    // return highest->getZipCode();
}

void PlaceBlock::sort() {
    std::sort(placeBlock.begin(), placeBlock.end());
}

std::optional<Place> PlaceBlock::getRecord(const std::string &key) {
    for (auto rec : placeBlock) {
        if (rec.getZipCode() == key) {
            return rec;
        }
    }
    return {};
}

void PlaceBlock::print() {
    std::cout << "Records in place block: " << std::endl;
    for (auto r : placeBlock) {
        std::cout << "\t";
        r.print();
    }
}

void PlaceBlock::merge(PlaceBlock &fromPB) {
    // std::vector<Place> temp;
    // for (auto rec : placeBlock) {
    //     if (CompareStr(rec.getZipCode(), fromPB.getZipCode())) {
    //         rec.pack(writeRecBuf);
    //     } else {
    //         place.pack(writeRecBuf);
    //     }
    // }
}