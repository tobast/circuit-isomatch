#include <cstdio>
#include <iostream>
#include "aux.h"
using namespace std;

int main(int, char** argv) {
    CircuitGroup* circuit = parse(argv[1]);
    circuit->freeze();

    cout << circuit->sign() << endl;

    delete circuit;
    return 0;
}

