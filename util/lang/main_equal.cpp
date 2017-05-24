#include <cstdio>
#include <iostream>
#include "aux.h"
using namespace std;

int main(int, char** argv) {
    CircuitGroup* circuit1 = parse(argv[1]);
    CircuitGroup* circuit2 = parse(argv[2]);
    circuit1->freeze();
    circuit2->freeze();

    cout << circuit1->equals(circuit2) << endl;
    cout << " =========== " << endl;
    cout << circuit2->equals(circuit1) << endl;

    delete circuit1;
    delete circuit2;
    return 0;
}

