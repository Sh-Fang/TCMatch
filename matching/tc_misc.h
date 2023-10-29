#ifndef TCM_TC_MISC_H
#define TCM_TC_MISC_H

#include <iostream>

#include "tc_global_index.h"

class Misc{
public:

    std::stringstream statistical_info;

    long getMemoryUse();

    size_t get_current_RSS();

    void write_result(IO *tc_io, Arg *tc_arg, GlobalIndex *tc_global_index, Search *tc_search, MatchOrder *tc_order,
                      Misc *tc_misc);
};


#endif //TCM_TC_MISC_H
