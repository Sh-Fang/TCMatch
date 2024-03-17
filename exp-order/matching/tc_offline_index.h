#ifndef TCM_TC_OFFLINE_INDEX_H
#define TCM_TC_OFFLINE_INDEX_H

#include <iostream>
#include <unordered_map>
#include <map>
#include <vector>


#include "tc_io.h"
#include "tc_basic_type.h"



//===============================

class OfflineIndex{
public:
    std::unordered_map<IndexKeyItem, std::vector<IndexKeyItem>, IndexKeyItemHash> q_edge_to_all_neighbor;
    std::unordered_map<Label, std::vector<IndexKeyItem>,LabelHash> label_to_all_q_edge;
    std::vector<IndexKeyItem> all_tc_q_edge;
    std::unordered_map<IndexKeyItem, std::vector<IndexKeyItem>, IndexKeyItemHash> q_edge_to_all_prec_time_order;
    std::unordered_map<IndexKeyItem, std::vector<IndexKeyItem>,IndexKeyItemHash> nc_edge_to_pre;
    std::vector<std::pair<IndexKeyItem,IndexKeyItem>> nc_pair_vec;
    std::unordered_map<IndexKeyItem, std::vector<IndexKeyItem>, IndexKeyItemHash> q_edge_to_all_succ;
    std::unordered_map<IndexKeyItem,uit,IndexKeyItemHash> q_edge_to_q_id;

public:
    void build_offline_index(IO *tc_io);
    void init_2_q_edge_to_all_neighbor(std::vector<QEdge> &Q_);
    void init_3_label_to_all_q_edge(std::vector<QEdge> &Q_);
    void init_4_all_tc_q_edge(std::vector<std::vector<QEdge>> &T_);
    void init_5_q_edge_to_all_prec_time_order(std::vector<QEdge> &Q_, std::vector<std::vector<QEdge>> &T_);
    void init_6_nc_edge_to_prec();
    void init_7_q_edge_to_all_succ(std::vector<QEdge> &Q_, std::vector<std::vector<QEdge>> &T_);
    void init_10_q_edge_to_q_id(std::map<uit, QEdge> &q_id_2_q_edge);

};



#endif //TCM_TC_OFFLINE_INDEX_H
