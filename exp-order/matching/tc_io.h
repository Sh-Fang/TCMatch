#ifndef TCM_TC_IO_H
#define TCM_TC_IO_H

#include <iostream>
#include <utility>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>

#include "tc_basic_type.h"
#include "tc_arg.h"


//================================================

class IO{
public:
    uit valid_update_edge_num = 0;
    uit valid_data_edge_num = 0;
    uit total_update_edge_num = 0;
    uit total_data_edge_num = 0;

    std::map<uit,QEdge> q_id_2_q_edge;

    std::vector<SEdge> S;
    std::vector<SEdge> S_data;
    std::vector<SEdge> S_update;
    void load_stream_2_data_graph_and_update_graph(Arg *tc_arg);

    std::vector<QEdge> Q;
    std::vector<std::vector<QEdge>> T;
    void load_timing_query(Arg *tc_arg);

};

#endif //TCM_TC_IO_H
