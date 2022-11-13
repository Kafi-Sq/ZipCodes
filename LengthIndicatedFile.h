#ifndef LENGTHINDICATEDFILE_H
#define LENGTHINDICATEDFILE_H

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>

#include "Header.h"
#include "LengthIndicatedBuffer.h"
#include "Place.h"
#include "PrimaryKey.h"
#include "enums.h"

class LengthIndicatedFile {
    static constexpr char MAGIC_HEADER_NUMBER[4] = {'Z', 'C', '0', '2'};

   private:
    PrimaryKey index;

    LengthIndicatedBuffer<LIHeader> readBuf;
    LengthIndicatedBuffer<LIHeader> writeBuf;

    std::string fileName;
    std::fstream file;

    int dataStart;

    bool openDataFile();
    bool indexFileExists();
    void initializeBuffers();
    void initializeIndex();
    void generateIndex();

   public:
    LIHeader header;
    LengthIndicatedFile(std::string fileName);
    ~LengthIndicatedFile();
    std::optional<Place> findRecord(std::string recordKey);
    std::optional<Place> getNextRecord();
    PrimaryKey getIndex();
};

#endif