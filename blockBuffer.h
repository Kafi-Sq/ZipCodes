#include <vector>
#include "LengthIndicatedBuffer.h"
#include "Place.h"
#include "PlaceBlock.h"
using namespace std;

class BlockBuffer {
    private:
        ///reads the data in the file
        ///@param file file to be read
        ///@pre none
        ///@post reads data into buffer. returns true or false based on if file was opened and read or not
        bool read(istream &file);

    public:
        std::vector<char> buffer;/** buffer itself*/
        BlockFileHeader header;/** file header*/
        BlockHeader blockHeader;/** header for the block*/
        int unpackedRecords = 0;/** records in buffer that remain unpacked*/
        int curr = 0;/** current position in buffer*/
        int length = 0;/** length of content in buffer*/
        int RBN;/** keeps track of the record block number*/
        ///processes the block header
        ///@pre buffer contains block of records
        ///@post places block header at beginning of block
        void processBlockHeader();
        ///function used to determine contents of block header
        ///@param blockHeader block header to be used
        ///@pre none
        ///@post sets in-class block header to equal block header placced into function
        void setBlockHeader(BlockHeader blockHeader);

        ///default contructor
        ///@pre none
        ///@post class comes as defined
        BlockBuffer() = default;

        ///constructor that specifies a header for the file
        ///@param header the header to set in-class
        ///@pre none
        ///@post class has custom file header
        BlockBuffer(BlockFileHeader &header);
        ///function that reads a particular record block number
        ///@param file the file to be read
        ///@param RBN the record block number to get
        ///@pre none
        ///@post returns success or failure
        bool read(istream &file, int RBN);
        ///function that writes to a particular block
        ///@param file the file to write to
        ///@param RBN desired record block number to write to
        ///@pre none
        ///@post writes data in desired file and block
        void write(ostream &file, int RBN);
        ///unpacking buffer data into a length buffer
        ///@param lBuf the length buffer
        ///@pre data must be in the block buffer
        ///@post data is read into the length buffer and returns true or false based on success
        bool unpack(LengthIndicatedBuffer<BlockFileHeader> &lBuf);
        ///packs data into block buffer for entire block
        /// @param pb place block class used to find all lines of block
        /// @param lBuf length buffer that gets data taken from it
        ///@pre none
        ///@post data is packed into block buffer
        void pack(PlaceBlock pb, LengthIndicatedBuffer<BlockFileHeader> &lBuf);
        ///packs data into the block buffer
        /// @param lBuf length buffer where data is retrieved
        ///@pre none
        ///@post data is packed into block buffer
        void pack(LengthIndicatedBuffer<BlockFileHeader> &lBuf);
        ///sets previous block number to the argument RBN
        ///@param RBN block number of previous block to be set
        ///@pre none
        ///@post sets preceding block to the value of RBN
        void setPrecedingBlock(int RBN);
        ///sets next block number to the argument RBN
        ///@param RBN block number of next block to be set
        ///@pre none
        ///@post sets succeeding block to the value of RBN
        void setSucceedingBlock(int RBN);
        ///resets the buffer
        ///@pre none
        ///@post empties the buffer and sets variables accordingly
        void clear();
        
        void init(BlockFileHeader &header);
};