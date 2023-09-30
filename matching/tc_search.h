#ifndef TCM_TC_SEARCH_H
#define TCM_TC_SEARCH_H

#include <iostream>
#include <vector>
#include <unordered_map>

//#include "config.h"
#include "tc_match_order.h"
#include "tc_offline_index.h"
#include "tc_arg.h"
#include "tc_basic_type.h"
#include "tc_global_index.h"

class GlobalIndex;  //Forward declaration.

//===================================

class Search{
public:

    uit match_count = 0;
    uit union_match_count = 0;

    H_TIME check_time_start, check_time_end;
    T_SPAN check_time_span;

    H_TIME enum_time_start, enum_time_end;
    T_SPAN enum_time_span;


    std::vector<std::vector<ResultEdge>> match_result;


    void direct_search_matching(IndexValueItem &value, MatchOrder *tc_order, OfflineIndex *tc_offline_index, Arg *tc_arg,
                           GlobalIndex *tc_global_index);

    //get the results based on a specified timestamp.
    void direct_search_matching_with_specified_timestamp(IndexValueItem &value, uit duplicated_timestamp,
                                                         MatchOrder *tc_order, OfflineIndex *tc_offline_index,
                                                         Arg *tc_arg, GlobalIndex *tc_global_index);

    void recursive_search(IndexValueItem &value,
                          std::unordered_map<IndexKeyItem, IndexValueItem, IndexKeyItemHash> &one_result,
                          std::vector<IndexKeyItem> &_search_order,
                          std::unordered_map<IndexKeyItem, std::vector<IndexValueItem>, IndexKeyItemHash> &global_index,
                          OfflineIndex *tc_offline_index, Arg *tc_arg);

    void recursive_search_with_dynamic_merge(IndexValueItem &value,
                                             std::unordered_map<IndexKeyItem, IndexValueItem, IndexKeyItemHash> &one_result,
                                             std::vector<IndexKeyItem> &_search_order,
                                             std::unordered_map<IndexKeyItem, std::vector<IndexValueItem>, IndexKeyItemHash> &global_index,
                                             OfflineIndex *tc_offline_index, Arg *tc_arg);


    IndexKeyItem* get_pre_order(const IndexKeyItem &key, std::vector<IndexKeyItem> &_search_order);
    IndexKeyItem* get_next_order(const IndexKeyItem &key, std::vector<IndexKeyItem> &_search_order);


    bool is_existed_connection(const IndexValueItem &value1, const IndexValueItem &value2);

};




#endif //TCM_TC_SEARCH_H
