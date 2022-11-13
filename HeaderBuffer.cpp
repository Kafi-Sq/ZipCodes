#include "HeaderBuffer.h"

#include <iostream>
// #include <string>

template <typename HeaderType>
void HeaderBuffer<HeaderType>::read(std::istream& ins) {
    ins.clear();
    ins.seekg(0);

    HeaderInfo hInfo;
    ins >> hInfo;

    buffer.resize(hInfo.headerSize);
    ins.clear();
    ins.seekg(0);

    char c;
    for (int i = 0; i < hInfo.headerSize; i++) {
        ins.read(&c, 1);
        buffer[i] = c;
    }
}

template <typename HeaderType>
HeaderType HeaderBuffer<HeaderType>::unpack() {
    HeaderType header;
    std::vector<FieldInfo> fields;

    auto fileInfoOffset = sizeof(HeaderType::headerInfo);

    memcpy(&header.headerInfo, &buffer[0], sizeof(HeaderInfo));
    memcpy(&header.fileInfo, &buffer[fileInfoOffset], sizeof(HeaderType::fileInfo));

    size_t fieldInfoOffset = fileInfoOffset + sizeof(HeaderType::fileInfo);
    for (int i = 0; i < header.fileInfo.fieldsPerRecord; i++) {
        FieldInfo fieldInfo;
        memcpy(&fieldInfo, &buffer[fieldInfoOffset], sizeof(FieldInfo));
        fields.push_back(fieldInfo);

        // set offset to beginning of next field info
        fieldInfoOffset += sizeof(FieldInfo);
    }

    header.fields = fields;

    return header;
}

template <typename HeaderType>
void HeaderBuffer<HeaderType>::unpack(HeaderType& header) {
    std::vector<FieldInfo> fields;

    auto fileInfoOffset = sizeof(HeaderType::headerInfo);

    memcpy(&header.headerInfo, &buffer[0], sizeof(HeaderInfo));
    memcpy(&header.fileInfo, &buffer[fileInfoOffset], sizeof(HeaderType::fileInfo));

    size_t fieldInfoOffset = fileInfoOffset + sizeof(HeaderType::fileInfo);
    for (int i = 0; i < header.fileInfo.fieldsPerRecord; i++) {
        FieldInfo fieldInfo;
        memcpy(&fieldInfo, &buffer[fieldInfoOffset], sizeof(FieldInfo));
        header.fields.push_back(fieldInfo);

        // set offset to beginning of next field info
        fieldInfoOffset += sizeof(FieldInfo);
    }

}


template class HeaderBuffer<LIHeader>;
template class HeaderBuffer<BlockFileHeader>;