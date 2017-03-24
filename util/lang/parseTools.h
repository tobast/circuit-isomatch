/** Auxiliary tools for parsing */

#pragma once

#include <vector>
#include <string>
#include <isomatch.h>  // build it against isomatch

template<typename T> struct ListElem {
    ListElem* next;
    T val;
    std::vector<T> toVect() const;
};

/** Builds a CircuitGroup from its description as parsed.
 * Warning! Allocates the group on the heap, you'll have to `delete` it. */
CircuitGroup* makeGroup(const char* name,
        const std::vector<std::string>& inputs,
        const std::vector<std::string>& outputs,
        const std::vector<CircuitTree*>& parts);

