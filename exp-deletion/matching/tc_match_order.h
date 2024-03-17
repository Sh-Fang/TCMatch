#ifndef TCM_TC_MATCH_ORDER_H
#define TCM_TC_MATCH_ORDER_H

#include <iostream>
#include <unordered_map>
#include <vector>
#include <stack>


#include "tc_basic_type.h"
#include "tc_io.h"
#include "tc_offline_index.h"



class MatchOrder{
public:
    std::unordered_map<IndexKeyItem, std::vector<IndexKeyItem>, IndexKeyItemHash> search_order;
    void build_search_order(IO *tc_io, OfflineIndex *tc_offline_index);
};

#endif //TCM_TC_MATCH_ORDER_H
