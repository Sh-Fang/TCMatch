#include "tc_offline_index.h"

void OfflineIndex::init_2_q_edge_to_all_neighbor(std::vector<QEdge> &Q_) {
    for(auto &q_it:Q_){
        IndexKeyItem key;
        key.qe = q_it;
        this->q_edge_to_all_neighbor[key];


        for(auto &q_it2:Q_){
            if(q_it2 == q_it){
                continue;
            }

            IndexKeyItem key2;
            key2.qe = q_it2;

            if(key.qe.u_source_id == key2.qe.u_source_id || key.qe.u_source_id == key2.qe.u_target_id || key.qe.u_target_id == key2.qe.u_source_id || key.qe.u_target_id == key2.qe.u_target_id){
                this->q_edge_to_all_neighbor[key].emplace_back(key2);
            }

        }
    }
}

void OfflineIndex::init_3_label_to_all_q_edge(std::vector<QEdge> &Q_) {
    for(auto &it:Q_){
        IndexKeyItem key;
        key.qe = it;

        Label label;
        label.source_label = it.u_source_label;
        label.target_label = it.u_target_label;
        label.edge_label = it.u_edge_label;

        this->label_to_all_q_edge[label].emplace_back(key);
    }
}

void OfflineIndex::init_4_all_tc_q_edge(std::vector<std::vector<QEdge>> &T_) {
    for(auto &row:T_){
        for(auto &it:row){
            IndexKeyItem key;
            key.qe = it;
            if(std::find(this->all_tc_q_edge.begin(), this->all_tc_q_edge.end(),key) == this->all_tc_q_edge.end()){
                this->all_tc_q_edge.emplace_back(key);
            }
        }
    }
}

void OfflineIndex::init_5_q_edge_to_all_prec_time_order(std::vector<QEdge> &Q_, std::vector<std::vector<QEdge>> &T_) {
    for(auto &it:Q_){
        IndexKeyItem key;
        key.qe = it;


        if(std::find(this->all_tc_q_edge.begin(), this->all_tc_q_edge.end(),key) != this->all_tc_q_edge.end()){

            bool is_break = false;

            for(auto &t_it:T_){
                for(uit i = 0 ; i < t_it.size() ; i++){
                    if(t_it[i] == it){
                        if(t_it[i] == *t_it.begin()){
                            this->q_edge_to_all_prec_time_order[key];
                            is_break = true;
                            break;
                        } else{
                            IndexKeyItem q_edge;
                            q_edge.qe = t_it[i - 1];
                            this->q_edge_to_all_prec_time_order[key].emplace_back(q_edge);
                        }
                    }
                }

                if(is_break){
                    break;
                }
            }
        } else{
            this->q_edge_to_all_prec_time_order[key];
        }
    }
}

void OfflineIndex::init_7_q_edge_to_all_succ(std::vector<QEdge> &Q_, std::vector<std::vector<QEdge>> &T_) {
    for(auto &tc_edge:this->all_tc_q_edge){
        for(auto &t_row:T_){
            auto finder = std::find(t_row.begin(), t_row.end(),tc_edge.qe);
            if(finder != t_row.end()){
                this->q_edge_to_all_succ[tc_edge];
                auto later_finder = finder;
                ++later_finder;
                while(later_finder != t_row.end()){
                    IndexKeyItem later_key;
                    later_key.qe = *(later_finder);

                    if(std::find(this->q_edge_to_all_succ[tc_edge].begin(), this->q_edge_to_all_succ[tc_edge].end(), later_key) == this->q_edge_to_all_succ[tc_edge].end()){
                        this->q_edge_to_all_succ[tc_edge].emplace_back(later_key);
                    }
                    ++later_finder;
                }
            }
        }
    }




    for(auto &q_it:Q_){
        IndexKeyItem key;
        key.qe = q_it;


        if(std::find(this->all_tc_q_edge.begin(), this->all_tc_q_edge.end(),key) == this->all_tc_q_edge.end()){
            this->q_edge_to_all_succ[key];
        }
    }
}

void OfflineIndex::init_6_nc_edge_to_prec() {
    for(auto &it:this->q_edge_to_all_prec_time_order){
        auto q_edge = it.first;
        auto all_pre = it.second;
        for(auto &pre_it:all_pre){

            if(q_edge.qe.u_source_id != pre_it.qe.u_source_id && q_edge.qe.u_source_id != pre_it.qe.u_target_id && q_edge.qe.u_target_id != pre_it.qe.u_source_id && q_edge.qe.u_target_id != pre_it.qe.u_target_id){
                this->nc_edge_to_pre[q_edge].emplace_back(pre_it);
                this->nc_pair_vec.emplace_back(q_edge,pre_it);
            }
        }
    }
}

void OfflineIndex::init_10_q_edge_to_q_id(std::map<uit, QEdge> &q_id_2_q_edge) {
    for(auto &it:q_id_2_q_edge){
        IndexKeyItem key;
        key.qe = it.second;
        this->q_edge_to_q_id[key] = it.first;
    }
}

void OfflineIndex::build_offline_index(IO *tc_io) {
    init_2_q_edge_to_all_neighbor(tc_io->Q);
    init_3_label_to_all_q_edge(tc_io->Q);
    init_4_all_tc_q_edge(tc_io->T);
    init_5_q_edge_to_all_prec_time_order(tc_io->Q, tc_io->T);
    init_6_nc_edge_to_prec();
    init_7_q_edge_to_all_succ(tc_io->Q, tc_io->T);
    init_10_q_edge_to_q_id(tc_io->q_id_2_q_edge);
}
