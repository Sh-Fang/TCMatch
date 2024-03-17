#include "tc_match_order.h"

void MatchOrder::build_search_order(IO *tc_io, OfflineIndex *tc_offline_index) {

    std::unordered_map<IndexKeyItem, bool, IndexKeyItemHash> visited_q;

    for(auto &it:tc_offline_index->q_edge_to_all_succ){
        for(auto &_it:tc_io->Q){
            IndexKeyItem key;
            key.qe = _it;
            visited_q[key] = false;
        }
        if(it.second.empty()){
            std::stack<IndexKeyItem> stk;
            auto &start_key = it.first;
            auto &current_search_order = this->search_order[start_key];

            stk.push(start_key);

            while(!stk.empty()){
                auto current_key = stk.top();
                stk.pop();
                if(!visited_q.at(current_key)){

                    visited_q.at(current_key) = true;

                    current_search_order.emplace_back(current_key);

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


                    for(auto &map_it:degree_to_keys){
                        for(auto &key_it:map_it.second){
                            stk.push(key_it);
                        }
                    }

                    for(auto &prec_it:nc_prec){
                        stk.push(prec_it);
                    }
                }
            }
        }

    }

}
