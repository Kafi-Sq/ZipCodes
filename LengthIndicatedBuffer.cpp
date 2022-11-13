#include "LengthIndicatedBuffer.h"

#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <vector>

#include "HeaderBuffer.h"

struct LIHeader;
struct BlockFileHeader;

template <typename HeaderType>
LengthIndicatedBuffer<HeaderType>::LengthIndicatedBuffer(const char magicNumber[4], const char delim) : delim(delim) {
    clear();
    memcpy(this->magicNumber, magicNumber, sizeof(this->magicNumber));

    // shouldn't be necessary since the program should not read any part of the buffer that has not been written to
    // but valgrind complained, so here we are
    memset(buffer, 0, sizeof(buffer));
}

template <typename HeaderType>
std::string LengthIndicatedBuffer<HeaderType>::getIndexFileName() {
    return header.fileInfo.indexFileName;
}

template <typename HeaderType>
bool LengthIndicatedBuffer<HeaderType>::checkFileType(std::istream& instream) {
    instream.clear();
    instream.seekg(0);
    char first4[4];

    instream.read(first4, sizeof(first4));

    bool good = true;
    for (int i = 0; i < 4; i++) {
        good = (first4[i] == magicNumber[i]);
    }
    return good;
}

template <typename HeaderType>
bool LengthIndicatedBuffer<HeaderType>::init(std::istream& instream) {
    if (checkFileType(instream)) {
        // if file has magic number
        instream.seekg(0);
        HeaderBuffer<HeaderType> hBuf;
        hBuf.read(instream);
        header = hBuf.unpack();
        initialized = true;
        instream.clear();

    } else {
        // if file does not have magic number
        initialized = false;
    }

    return initialized;
}

template <typename HeaderType>
bool LengthIndicatedBuffer<HeaderType>::read(std::vector<char>& iBuffer, int offset) {
    clear();
    auto lengthIndicatorLength = header.fileInfo.lengthIndicatorSize;

    int recordLength = 0;

    // get actual record length
    switch (LengthIndicatorType(header.fileInfo.lengthIndicatorFormat)) {
        case LengthIndicatorType::BINARY:
            memcpy(&lengthIndicatorLength, iBuffer.data() + offset, lengthIndicatorLength);
            break;
        case LengthIndicatorType::ASCII: {
            std::string recordLengthStr;
            char c;
            for (int i = 0; i < lengthIndicatorLength; i++) {
                c = iBuffer[offset + i];
                recordLengthStr.push_back(c);
            }
            recordLength = std::stoi(recordLengthStr);

            break;
        }
        case LengthIndicatorType::BCD:
            break;
    }

    // read record
    memcpy(buffer, iBuffer.data() + offset + lengthIndicatorLength, recordLength);
    this->recordLength = recordLength;
    return true;
}

template <typename HeaderType>
bool LengthIndicatedBuffer<HeaderType>::read(std::istream& instream, int indexOffset) {
    instream.clear();
    instream.seekg(indexOffset);
    return read(instream);
}

template <typename HeaderType>
bool LengthIndicatedBuffer<HeaderType>::read(std::istream& instream) {
    clear();

    // get length of record length indicator
    auto lengthIndicatorLength = header.fileInfo.lengthIndicatorSize;

    int recordLength = 0;

    // get actual record length
    switch (LengthIndicatorType(header.fileInfo.lengthIndicatorFormat)) {
        case LengthIndicatorType::BINARY:
            instream.read((char*)&recordLength, lengthIndicatorLength);
            break;
        case LengthIndicatorType::ASCII: {
            std::string recordLengthStr;
            char c;
            for (int i = 0; i < lengthIndicatorLength; i++) {
                instream.get(c);
                if (c == EOF) {
                    return false;
                }
                recordLengthStr.push_back(c);
            }

            try {
                recordLength = std::stoi(recordLengthStr);
            } catch (std::invalid_argument& err) {
                if (instream.eof()) {
                    return false;
                }
                throw err;
            }

            break;
        }
        case LengthIndicatorType::BCD:
            break;
    }

    // read record
    instream.read(buffer, recordLength);
    this->recordLength = recordLength;
    return instream.good();
}

template <typename HeaderType>
void LengthIndicatedBuffer<HeaderType>::writeHeader(std::ostream& outstream) {
    outstream.seekp(0);
    outstream << header;
}

template <typename HeaderType>
bool LengthIndicatedBuffer<HeaderType>::unpack(std::string& str) {
    auto state = CSVState::UnquotedField;  // assume field is not quoted by default

    bool fieldHasMore = true;
    bool recordHasMore = true;
    while (fieldHasMore) {
        char c = buffer[curr];
        switch (state) {
            case CSVState::UnquotedField:
                if (c == delim) {
                    fieldHasMore = false;
                    fieldNum++;
                } else if (curr >= recordLength) {
                    fieldHasMore = false;
                    recordHasMore = false;
                    fieldNum = 0;
                } else if (c == '"') {
                    state = CSVState::QuotedField;
                } else {
                    str.push_back(c);
                }
                break;
            case CSVState::QuotedField:
                if (c == '"') {
                    state = CSVState::QuotedQuote;
                } else {
                    str.push_back(c);
                }
                break;
            case CSVState::QuotedQuote:
                if (c == delim) {
                    fieldHasMore = false;
                    fieldNum++;
                } else if (c == '"') {
                    str.push_back(c);
                    state = CSVState::QuotedField;
                } else {
                    state = CSVState::UnquotedField;
                }
                break;
        }
        curr++;
    }
    return recordHasMore;
}

template <typename HeaderType>
void LengthIndicatedBuffer<HeaderType>::clear() {
    recordLength = curr = 0;
}

template <typename HeaderType>
void LengthIndicatedBuffer<HeaderType>::pack(const std::string str) {
    // put delimiters in between each field skipping the first
    if (curr > 0) {
        buffer[curr++] = delim;
    }

    // note that the size method of std::string does not include
    // the null terminator in the length, thus we are just copying the
    // values in the string to the buffer, as desired
    memcpy(&buffer[curr], str.c_str(), str.size());

    // move curr pointer to the end of the field we just added
    curr += str.size();
    recordLength = curr;
}

template <typename HeaderType>
void LengthIndicatedBuffer<HeaderType>::write(std::vector<char>& iBuffer, int offset) {
    int lengthChars = 0;
    switch (LengthIndicatorType(header.fileInfo.lengthIndicatorFormat)) {
        case LengthIndicatorType::ASCII: {
            std::ostringstream lengthStream;
            lengthStream << std::setfill('0') << std::setw(header.fileInfo.lengthIndicatorSize) << (curr);
            auto lengthStr = lengthStream.str();
            memcpy(iBuffer.data() + offset, lengthStr.c_str(), lengthStream.str().size());

            lengthChars = lengthStream.str().size();
            break;
        }
        case LengthIndicatorType::BCD:
        case LengthIndicatorType::BINARY:
            break;
    }
    memcpy(iBuffer.data() + offset + lengthChars, buffer, curr);
}

template <typename HeaderType>
void LengthIndicatedBuffer<HeaderType>::write(std::ostream& outstream) {
    switch (LengthIndicatorType(header.fileInfo.lengthIndicatorFormat)) {
        case LengthIndicatorType::ASCII: {
            std::ostringstream lengthStream;
            lengthStream << std::setfill('0') << std::setw(header.fileInfo.lengthIndicatorSize) << (curr);
            auto lengthStr = lengthStream.str();
            outstream.write(lengthStr.c_str(), lengthStream.str().size());
            break;
        }
        case LengthIndicatorType::BCD:
        case LengthIndicatorType::BINARY:
            break;
    }

    outstream.write(buffer, curr);
}

template <typename HeaderType>
FieldInfo LengthIndicatedBuffer<HeaderType>::getCurFieldHeader() {
    return header.fields[fieldNum];
}