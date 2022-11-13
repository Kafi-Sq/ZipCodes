#include "BlockBuffer.h"

BlockBuffer::BlockBuffer(BlockFileHeader &header) {
    this->header = header;
    buffer.resize(header.fileInfo.blockSize - sizeof(BlockHeader));
    blockHeader.recordCount = 0;
}

void BlockBuffer::init(BlockFileHeader &header) {
    this->header = header;
    buffer.resize(header.fileInfo.blockSize - sizeof(BlockHeader));
    blockHeader.recordCount = 0;
}

bool BlockBuffer::read(std::istream &file) {
    if (file.peek() == EOF) {
        return false;
    }
    clear();
    file.read((char *)&blockHeader, sizeof(blockHeader));
    file.read(buffer.data(), header.fileInfo.blockSize - sizeof(blockHeader));
    return true;
}

bool BlockBuffer::read(std::istream &file, int RBN) {
    this->RBN = RBN;
    int offset = header.headerInfo.headerSize + RBN * header.fileInfo.blockSize;
    file.clear();
    file.seekg(offset);
    return read(file);
}

bool BlockBuffer::unpack(LengthIndicatedBuffer<BlockFileHeader> &lBuf) {
    if (blockHeader.recordCount > 0 && unpackedRecords < blockHeader.recordCount) {
        lBuf.read(buffer, curr);
        curr += lBuf.recordLength + header.fileInfo.lengthIndicatorSize;
        length = curr;
        ++unpackedRecords;
        return true;
    }
    return false;
}

void BlockBuffer::pack(PlaceBlock pb, LengthIndicatedBuffer<BlockFileHeader> &lBuf) {
    for (auto rec : pb.placeBlock) {
        rec.pack(lBuf);
        pack(lBuf);
    }
}

void BlockBuffer::pack(LengthIndicatedBuffer<BlockFileHeader> &lBuf) {
    lBuf.write(buffer, length);
    length += lBuf.recordLength + lBuf.header.fileInfo.lengthIndicatorSize;
    ++blockHeader.recordCount;
    // lBuf.clear();
}

void BlockBuffer::write(std::ostream &file, int RBN) {
    int offset = header.headerInfo.headerSize + RBN * header.fileInfo.blockSize;
    file.seekp(offset);
    file.clear();
    file.write((char *)&blockHeader, sizeof(blockHeader));
    file.write(buffer.data(), buffer.size());
    file.flush();
    return;
}

/**
 * @brief
 *
 * @pre buffer contains block of records
 */
void BlockBuffer::processBlockHeader() {
    // std::copy(buffer.begin(), buffer.begin() + sizeof(blockHeader), blockHeader);
    memcpy((char *)&blockHeader, buffer.data(), sizeof(blockHeader));
    unpackedRecords = blockHeader.recordCount;
}

void BlockBuffer::setBlockHeader(BlockHeader blockHeader){
    this->blockHeader = blockHeader;
}

void BlockBuffer::setPrecedingBlock(int RBN) {
    blockHeader.precedingBlock = RBN;
}
void BlockBuffer::setSucceedingBlock(int RBN) {
    blockHeader.succeedingBlock = RBN;
}

void BlockBuffer::clear() {
    curr = length = 0;
    unpackedRecords = 0;
    blockHeader.recordCount = 0;
    buffer.assign(buffer.size(), 0);
}