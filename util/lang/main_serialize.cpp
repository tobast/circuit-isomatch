#include <cstdio>
#include <iostream>
#include "aux.h"
using namespace std;

int main(int, char** argv) {
    CircuitGroup* circuit = parse(argv[1]);
    circuit->freeze();
    circuit->serialize(cout);

    delete circuit;
    return 0;
}

