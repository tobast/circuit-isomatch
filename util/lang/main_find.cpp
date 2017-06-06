#include <cstdio>
#include <iostream>
#include "aux.h"
using namespace std;

int main(int argc, char** argv) {
    if(argc != 3) {
        cerr << "Bad arguments. Usage:\n" << argv[0]
             << " [haystack.circ] [needle.circ]" << endl;
        return 1;
    }

    CircuitGroup* haystack = parse(argv[1]);
    CircuitGroup* needle = parse(argv[2]);
    haystack->freeze();
    needle->freeze();

    vector<MatchResult> matches = haystack->find(needle);

    cout << matches.size() << " matches" << endl;

    delete haystack;
    delete needle;
    return 0;
}

