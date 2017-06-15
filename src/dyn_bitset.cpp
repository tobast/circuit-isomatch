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
