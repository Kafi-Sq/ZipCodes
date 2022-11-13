#include "LengthIndicatedFile.h"
LengthIndicatedFile::LengthIndicatedFile(std::string fileName) : fileName(fileName), readBuf(MAGIC_HEADER_NUMBER), writeBuf(MAGIC_HEADER_NUMBER) {
    openDataFile();
    initializeBuffers();
    dataStart = header.headerInfo.headerSize;
    initializeIndex();
}

LengthIndicatedFile::~LengthIndicatedFile() {
    file.close();
}

void LengthIndicatedFile::initializeBuffers() {
    readBuf.init(file);
    writeBuf.init(file);
    this->header = readBuf.header;
}

void LengthIndicatedFile::initializeIndex() {
    if (indexFileExists()) {
        index.ReadIndexFile(header.fileInfo.indexFileName);
    } else {
        generateIndex();
    }
}

bool LengthIndicatedFile::openDataFile() {
    file.open(fileName, std::ios::binary | std::ios::in | std::ios::out);
    return file.good();
}

std::optional<Place> LengthIndicatedFile::findRecord(std::string recordKey) {
    file.clear();
    auto recordFound = index.BinarySearch(recordKey);

    if (recordFound == index.notFound) {
        return {};
    }
    Place p;
    readBuf.read(file, recordFound);
    p.unpack(readBuf);
    return p;
}

void LengthIndicatedFile::generateIndex() {
    file.clear();
    file.seekg(dataStart);

    auto pos = (unsigned int)file.tellg();
    while (readBuf.read(file)) {
        Place place;
        place.unpack(readBuf);
        index.Add({place.getZipCode(), pos});
        pos = (unsigned int)file.tellg();
    }
    index.GenerateIndexFile(header.fileInfo.indexFileName);
}

bool LengthIndicatedFile::indexFileExists() {
    return std::filesystem::exists(header.fileInfo.indexFileName);
}

std::optional<Place> LengthIndicatedFile::getNextRecord() {
    if (readBuf.read(file)) {
        Place p;
        p.unpack(readBuf);
        return p;
    }
    return {};
}

PrimaryKey LengthIndicatedFile::getIndex() {
    return index;
}