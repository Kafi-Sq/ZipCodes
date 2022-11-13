#ifndef HEADER_H
#define HEADER_H

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "enums.h"

// needed to remove automatic alignment of struct members
#pragma pack(push, 1)
struct HeaderInfo {
    char magic[4];   /// 4 bytes at the start indicating that the file is of the correct type
    int version;     /// version number
    int headerSize;  /// size of header in bytes, including header info

    friend std::istream& operator>>(std::istream& ins, HeaderInfo& headerInfo) {
        ins.read((char*)(&headerInfo), sizeof(headerInfo));
        return ins;
    }

    friend std::ostream& operator<<(std::ostream& os, HeaderInfo& headerInfo) {
        os.write(reinterpret_cast<char*>(&headerInfo.magic), sizeof(headerInfo.magic));
        os.write(reinterpret_cast<char*>(&headerInfo.version), sizeof(headerInfo.version));
        os.write(reinterpret_cast<char*>(&headerInfo.headerSize), sizeof(headerInfo.headerSize));
        return os;
    }
};

struct BlockFileInfo {
    int lengthIndicatorSize;                    /// number of bytes in length indicator
    LengthIndicatorType lengthIndicatorFormat;  /// ASCII, BINARY, or BCD

    int fieldsPerRecord;     /// number of fields in each record
    int primaryKeyPosition;  /// the ordinal position of the primary key used to index the file

    int blockSize;
    int availBlock;
    int activeBlock;

    int blockCount;

    char indexFileName[100];  /// the name of the index file to be loaded at program start

    friend std::istream& operator>>(std::istream& ins, BlockFileInfo& fileInfo) {
        ins.read((char*)(&fileInfo), sizeof(fileInfo));
        return ins;
    }

    friend std::ostream& operator<<(std::ostream& os, BlockFileInfo& fileInfo) {
        os.write(reinterpret_cast<char*>(&fileInfo.lengthIndicatorSize), sizeof(fileInfo.lengthIndicatorSize));
        os.write(reinterpret_cast<char*>(&fileInfo.lengthIndicatorFormat), sizeof(fileInfo.lengthIndicatorFormat));

        os.write(reinterpret_cast<char*>(&fileInfo.fieldsPerRecord), sizeof(fileInfo.fieldsPerRecord));
        os.write(reinterpret_cast<char*>(&fileInfo.primaryKeyPosition), sizeof(fileInfo.primaryKeyPosition));

        os.write(reinterpret_cast<char*>(&fileInfo.blockSize), sizeof(fileInfo.blockSize));
        os.write(reinterpret_cast<char*>(&fileInfo.availBlock), sizeof(fileInfo.availBlock));
        os.write(reinterpret_cast<char*>(&fileInfo.activeBlock), sizeof(fileInfo.activeBlock));

        os.write(reinterpret_cast<char*>(&fileInfo.blockCount), sizeof(fileInfo.blockCount));

        os.write(reinterpret_cast<char*>(&fileInfo.indexFileName), sizeof(fileInfo.indexFileName));

        return os;
    }
};

struct LIFileInfo {
    int lengthIndicatorSize;                    /// number of bytes in length indicator
    LengthIndicatorType lengthIndicatorFormat;  /// ASCII, BINARY, or BCD

    int fieldsPerRecord;     /// number of fields in each record
    int primaryKeyPosition;  /// the ordinal position of the primary key used to index the file

    char indexFileName[100];  /// the name of the index file to be loaded at program start

    friend std::istream& operator>>(std::istream& ins, LIFileInfo& fileInfo) {
        ins.read((char*)(&fileInfo), sizeof(fileInfo));
        return ins;
    }

    friend std::ostream& operator<<(std::ostream& os, LIFileInfo& fileInfo) {
        os.write(reinterpret_cast<char*>(&fileInfo.lengthIndicatorSize), sizeof(fileInfo.lengthIndicatorSize));
        os.write(reinterpret_cast<char*>(&fileInfo.lengthIndicatorFormat), sizeof(fileInfo.lengthIndicatorFormat));

        os.write(reinterpret_cast<char*>(&fileInfo.fieldsPerRecord), sizeof(fileInfo.fieldsPerRecord));
        os.write(reinterpret_cast<char*>(&fileInfo.primaryKeyPosition), sizeof(fileInfo.primaryKeyPosition));

        os.write(reinterpret_cast<char*>(&fileInfo.indexFileName), sizeof(fileInfo.indexFileName));

        return os;
    }
};

struct FieldInfo {
    char fieldName[50];     /// the name of the field
    HeaderField fieldType;  /// the HeaderField type of the field

    friend std::istream& operator>>(std::istream& ins, FieldInfo& fieldInfo) {
        ins.read((char*)(&fieldInfo), sizeof(fieldInfo));
        return ins;
    }

    friend std::ostream& operator<<(std::ostream& os, FieldInfo& fieldInfo) {
        os.write(reinterpret_cast<char*>(&fieldInfo), sizeof(fieldInfo));
        return os;
    }
};

struct LIHeader {
    HeaderInfo headerInfo;
    LIFileInfo fileInfo;
    std::vector<FieldInfo> fields;

    friend std::ostream& operator<<(std::ostream& os, LIHeader& header) {
        os << header.headerInfo;
        os << header.fileInfo;

        for (auto f : header.fields) {
            os << f;
        }
        return os;
    }
};

struct BlockFileHeader {
    HeaderInfo headerInfo;
    BlockFileInfo fileInfo;
    std::vector<FieldInfo> fields;

    friend std::ostream& operator<<(std::ostream& os, BlockFileHeader& header) {
        os << header.headerInfo;
        os << header.fileInfo;

        for (auto f : header.fields) {
            os << f;
        }
        return os;
    }

    BlockFileHeader() = default;

    BlockFileHeader(LIHeader liHeader) {
        this->headerInfo.headerSize = liHeader.headerInfo.headerSize;
        this->headerInfo.version = liHeader.headerInfo.version;
        memcpy(this->headerInfo.magic, "ZC03", sizeof(this->headerInfo.magic));

        this->fields = liHeader.fields;

        this->fileInfo.lengthIndicatorSize = liHeader.fileInfo.lengthIndicatorSize;
        this->fileInfo.lengthIndicatorFormat = liHeader.fileInfo.lengthIndicatorFormat;
        this->fileInfo.fieldsPerRecord = liHeader.fileInfo.fieldsPerRecord;
        this->fileInfo.primaryKeyPosition = liHeader.fileInfo.primaryKeyPosition;

        memcpy(this->fileInfo.indexFileName, liHeader.fileInfo.indexFileName, sizeof(liHeader.fileInfo.indexFileName));
    }
};

struct BlockHeader {
    int recordCount;
    int precedingBlock;
    int succeedingBlock;
};

#pragma pack(pop)

#endif  // HEADER_H