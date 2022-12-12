#include <optional>
#include "BlockBuffer.h"
#include "Place.h"
using namespace std;

class PlaceBlock {
   private:
    int size = -1;/**size of place block*/
    string highestKey = "";/**highest key*/

   public:
    BlockHeader blockHeader;/** header for blocks*/
    vector<Place> placeBlock;/**content of place block class*/
    ///constructor that contructs class with default values
    ///@pre none
    ///@post creates object with default values
    PlaceBlock() = default;

    bool unpack(BlockBuffer& bBuf, LengthIndicatedBuffer<BlockFileHeader>& lBuf);
    string getHighestKey();
    ///retrieves records with matching key
    ///@param key key to be searched
    ///@pre none
    ///@post returns object if one matches the key, returns empty object if no matching key
    optional<Place> getRecord(const string& key);
    ///prints the records contained within
    ///@pre none
    ///@post sends contents held within to cout
    void print();
    ///sorts place block content
    ///@pre must have content within
    ///@post place block is now sorted
    void sort();
    ///returns the size of place block object
    ///@pre none
    ///@post returns int variable 
    int getSize();
};