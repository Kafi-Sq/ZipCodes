#include "PlaceBlock.h"
#include <algorithm>
#include "PrimaryKey.h"
using namespace std;

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

string PlaceBlock::getHighestKey() {
    string max = "";
    for (auto rec : placeBlock) {
        if (CompareStr(max, rec.getZipCode())) {
            max = rec.getZipCode();
        }
    }
    return max;
}

void PlaceBlock::sort() {
    sort(placeBlock.begin(), placeBlock.end());
}

optional<Place> PlaceBlock::getRecord(const string &key) {
    for (auto rec : placeBlock) {
        if (rec.getZipCode() == key) {
            return rec;
        }
    }
    return {};
}

void PlaceBlock::print() {
    cout << "Records in place block: " << endl;
    for (auto r : placeBlock) {
        cout << "\t";
        r.print();
    }
}