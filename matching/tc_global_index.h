#ifndef TCM_TC_GLOBAL_INDEX_H
#define TCM_TC_GLOBAL_INDEX_H

#include <unordered_map>
#include <utility>

#include "tc_arg.h"
#include "tc_io.h"
#include "tc_search.h"
#include "tc_match_order.h"
#include "tc_g_index.h"

class Search;  //Forward declaration
//=========================================


class GlobalIndex{
public:

    //==misc===========
    uit duplicated_edge_count = 0;
    uit because_copy_link_count = 0;
    uit because_succ_edge_count = 0;

    //== time && space variable===============
    H_TIME index_time_start, index_time_end;
    T_SPAN index_time_span;

    H_TIME update_time_start, update_time_end;
    T_SPAN update_time_span;

    H_TIME search_time_start, search_time_end;
    T_SPAN search_time_span;

    H_TIME online_time_start, online_time_end;
    T_SPAN online_time_span;

    H_TIME connection_time_start, connection_time_end;
    T_SPAN connection_time_span;

    size_t index_RSS_size_start, index_RSS_size_end, index_RSS_size_span{};

    size_t online_RSS_size_start, online_RSS_size_end, online_RSS_size_span{};

    //===storage variable=====================
    std::unordered_map<IndexKeyItem, std::vector<IndexValueItem>, IndexKeyItemHash> global_index;
    std::unordered_map<IndexKeyItem,std::unordered_map<std::string,uit>,IndexKeyItemHash> fht;

    //===index function=====================
    //build index
    void index_process(IO *tc_io, OfflineIndex *tc_offline_index, Arg *tc_arg, GIndex *tc_g_index);

    bool index_incremental_update(SEdge &edge_, OfflineIndex *tc_offline_index, Arg *tc_arg, GIndex *tc_g_index);

    void index_update_global_index_consider_duplication(IndexKeyItem &key, SEdge &edge_,
                                                        OfflineIndex *tc_offline_index, GIndex *tc_g_index,
                                                        Arg *tc_arg);

    void index_update_global_index_without_consider_duplication(IndexKeyItem &key, SEdge &edge_,
                                                                OfflineIndex *tc_offline_index,
                                                                GIndex *tc_g_index, Arg *tc_arg);


    //===online function=====================
    //online process

    void save_fht(IndexValueItem &value);

    bool online_process(OfflineIndex *tc_offline_index, Search *tc_search, MatchOrder *tc_order, Arg *tc_arg,
                        GIndex *tc_g_index, IO *tc_io);

    bool is_satisfy_time_constrain(IndexKeyItem &key, SEdge &edge_, std::vector<IndexKeyItem> &all_pre_keys,
                                   OfflineIndex *tc_offline_index);

    void update_global_index_consider_duplication(IndexKeyItem &key, SEdge &edge_, OfflineIndex *tc_offline_index,
                                                  Search *tc_search, MatchOrder *tc_order, Arg *tc_arg,
                                                  GIndex *tc_g_index);

    void update_global_index_without_consider_duplication(IndexKeyItem &key, SEdge &edge_,
                                                          OfflineIndex *tc_offline_index, Search *tc_search,
                                                          MatchOrder *tc_order, Arg *tc_arg,
                                                          GIndex *tc_g_index);

    void create_double_link(IndexValueItem &value, OfflineIndex *tc_offline_index, GIndex *tc_g_index, Arg *tc_arg);

    void copy_double_link(IndexValueItem &existed_value, IndexValueItem &new_value, OfflineIndex *tc_offline_index);

};

#endif //TCM_TC_GLOBAL_INDEX_H
