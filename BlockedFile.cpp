#include "BlockedFile.h"

#include <sstream>

#include "HeaderBuffer.h"

BlockedFile::BlockedFile(std::string fileName) : fileName(fileName), readRecBuf(MAGIC_HEADER_NUMBER), writeRecBuf(MAGIC_HEADER_NUMBER) {
    if (std::filesystem::exists(fileName)) {
        openDataFile();
        initializeBuffers();
    } else {
        throw;
    }

    //     dataStartRBN = header.headerInfo.headerSize;
    //     initializeIndex();
}

bool BlockedFile::openDataFile() {
    if (std::filesystem::exists(fileName)) {
        file.open(fileName, std::ios::binary | std::ios::in | std::ios::out);
    } else {
        file.open(fileName, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
    }
    return file.good();
}

void BlockedFile::initializeBuffers() {
    readRecBuf.init(file);
    writeRecBuf.init(file);
    this->header = readRecBuf.header;
    readBlockBuf.init(this->header);
    writeBlockBuf.init(this->header);
}

void BlockedFile::initializeIndex() {
    if (indexFileExists()) {
        index.ReadIndexFile(header.fileInfo.indexFileName);
    } else {
        generateIndex();
    }
}

void BlockedFile::generateIndex() {
    auto startRBN = header.fileInfo.activeBlock;

    int currRBN = startRBN;
    while (currRBN != BlockBuffer::noSuccessor) {
        PlaceBlock pb;
        readBlockBuf.read(file, currRBN);
        pb.unpack(readBlockBuf, readRecBuf);
        std::cout << pb.getHighestKey() << std::endl;
        index.Add({pb.getHighestKey(), currRBN});
        currRBN = readBlockBuf.blockHeader.succeedingBlock;
    }
    index.GenerateIndexFile(header.fileInfo.indexFileName);
}

bool BlockedFile::indexFileExists() {
    return std::filesystem::exists(header.fileInfo.indexFileName);
}

void BlockedFile::splitBlock(int RBN, Place newRecord) {
    PlaceBlock pbOrig;
    PlaceBlock stay;
    PlaceBlock move;
    readBlockBuf.read(file, RBN);

    file.clear();
    pbOrig.unpack(readBlockBuf, readRecBuf);

    pbOrig.placeBlock.push_back(newRecord);
    pbOrig.blockHeader.recordCount++;
    pbOrig.sort();

    int halfSize = pbOrig.getSize() / 2;
    int total = 0;
    for (auto rec : pbOrig.placeBlock) {
        if ((total += rec.getSize()) > halfSize) {
            move.placeBlock.push_back(rec);
        } else {
            stay.placeBlock.push_back(rec);
        }
    }

    readBlockBuf.read(file, availBlockRBN);

    auto nextAvailRBN = readBlockBuf.blockHeader.succeedingBlock;
    if (nextAvailRBN == BlockBuffer::noSuccessor) {
        header.fileInfo.blockCount += 1;
        addEmptyBlock(header.fileInfo.blockCount, {0, -1, -1});
        header.fileInfo.availBlock = header.fileInfo.blockCount;
        // writeHeader();
    } else {
        header.fileInfo.availBlock = nextAvailRBN;
    }

    writeBlockBuf.blockHeader.recordCount = stay.placeBlock.size();
    writeBlockBuf.blockHeader.precedingBlock = pbOrig.blockHeader.precedingBlock;
    writeBlockBuf.blockHeader.succeedingBlock = pbOrig.blockHeader.succeedingBlock;
    writeBlockBuf.pack(stay, writeRecBuf);
    writeBlockBuf.write(file, RBN);
    writeBlockBuf.clear();

    // set new block's predecessor to this one
    writeBlockBuf.setPrecedingBlock(RBN);
    // set new block's successor to the successor of this one
    writeBlockBuf.setSucceedingBlock(pbOrig.blockHeader.succeedingBlock);
    writeBlockBuf.pack(move, writeRecBuf);
    writeBlockBuf.write(file, availBlockRBN);
}

void BlockedFile::addEmptyBlock(int RBN, BlockHeader blockHeader) {
    writeBlockBuf.blockHeader = blockHeader;
    writeBlockBuf.clear();
    writeBlockBuf.write(file, RBN);
}

void BlockedFile::addRecord(Place place) {
    static int c = 0;
    PlaceBlock pb;
    auto RBN = index.Find(place.getZipCode());

    if (RBN == PrimaryKey::notFound) {
        RBN = header.fileInfo.activeBlock;
    }
    // TODO if index returns not found, add record to last block

    // if file has block
    if (readBlockBuf.read(file, RBN)) {
        file.clear();
        pb.unpack(readBlockBuf, readRecBuf);
    }

    if (pb.getRecord(place.getZipCode())) {
        return;
    }

    // if record won't fit
    auto newBufLength = readBlockBuf.length + sizeof(readBlockBuf.blockHeader) + place.getSize() + header.fileInfo.lengthIndicatorSize;
    if (newBufLength > header.fileInfo.blockSize) {
        splitBlock(RBN, place);
    }

    // if block is empty
    else if (pb.getSize() == 0) {
        writeBlockBuf.setPrecedingBlock(readBlockBuf.blockHeader.precedingBlock);
        writeBlockBuf.setSucceedingBlock(readBlockBuf.blockHeader.succeedingBlock);
        place.pack(writeRecBuf);
        writeBlockBuf.pack(writeRecBuf);
        writeBlockBuf.write(file, RBN);
        writeBlockBuf.clear();
    }

    // if block has records, find the correct place
    else {
        writeBlockBuf.setPrecedingBlock(readBlockBuf.blockHeader.precedingBlock);
        writeBlockBuf.setSucceedingBlock(readBlockBuf.blockHeader.succeedingBlock);
        pb.placeBlock.push_back(place);
        pb.sort();
        writeBlockBuf.pack(pb, writeRecBuf);
        writeBlockBuf.write(file, RBN);
        writeBlockBuf.clear();
    }
}

bool BlockedFile::deleteRecord(const std::string& recordKey) {
    auto RBN = index.BinarySearch(recordKey);
    if (RBN == PrimaryKey::notFound) {
        return false;
    }

    PlaceBlock pb;
    readBlockBuf.read(file, RBN);
    pb.unpack(readBlockBuf, readRecBuf);

    auto recordOpt = pb.getRecord(recordKey);

    if (!recordOpt) {
        return false;
    }

    int sizeAfter = pb.getSize() - recordOpt->getSize();
    int halfSize = header.fileInfo.blockSize / 2;
    if (sizeAfter < halfSize) {
        // TODO, do thing here
    }
    return true;
}

void BlockedFile::mergeBlocks(int fromRBN, int toRBN) {
    PlaceBlock fromPB, toPB;

    readBlockBuf.read(file, fromRBN);
    fromPB.unpack(readBlockBuf, readRecBuf);

    readBlockBuf.read(file, toRBN);
    toPB.unpack(readBlockBuf, readRecBuf);

    toPB.merge(fromPB);

    writeBlockBuf.pack(toPB, readRecBuf);
    writeBlockBuf.write(file, toRBN);
}

void BlockedFile::redistribute(int RBN) {
    readBlockBuf.read(file, RBN);
}

void BlockedFile::printBlock(int RBN) {
    readBlockBuf.read(file, RBN);
    PlaceBlock pb;
    pb.unpack(readBlockBuf, readRecBuf);

    std::cout << readBlockBuf.blockHeader.precedingBlock << " ";
    if (readBlockBuf.blockHeader.recordCount > 0) {
        for (auto rec : pb.placeBlock) {
            std::cout << rec.getZipCode() << " ";
        }
    } else {
        std::cout << "*available* ";
    }
    std::cout << readBlockBuf.blockHeader.succeedingBlock << std::endl;
}

BlockFileHeader BlockedFile::writeHeader() {
    HeaderBuffer<BlockFileHeader> hb;
    BlockFileHeader currentHeader;
    
    hb.read(file);
    hb.unpack(currentHeader);

    // if (currentHeader.headerInfo.headerSize != header.headerInfo.headerSize) {
    //     std::cerr << "header size is wrong" << std::endl;
    //     exit(1);
    // }
    file.seekp(0);

    file << header;
    file.flush();
    file.clear();
}

void BlockedFile::createFromLIFile(std::string fileName, LengthIndicatedFile& liFile) {
    std::fstream file(fileName, std::ios::out | std::ios::trunc);
    BlockFileHeader bfHeader(liFile.header);

    auto indexFileName = fileName.substr(0, 96) + ".idx";
    memset(bfHeader.fileInfo.indexFileName, 0, 100);
    indexFileName.copy(bfHeader.fileInfo.indexFileName, 100);

    bfHeader.fileInfo.blockSize = 512;
    bfHeader.fileInfo.availBlock = 1;
    bfHeader.fileInfo.activeBlock = 0;
    bfHeader.fileInfo.blockCount = 1;

    auto headerSize = sizeof(bfHeader.headerInfo) + sizeof(bfHeader.fileInfo) + sizeof(FieldInfo) * bfHeader.fileInfo.fieldsPerRecord;
    bfHeader.headerInfo.headerSize = headerSize;

    file << bfHeader;
    file.close();
    BlockedFile bf(fileName);
    bf.addEmptyBlock(bfHeader.fileInfo.activeBlock, {0, -1, -1});
    bf.addEmptyBlock(bfHeader.fileInfo.availBlock, {0, -1, -1});
}

void BlockedFile::logicalDump() {
    printBlock(header.fileInfo.activeBlock);
    while (readBlockBuf.blockHeader.succeedingBlock != BlockBuffer::noSuccessor) {
        printBlock(readBlockBuf.blockHeader.succeedingBlock);
    }
}