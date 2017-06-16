#include "dyn_bitset.h"

#include <cstring>

DynBitset::Reference& DynBitset::Reference::operator=(bool e) {
    if(e)
        set();
    else
        reset();
    return *this;
}

DynBitset::Reference& DynBitset::Reference::operator=(
        const DynBitset::Reference& e)
{
    if(e)
        set();
    else
        reset();
    return *this;
}

DynBitset::DynBitset(size_t size)
    : size_(size)
{
    data = new Word[nbWords()];
    reset();
}

DynBitset::DynBitset(const DynBitset& oth)
    : size_(oth.size_)
{
    data = new Word[nbWords()];
    memcpy(data, oth.data, nbWords() * sizeof(Word));
    // We assume `oth`'s last bits are 0s, as it should be
}

DynBitset::DynBitset(size_t size, Word* words)
    : size_(size), data(words)
{}

DynBitset::DynBitset(DynBitset&& oth)
    : size_(oth.size_), data(oth.data)
{
    // This could seriously mess things up if called improperly
    oth.data = nullptr;
}

DynBitset::~DynBitset() {
    delete[] data;
}

void DynBitset::operator=(const DynBitset& oth) {
    checkSize(oth);
    memcpy(data, oth.data, nbWords() * sizeof(Word));
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

    // Keep 0s at the end
    Word lastMask = 0;
    lastMask = ~lastMask;
    size_t lastWordBits = size_ % word_size;
    if(lastWordBits > 0)
        lastMask >>= word_size - lastWordBits;
    data[nbWords() - 1] &= lastMask;

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

void DynBitset::reset() {
    for(size_t word = 0; word < nbWords(); ++word)
        data[word] = 0;
}

bool DynBitset::any() const {
    for(size_t word = 0; word < nbWords(); ++word)
        if(data[word] != 0)
            return true;
    return false;
}

bool DynBitset::anyOver(size_t pos) const {
    size_t firstWord = pos / word_size;

    Word firstMask = 0;
    firstMask = ~firstMask;
    firstMask <<= (pos % word_size);
    if(data[firstWord] & firstMask)
        return true;

    for(size_t word = firstWord + 1; word < nbWords(); ++word)
        if(data[word] != 0)
            return true;
    return false;
}

int DynBitset::whichBit(DynBitset::Word word, size_t, int offset) const {
    /*
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
    */
    for(int bit = 0; word != 0; ++bit) {
        if(word & 0x1lu) {
            if(word != 1)
                return -1;
            return offset + bit;
        }
        word >>= 1;
    }
    return -1;
}

int DynBitset::singleBit() const {
    int singlePos = -1;
    for(size_t word = 0; word < nbWords() - 1; ++word) {
        if(data[word] != 0) {
            if(singlePos >= 0)
                return -1;
            singlePos = whichBit(data[word], word_size, word * word_size);
        }
    }
    return singlePos;
}

std::string DynBitset::dump() const {
    std::string out;
    for(size_t word = 0; word < nbWords(); ++word) {
        Word cWord = data[word];
        for(size_t hexDigit = 0 ; hexDigit < word_size / 4; ++hexDigit) {
            char c = cWord % 16;
            out += (c >= 10) ? ('A' + c - 10) : ('0' + c);
            cWord >>= 4;
        }
    }
    return out;
}
