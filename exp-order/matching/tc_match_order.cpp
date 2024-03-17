#include "tc_match_order.h"

void MatchOrder::build_search_order(IO *tc_io, OfflineIndex *tc_offline_index) {

    std::unordered_map<IndexKeyItem, bool, IndexKeyItemHash> visited_q;

    // only consider those edges whose succ-edge vector not empty
    for(auto &it:tc_offline_index->q_edge_to_all_succ){

        // init visited_q
        for(auto &_it:tc_io->Q){
            IndexKeyItem key;
            key.qe = _it;
            visited_q[key] = false;
        }


        if(it.second.empty()){
            std::stack<IndexKeyItem> stk;
            auto &start_key = it.first;

            // the current_search_order now is empty
            auto &current_search_order = this->search_order[start_key];

            stk.push(start_key);

            while(!stk.empty()){
                auto current_key = stk.top();
                stk.pop();
                if(!visited_q.at(current_key)){  // if not be visited

                    visited_q.at(current_key) = true;

                    current_search_order.emplace_back(current_key);  // push into stack

                    // to check out 'current_key' whether exists a neighbor who is the nc-prec key of some key who has already in current_search_order.
                    std::vector<IndexKeyItem> nc_prec;
                    for(auto &nei:tc_offline_index->q_edge_to_all_neighbor.at(current_key)){

                        if(!visited_q.at(nei)){

                            for(auto &existed_nei:current_search_order){
                                if(existed_nei == current_key) continue;
                                if(std::find(tc_offline_index->q_edge_to_all_prec_time_order.at(existed_nei).begin(),
                                             tc_offline_index->q_edge_to_all_prec_time_order.at(existed_nei).end(),nei) != tc_offline_index->q_edge_to_all_prec_time_order.at(existed_nei).end()){
                                    if(existed_nei.qe.u_source_id != nei.qe.u_source_id && existed_nei.qe.u_source_id != nei.qe.u_target_id && existed_nei.qe.u_target_id != nei.qe.u_source_id && existed_nei.qe.u_target_id != nei.qe.u_target_id){
                                        nc_prec.emplace_back(nei);
                                    }
                                }
                            }

                        }

                    }

                    // to figure out 'current_key' all neighbor's degree
                    std::map<uit,std::vector<IndexKeyItem>> degree_to_keys;
                    for(auto &nei:tc_offline_index->q_edge_to_all_neighbor.at(current_key)){
                        if(!visited_q.at(nei)){

                            if(std::find(nc_prec.begin(), nc_prec.end(),nei) != nc_prec.end()){
                                continue;
                            }
                            uit nei_degree = tc_offline_index->q_edge_to_all_neighbor.at(nei).size();
                            degree_to_keys[nei_degree].emplace_back(nei);
                        }
                    }

                    // firstly, push keys by order degree from small to large
                    for(auto &map_it:degree_to_keys){
                        for(auto &key_it:map_it.second){
                            stk.push(key_it);
                        }
                    }

                    // finally, push all nc-prec keys
                    // (since this is a stack, all nc-prec keys will present at stack top,
                    // which means in the next loop, those nc-prec key will be processed first)
                    for(auto &prec_it:nc_prec){
                        stk.push(prec_it);
                    }
                }
            }
        }

    }

}

void MatchOrder::format_read_Q(const std::string& filePath) {
    const std::string& file_path = filePath;

    std::ifstream infile;

    infile.open(file_path);

    if(!infile){
        std::cerr << "Failed To Load Query" << std::endl;
        exit(-1);
    }


    std::unordered_map<int,std::unordered_map<int,bool>> Q_graph;

    //=============read-Q=========================

    std::string	line;

    while (getline(infile, line)) {
        if(line == "\r" || line.empty()) continue;

        char type = line[0];
        line = line.substr(2);

        if (type == 'e') {
            int id{},source_id{}, target_id{},source_label{}, target_label{},edge_label{};

            std::stringstream(line) >> id >> source_id >> target_id >> edge_label >> source_label >> target_label;

            Q_graph[source_id][target_id];
            Q_graph[target_id];

        }
    }

    infile.close();

}

void MatchOrder::build_search_order_with_RI(IO *tc_io, OfflineIndex *tc_offline_index) {

    std::unordered_map<IndexKeyItem, bool, IndexKeyItemHash> visited_q;

    // only consider those edges whose succ-edge vector not empty
    for(auto &it:tc_offline_index->q_edge_to_all_succ){

        // init visited_q
        for(auto &_it:tc_io->Q){
            IndexKeyItem key;
            key.qe = _it;
            visited_q[key] = false;
        }


        if(it.second.empty()){
            std::stack<IndexKeyItem> stk;
            auto &start_key = it.first;

            // the current_search_order now is empty
            auto &current_search_order = this->search_order[start_key];

            current_search_order.emplace_back(start_key); // save first-order into order

            // select start_key's max degree neighbor as second-order
            {
                IndexKeyItem max_degree_key = tc_offline_index->q_edge_to_all_neighbor.at(start_key).front();
                int max_degree=0;

                for(auto &nei_key:tc_offline_index->q_edge_to_all_neighbor.at(start_key)){
                    auto nei_degree = tc_offline_index->q_edge_to_all_neighbor.at(nei_key).size();
                    if(nei_degree > max_degree){
                        max_degree = (int)nei_degree;
                        max_degree_key = nei_key;
                    }
                }

                current_search_order.emplace_back(max_degree_key);  // save second-order into order
            }


            // for each existed order, calculate neighbor-connection-number for all existed order, select max neighbor-connection-number as next order
            while(current_search_order.size() < tc_offline_index->q_edge_to_all_neighbor.size()){
                // for each existed order, calculate neighbor-connection-number
                int max_neighbor_connection_count = 0;
                IndexKeyItem max_neighbor_connection_key;

                for(auto &existed_key:current_search_order){
                    for(auto &nei_key:tc_offline_index->q_edge_to_all_neighbor.at(existed_key)){
                        auto finder = std::find(current_search_order.begin(), current_search_order.end(),nei_key);
                        // if this nei_key not in current_search_order
                        if(finder == current_search_order.end()){
                            int nei_key_connection_count = 0;
                            for(auto &nei_nei_key:tc_offline_index->q_edge_to_all_neighbor.at(nei_key)){
                                auto nei_nei_finder = std::find(current_search_order.begin(), current_search_order.end(),nei_nei_key);
                                // if nei_nei_key can be found in order, connection count + 1
                                if(nei_nei_finder != current_search_order.end()){
                                    nei_key_connection_count++;
                                }
                            }
                            // find max
                            if(nei_key_connection_count > max_neighbor_connection_count){
                                max_neighbor_connection_count = nei_key_connection_count;
                                max_neighbor_connection_key = nei_key;
                            }
                        }
                    }
                }

                current_search_order.emplace_back(max_neighbor_connection_key);  // save third~end into order

            }


            // if one key is nc-pair for another key, even if this key has a small neighbor-connection-number, select this key as next order
            
            // nc-pair > max-neighbor-connection


        }

    }
}
