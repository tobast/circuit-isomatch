#include "dyn_bitset.h"

#include <cstring>

DynBitset::DynBitset(size_t size)
    : size_(size)
{
    data = new Word[nbWords()];
}

DynBitset::DynBitset(const DynBitset& oth)
    : size_(oth.size_)
{
    data = new Word[nbWords()];
    memcpy(data, oth.data, nbWords());
}

DynBitset::DynBitset(size_t size, Word* words)
    : size_(size), data(words)
{}

DynBitset::DynBitset(DynBitset&& oth)
    : size_(oth.size_), data(oth.data)
{
    oth.data = nullptr;
}

DynBitset::~DynBitset() {
    delete[] data;
}

void DynBitset::operator=(const DynBitset& oth) {
    checkSize(oth);
    memcpy(data, oth.data, nbWords());
}

DynBitset::Reference DynBitset::operator[](size_t pos) {
    return Reference(&data[pos/word_size], pos % word_size);
}

DynBitset& DynBitset::operator&=(const DynBitset& oth) {
    checkSize(oth);
    for(size_t word = 0; word < nbWords(); ++word)
        data[word] &= oth.data[word];
    return *this;
}

DynBitset& DynBitset::operator|=(const DynBitset& oth) {
    checkSize(oth);
    for(size_t word = 0; word < nbWords(); ++word)
        data[word] |= oth.data[word];
    return *this;
}

DynBitset& DynBitset::operator^=(const DynBitset& oth) {
    checkSize(oth);
    for(size_t word = 0; word < nbWords(); ++word)
        data[word] ^= oth.data[word];
    return *this;
}

DynBitset& DynBitset::flip() {
    for(size_t word = 0; word < nbWords(); ++word)
        data[word] = ~data[word];
    return *this;
}

DynBitset DynBitset::operator&(const DynBitset& oth) const {
    checkSize(oth);
    DynBitset out(*this);
    out &= oth;
    return out;
}

DynBitset DynBitset::operator|(const DynBitset& oth) const {
    checkSize(oth);
    DynBitset out(*this);
    out |= oth;
    return out;
}

DynBitset DynBitset::operator^(const DynBitset& oth) const {
    checkSize(oth);
    DynBitset out(*this);
    out ^= oth;
    return out;
}

DynBitset DynBitset::operator~() const {
    DynBitset out(*this);
    out.flip();
    return out;
}

bool DynBitset::any() const {
    for(size_t word = 0; word < nbWords() - 1; ++word)
        if(data[word] != 0)
            return true;

    Word lastmask = ~0;
    size_t lastWordBits = size_ % word_size;
    if(lastWordBits > 0)
        lastmask >>= word_size - lastWordBits;
    return (data[nbWords() - 1] & lastmask) != 0;
}

int DynBitset::whichBit(DynBitset::Word word, size_t upTo) const {
    if(upTo == 1)
        return (word & 0x1);
    else if(word == 0)
        return -1;

    size_t nextSliceSize = upTo / 2;
    Word nextmask = (~0) >> (word_size - nextSliceSize);
    int topResult = whichBit(word >> nextSliceSize, nextSliceSize);
    int bottomResult = whichBit(word & (nextmask << nextSliceSize),
            nextSliceSize);
    if(topResult < 0 && bottomResult < 0)
        return -1;
    else if(topResult < 0)
        return bottomResult;
    // topResult >= 0
    else if(bottomResult >= 0)
        return -1;
    return topResult + nextSliceSize;
}

int DynBitset::singleBit() const {
    int singlePos = -1;
    for(size_t word = 0; word < nbWords() - 1; ++word) {
        if(data[word] != 0) {
            if(singlePos >= 0)
                return -1;
            singlePos = whichBit(data[word], word_size);
        }
    }
    return singlePos;
}
