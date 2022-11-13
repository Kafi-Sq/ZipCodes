#ifndef BLOCKEDFILE_H
#define BLOCKEDFILE_H

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>

#include "BlockBuffer.h"
#include "Header.h"
#include "LengthIndicatedFile.h"
#include "Place.h"
#include "PrimaryKey.h"
#include "enums.h"

class BlockedFile {
   private:
    static constexpr char MAGIC_HEADER_NUMBER[4] = {'Z', 'C', '0', '3'};

    BlockFileHeader header;
    PrimaryKey index;

    BlockBuffer readBlockBuf;
    BlockBuffer writeBlockBuf;

    LengthIndicatedBuffer<BlockFileHeader> readRecBuf;
    LengthIndicatedBuffer<BlockFileHeader> writeRecBuf;

    std::string fileName;
    std::fstream file;

    int dataStartRBN;
    int availBlockRBN;

    void initializeBuffers();
    void initializeIndex();
    bool openDataFile();

    void mergeBlocks(int fromRBN, int toRBN);
    void redistribute(int RBN);
    void splitBlock(int RBN, Place newRecord);

    void changeHeaderRecordCount(int newCount);
    void changeHeaderBlockCount(int newCount);

    void modifyBlockHeader(BlockHeader newBlockHeader, int RBN);

   public:
    void addEmptyBlock(int RBN, BlockHeader blockHeader);
    BlockedFile() = default;
    BlockedFile(std::string fileName);
    // ~BlockedFile();

    std::optional<Place> findRecord(std::string recordKey);
    void generateIndex();
    bool indexFileExists();
    bool deleteRecord(const std::string& recordKey);
    void addRecord(Place place);
    void printBlock(int RBN);
    void makeTestFile();

    void logicalDump();
    void physicalDump();

    BlockFileHeader writeHeader();
    static void createFromLIFile(std::string fileName, LengthIndicatedFile& liFile);
};

#endif