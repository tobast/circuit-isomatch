#include <cstdio>
#include <iostream>
#include "parser.tab.hpp"
using namespace std;

int main(int, char** argv) {
    FILE* fPtr = fopen(argv[1], "r"); // Sanity checks? What sanity checks?
    CircuitGroup* circuit = doParse(fPtr);
    fclose(fPtr);

    if(circuit == NULL) {
        fprintf(stderr, "error: parsing failed.\n");
        return 1;
    }

    circuit->toDot(cout);

    delete circuit;
    return 0;
}

