#include "PrimaryKey.h"

#include <algorithm>
#include <sstream>

bool CompareKeys(PrimaryKey::KeyStruct& s1, PrimaryKey::KeyStruct& s2) {
    return CompareStr(s1.key, s2.key);
}

bool CompareStr(const std::string s1, const std::string& s2) {
    return s1.length() < s2.length() || (s1.length() == s2.length() && s1 < s2);
}

void PrimaryKey::GenerateIndexFile(std::string fileName) {
    std::ofstream ofile(fileName, std::ios::binary);

    header.version = 2;
    header.keyCount = vKey.size();
    header.format = IndexFileFormat::ASCII;
    ofile << header.version << " " << header.keyCount << " " << header.format << std::endl;

    for (size_t i = 0; i < vKey.size(); i++) {
        ofile << vKey[i].key << " " << vKey[i].RBN << '\n';
    }

    ofile.close();
}

bool PrimaryKey::ReadIndexFile(std::string fileName) {
    std::ifstream iFile(fileName, std::ios::binary);

    if (!iFile.is_open())
        return false;

    vKey.clear();

    std::string str;
    unsigned int offset;

    std::string line;
    PrimaryKey::KeyStruct prevZip = {"0", 0};  // dummy value for first comparison

    isSorted = true;
    bool initHeader = false;
    while (std::getline(iFile, line)) {
        std::stringstream ss(line);
        if (!initHeader) {
            ss >> header.version;
            ss >> header.keyCount;
            ss >> header.format;
            initHeader = true;
            continue;
        }
        ss >> str;
        ss >> offset;
        PrimaryKey::KeyStruct currZip = {str, offset};
        vKey.push_back(currZip);

        // here we check each record to see if the zip codes are in ascending order in the file
        // to determine if we need to sort before binary searching
        if (isSorted && !CompareKeys(prevZip, currZip)) {
            isSorted = false;
            // if the index is not sorted, the if statement will short circut upon checking isSorted
            // and thus we don't need to keep reassigning prevZip, hence the continue
            continue;
        } else {
            prevZip = currZip;
        }
    }

    iFile.close();
    return true;
}

void PrimaryKey::Add(KeyStruct keyStruct) {
    vKey.push_back(keyStruct);
}

int PrimaryKey::Find(std::string key) {
    for (int i = 0; i < vKey.size(); i++) {
        if (key == vKey[i].key) {
            return vKey[i].RBN;
        }
        if (!CompareStr(key, vKey[i].key)) {
            if (i == 0) {
                return vKey[0].RBN;
            } else {
                return vKey[i - 1].RBN;
            }
        }
    }
    if (!vKey.size()) {
        return notFound; 
    }
    return vKey[vKey.size() - 1].RBN;
}

int PrimaryKey::BinarySearch(std::string key) {  // Binary search for string type
    if (!isSorted) {
        std::sort(vKey.begin(), vKey.end(), CompareKeys);
        isSorted = true;
    }

    int left = 0;
    int right = vKey.size() - 1;
    int middle;

    while (left <= right) {
        middle = left + ((right - left) / 2);

        std::string& k = vKey[middle].key;
        if (key == k) {
            return vKey[middle].RBN;
        }

        if (key.length() > k.length() || (key > k && key.length() == k.length())) {
            left = middle + 1;
        } else {
            right = middle - 1;
        }
    }

    return notFound;
}