#include "tc_global_index.h"
#include "tc_search.h"


void IndexValueItem::save_in_neighbor_link(IndexKeyItem &save_key, uit vec_position) {
    if(this->neighbor_link.find(save_key) != this->neighbor_link.end()){
        this->neighbor_link[save_key].emplace_back(vec_position);
    }
}



void IndexValueItem::save_in_neighbor_link(IndexKeyItem &save_key) {
    if(this->neighbor_link.find(save_key) == this->neighbor_link.end()){
        this->neighbor_link[save_key];
    }
}


void IndexValueItem::update_self_status() {
    bool is_all_neighbor_complete = true;

    for(const auto& nei_it:neighbor_link){
        if(nei_it.second.empty()){
            is_all_neighbor_complete = false;
            break;
        }
    }

    if(is_all_neighbor_complete){
        status = true;
    } else{
        status = false;
    }
}




void GlobalIndex::index_process(IO *tc_io, OfflineIndex *tc_offline_index, Arg *tc_arg, GIndex *tc_g_index) {
    //Occupy position
    for(auto &q_it:tc_io->Q){
        IndexKeyItem key{q_it.u_source_id, q_it.u_target_id, q_it.u_edge_label, q_it.u_source_label, q_it.u_target_label};
        this->global_index[key];
        this->fht[key];
    }

    //start to initialize
    for(auto &edge:tc_io->S_data){
        this->index_incremental_update(edge, tc_offline_index, tc_arg, tc_g_index);
    }
}




void GlobalIndex::save_fht(IndexValueItem &value) {
    std::string edge_to_str = std::to_string(value.v_source_id) + "_" + std::to_string(value.v_target_id) + "_" + std::to_string(value.edge_label) + "_" + std::to_string(value.v_source_label) + "_" + std::to_string(value.v_target_label);
    this->fht[value.self_key][edge_to_str] = value.self_pos;
}




bool GlobalIndex::is_satisfy_time_constrain(IndexKeyItem &key, SEdge &edge_, std::vector<IndexKeyItem> &all_pre_keys,
                                            OfflineIndex *tc_offline_index) {


    if(tc_offline_index->nc_edge_to_pre.find(key) != tc_offline_index->nc_edge_to_pre.end()){

        bool is_nc_satisfy = false;

        for(auto &pre_it:all_pre_keys){

            is_nc_satisfy = false;

            if(this->global_index[pre_it].empty()){
                return false;
            } else{
                for(auto &g_it:this->global_index[pre_it]){
                    if((g_it.v_source_id != edge_.v_source_id || g_it.v_target_id != edge_.v_target_id || g_it.edge_label != edge_.edge_label ) && (g_it.timestamp < edge_.timestamp)){
                        is_nc_satisfy = true;
                        break;
                    }
                }
            }
        }

        if(is_nc_satisfy){
            return true;
        } else{
            return false;
        }
    }


    else{

        bool is_satisfy_all_pre = false;

        for(auto &pre_it:all_pre_keys){

            is_satisfy_all_pre = false;

            if(this->global_index.at(pre_it).empty()){
                return false;
            } else{
                if(pre_it.qe.u_source_id == key.qe.u_source_id){
                    for(auto &g_it:this->global_index.at(pre_it)){
                        if(g_it.v_source_id == edge_.v_source_id && g_it.v_target_id == edge_.v_target_id && g_it.edge_label == edge_.edge_label && g_it.timestamp == edge_.timestamp){
                            continue;
                        }

                        if((g_it.v_source_id == edge_.v_source_id) && (g_it.timestamp < edge_.timestamp)){
                            is_satisfy_all_pre = true;
                            break;
                        }
                    }
                } else if(pre_it.qe.u_source_id == key.qe.u_target_id){
                    for(auto &g_it:this->global_index[pre_it]){
                        if(g_it.v_source_id == edge_.v_source_id && g_it.v_target_id == edge_.v_target_id && g_it.edge_label == edge_.edge_label && g_it.timestamp == edge_.timestamp){
                            continue;
                        }

                        if((g_it.v_source_id == edge_.v_target_id) && (g_it.timestamp < edge_.timestamp)){
                            is_satisfy_all_pre = true;
                            break;
                        }
                    }
                } else if(pre_it.qe.u_target_id == key.qe.u_target_id){
                    for(auto &g_it:this->global_index[pre_it]){
                        if(g_it.v_source_id == edge_.v_source_id && g_it.v_target_id == edge_.v_target_id && g_it.edge_label == edge_.edge_label && g_it.timestamp == edge_.timestamp){
                            continue;
                        }

                        if((g_it.v_target_id == edge_.v_target_id) && (g_it.timestamp < edge_.timestamp)){
                            is_satisfy_all_pre = true;
                            break;
                        }
                    }
                } else if(pre_it.qe.u_target_id == key.qe.u_source_id){
                    for(auto &g_it:this->global_index[pre_it]){
                        if(g_it.v_source_id == edge_.v_source_id && g_it.v_target_id == edge_.v_target_id && g_it.edge_label == edge_.edge_label && g_it.timestamp == edge_.timestamp){
                            continue;
                        }

                        if((g_it.v_target_id == edge_.v_source_id) && (g_it.timestamp < edge_.timestamp)){
                            is_satisfy_all_pre = true;
                            break;
                        }
                    }
                }
            }
        }

        if(is_satisfy_all_pre){
            return true;
        } else{
            return false;
        }
    }
}




bool
GlobalIndex::online_process(OfflineIndex *tc_offline_index, Search *tc_search, MatchOrder *tc_order, Arg *tc_arg,
                            GIndex *tc_g_index, IO *tc_io) {

    for(auto &edge_:tc_io->S_update){
        Label label{edge_.v_source_label, edge_.v_target_label, edge_.edge_label};

        auto finder = tc_offline_index->label_to_all_q_edge.find(label);

        if(finder != tc_offline_index->label_to_all_q_edge.end()){
            for(auto &key:finder->second){
                auto all_pre_keys = tc_offline_index->q_edge_to_all_prec_time_order[key];
                if(all_pre_keys.empty()){

                    if(tc_arg->is_using_static_merge == "y"){

                        update_global_index_consider_duplication(key, edge_, tc_offline_index, tc_search, tc_order,
                                                                 tc_arg, tc_g_index);

                    } else if(tc_arg->is_using_static_merge == "n"){

                        update_global_index_without_consider_duplication(key, edge_, tc_offline_index, tc_search, tc_order,
                                                                         tc_arg, tc_g_index);

                    } else{
                        std::cerr << "Incorrect argument format : [is_using_static_merge] " << std::endl;
                        exit(1);
                    }
                } else{
                    if(is_satisfy_time_constrain(key, edge_, all_pre_keys, tc_offline_index)){


                        if(tc_arg->is_using_static_merge == "y"){

                            update_global_index_consider_duplication(key, edge_, tc_offline_index, tc_search, tc_order,
                                                                     tc_arg, tc_g_index);

                        }else if(tc_arg->is_using_static_merge == "n"){

                            update_global_index_without_consider_duplication(key, edge_, tc_offline_index, tc_search,
                                                                             tc_order, tc_arg, tc_g_index);

                        } else{
                            std::cerr << "Incorrect argument format : [is_using_static_merge] " << std::endl;
                            exit(1);
                        }
                    }
                }
            }
        } else{
            continue;
        }
    }


    return true;
}

void
GlobalIndex::update_global_index_consider_duplication(IndexKeyItem &key, SEdge &edge_, OfflineIndex *tc_offline_index,
                                                      Search *tc_search, MatchOrder *tc_order, Arg *tc_arg,
                                                      GIndex *tc_g_index) {


    std::string edge_to_str = std::to_string(edge_.v_source_id) + "_" + std::to_string(edge_.v_target_id) + "_" + std::to_string(edge_.edge_label) + "_" + std::to_string(edge_.v_source_label) + "_" + std::to_string(edge_.v_target_label);

    if(this->fht.at(key).find(edge_to_str) != this->fht.at(key).end()){

        this->duplicated_edge_count++;

        auto existed_value_pos = this->fht.at(key).at(edge_to_str);
        auto &existed_value = this->global_index.at(key).at(existed_value_pos);

        auto &all_succ = tc_offline_index->q_edge_to_all_succ.at(existed_value.self_key);

        bool is_no_succ = true;

        for(auto &succ_it:all_succ){
            if(existed_value.neighbor_link.find(succ_it) != existed_value.neighbor_link.end()){
                if(!existed_value.neighbor_link.at(succ_it).empty()){
                    is_no_succ = false;
                    break;
                }
            }
        }


        if(is_no_succ){

            uit prec_nei_num = 0;

            for(auto &prec_it:tc_offline_index->q_edge_to_all_prec_time_order.at(existed_value.self_key)){
                if(existed_value.neighbor_link.find(prec_it) != existed_value.neighbor_link.end()){
                    prec_nei_num += existed_value.neighbor_link.at(prec_it).size();
                }

            }

            uit system_prec_nei_num = 0;

            for(auto &prec_it:tc_offline_index->q_edge_to_all_prec_time_order.at(existed_value.self_key)){

                std::string us_ut_el_sl_tl = std::to_string(prec_it.qe.u_source_id) + "_" + std::to_string(prec_it.qe.u_target_id) + "_" + std::to_string(prec_it.qe.u_edge_label) + "_" + std::to_string(prec_it.qe.u_source_label) + "_" + std::to_string(prec_it.qe.u_target_label);

                if(existed_value.self_key.qe.u_source_id == prec_it.qe.u_source_id){ //ss
                    if(tc_g_index->g_index.find(existed_value.v_source_id) != tc_g_index->g_index.end()){
                        if(tc_g_index->g_index.at(existed_value.v_source_id).find(us_ut_el_sl_tl) != tc_g_index->g_index.at(existed_value.v_source_id).end()){
                            if(tc_g_index->g_index.at(existed_value.v_source_id).at(us_ut_el_sl_tl).find("s") != tc_g_index->g_index.at(existed_value.v_source_id).at(us_ut_el_sl_tl).end()){
                                system_prec_nei_num += tc_g_index->g_index.at(existed_value.v_source_id).at(us_ut_el_sl_tl).at("s").size();
                            }
                        }
                    }
                } else if(existed_value.self_key.qe.u_source_id == prec_it.qe.u_target_id){ //st
                    if(tc_g_index->g_index.find(existed_value.v_source_id) != tc_g_index->g_index.end()){
                        if(tc_g_index->g_index.at(existed_value.v_source_id).find(us_ut_el_sl_tl) != tc_g_index->g_index.at(existed_value.v_source_id).end()){
                            if(tc_g_index->g_index.at(existed_value.v_source_id).at(us_ut_el_sl_tl).find("t") != tc_g_index->g_index.at(existed_value.v_source_id).at(us_ut_el_sl_tl).end()){
                                system_prec_nei_num += tc_g_index->g_index.at(existed_value.v_source_id).at(us_ut_el_sl_tl).at("t").size();
                            }
                        }
                    }

                } else if(existed_value.self_key.qe.u_target_id == prec_it.qe.u_source_id){  //ts
                    if(tc_g_index->g_index.find(existed_value.v_target_id) != tc_g_index->g_index.end()){
                        if(tc_g_index->g_index.at(existed_value.v_target_id).find(us_ut_el_sl_tl) != tc_g_index->g_index.at(existed_value.v_target_id).end()){
                            if(tc_g_index->g_index.at(existed_value.v_target_id).at(us_ut_el_sl_tl).find("s") != tc_g_index->g_index.at(existed_value.v_target_id).at(us_ut_el_sl_tl).end()){
                                system_prec_nei_num += tc_g_index->g_index.at(existed_value.v_target_id).at(us_ut_el_sl_tl).at("s").size();
                            }
                        }
                    }

                } else if(existed_value.self_key.qe.u_target_id == prec_it.qe.u_target_id){ //tt
                    if(tc_g_index->g_index.find(existed_value.v_target_id) != tc_g_index->g_index.end()){
                        if(tc_g_index->g_index.at(existed_value.v_target_id).find(us_ut_el_sl_tl) != tc_g_index->g_index.at(existed_value.v_target_id).end()){
                            if(tc_g_index->g_index.at(existed_value.v_target_id).at(us_ut_el_sl_tl).find("t") != tc_g_index->g_index.at(existed_value.v_target_id).at(us_ut_el_sl_tl).end()){
                                system_prec_nei_num += tc_g_index->g_index.at(existed_value.v_target_id).at(us_ut_el_sl_tl).at("t").size();
                            }
                        }
                    }

                }
            }


            if(system_prec_nei_num > prec_nei_num){

                this->because_succ_edge_count++;

                this->global_index.at(key).emplace_back(edge_.v_source_id, edge_.v_target_id, edge_.edge_label,
                                                        edge_.v_source_label,edge_.v_target_label,  edge_.timestamp, key,
                                                        tc_offline_index->q_edge_to_all_neighbor[key]);
                IndexValueItem &last_insert = this->global_index.at(key).back();
                last_insert.self_pos = this->global_index.at(key).size() - 1;

                this->save_fht(last_insert);

                tc_g_index->save_g_edge(last_insert);

                this->connection_time_start = T_NOW;

                create_double_link(last_insert, tc_offline_index, tc_g_index, tc_arg);

                this->connection_time_end = T_NOW;
                this->connection_time_span += this->connection_time_end - this->connection_time_start;

                last_insert.update_self_status();


                if (last_insert.status) {

                    this->search_time_start = T_NOW;

                    tc_search->direct_search_matching(last_insert, tc_order, tc_offline_index,tc_arg, this);

                    this->search_time_end = T_NOW;
                    this->search_time_span += this->search_time_end - this->search_time_start;
                }

            }


            else if(system_prec_nei_num == prec_nei_num){

                existed_value.duplicated_edge_timestamp.emplace_back(edge_.timestamp);

                if(existed_value.status){
                    this->search_time_start = T_NOW;


                    tc_search->direct_search_matching_with_specified_timestamp(existed_value, edge_.timestamp, tc_order,
                                                                               tc_offline_index, tc_arg, this);

                    this->search_time_end = T_NOW;
                    this->search_time_span += this->search_time_end - this->search_time_start;
                }

            } else{
                std::cerr << "error: system_prec_nei_num < prec_nei_num" << std::endl;
                exit(-1);
            }

            //==================================================



        } else{

            this->because_copy_link_count++;


            auto existed_value_ = existed_value;  //Make a copy

            this->global_index.at(key).emplace_back(edge_.v_source_id, edge_.v_target_id, edge_.edge_label,
                                                    edge_.v_source_label,edge_.v_target_label,  edge_.timestamp, key,
                                                    tc_offline_index->q_edge_to_all_neighbor[key]);
            IndexValueItem &last_insert = this->global_index.at(key).back();
            last_insert.self_pos = this->global_index.at(key).size() - 1;

            this->save_fht(last_insert);

            tc_g_index->save_g_edge(last_insert);

            //use the "existed_value"'s copy
            copy_double_link(existed_value_, last_insert, tc_offline_index);

        }

    } else{

        this->global_index.at(key).emplace_back(edge_.v_source_id, edge_.v_target_id, edge_.edge_label,
                                                edge_.v_source_label,edge_.v_target_label,  edge_.timestamp, key,
                                                tc_offline_index->q_edge_to_all_neighbor[key]);
        IndexValueItem &last_insert = this->global_index.at(key).back();
        last_insert.self_pos = this->global_index.at(key).size() - 1;

        this->save_fht(last_insert);

        tc_g_index->save_g_edge(last_insert);

        this->connection_time_start = T_NOW;

        create_double_link(last_insert, tc_offline_index, tc_g_index, tc_arg);

        this->connection_time_end = T_NOW;
        this->connection_time_span += this->connection_time_end - this->connection_time_start;

        last_insert.update_self_status();


        if (last_insert.status) {

            this->search_time_start = T_NOW;

            tc_search->direct_search_matching(last_insert, tc_order, tc_offline_index, tc_arg,
                                              this);

            this->search_time_end = T_NOW;
            this->search_time_span += this->search_time_end - this->search_time_start;
        }

    }

}

void GlobalIndex::update_global_index_without_consider_duplication(IndexKeyItem &key, SEdge &edge_,
                                                                   OfflineIndex *tc_offline_index, Search *tc_search,
                                                                   MatchOrder *tc_order, Arg *tc_arg,
                                                                   GIndex *tc_g_index) {

    this->global_index.at(key).emplace_back(edge_.v_source_id, edge_.v_target_id, edge_.edge_label,
                                            edge_.v_source_label,edge_.v_target_label,  edge_.timestamp, key,
                                            tc_offline_index->q_edge_to_all_neighbor[key]);

    IndexValueItem &last_insert = this->global_index[key].back();
    last_insert.self_pos = this->global_index[key].size() - 1;

    tc_g_index->save_g_edge(last_insert);

    this->save_fht(last_insert);

    create_double_link(last_insert, tc_offline_index, tc_g_index, tc_arg);

    last_insert.update_self_status();

    if (last_insert.status) {
        this->search_time_start = T_NOW;

        tc_search->direct_search_matching(last_insert, tc_order, tc_offline_index, tc_arg, this);

        this->search_time_end = T_NOW;
        this->search_time_span += this->search_time_end - this->search_time_start;
    }
}

void GlobalIndex::create_double_link(IndexValueItem &value, OfflineIndex *tc_offline_index, GIndex *tc_g_index,
                                     Arg *tc_arg) {

    std::vector<IndexKeyItem> valid_key;

    for(auto &nei_key:tc_offline_index->q_edge_to_all_neighbor[value.self_key]){
        if(std::find(tc_offline_index->q_edge_to_all_succ[value.self_key].begin(), tc_offline_index->q_edge_to_all_succ[value.self_key].end(), nei_key) == tc_offline_index->q_edge_to_all_succ[value.self_key].end()){
            valid_key.emplace_back(nei_key);
        }
    }



    for(auto &nei_key:valid_key){

        //==================================================

        std::string us_ut_el_sl_tl = std::to_string(nei_key.qe.u_source_id) + "_" + std::to_string(nei_key.qe.u_target_id) + "_" + std::to_string(nei_key.qe.u_edge_label) + "_" + std::to_string(nei_key.qe.u_source_label) + "_" + std::to_string(nei_key.qe.u_target_label);


        if(value.self_key.qe.u_source_id == nei_key.qe.u_source_id){ //ss

            if(tc_g_index->g_index.find(value.v_source_id) == tc_g_index->g_index.end()){
                break;
            }

            if(tc_g_index->g_index.at(value.v_source_id).find(us_ut_el_sl_tl) == tc_g_index->g_index.at(value.v_source_id).end()){
                continue;
            }

            if(tc_g_index->g_index.at(value.v_source_id).at(us_ut_el_sl_tl).find("s") == tc_g_index->g_index.at(value.v_source_id).at(us_ut_el_sl_tl).end()){
                continue;
            }

            auto &all_suitable_pos = tc_g_index->g_index.at(value.v_source_id).at(us_ut_el_sl_tl).at("s");


            for(auto &nei_pos:all_suitable_pos){

                if(this->global_index.at(nei_key).empty() || nei_pos > this->global_index.at(nei_key).size() - 1){
                    continue;
                }


                auto &nei_value = this->global_index.at(nei_key).at(nei_pos);


                if(nei_value.timestamp == value.timestamp){
                    continue;
                } else{
                    value.save_in_neighbor_link(nei_value.self_key, nei_value.self_pos);
                    nei_value.save_in_neighbor_link(value.self_key, value.self_pos);

                    value.update_self_status();
                    nei_value.update_self_status();
                }

            }

        } else if(value.self_key.qe.u_source_id == nei_key.qe.u_target_id){  //st

            if(tc_g_index->g_index.find(value.v_source_id) == tc_g_index->g_index.end()){
                break;
            }

            if(tc_g_index->g_index.at(value.v_source_id).find(us_ut_el_sl_tl) == tc_g_index->g_index.at(value.v_source_id).end()){
                continue;
            }

            if(tc_g_index->g_index.at(value.v_source_id).at(us_ut_el_sl_tl).find("t") == tc_g_index->g_index.at(value.v_source_id).at(us_ut_el_sl_tl).end()){
                continue;
            }

            auto &all_suitable_pos = tc_g_index->g_index.at(value.v_source_id).at(us_ut_el_sl_tl).at("t");


            for(auto &nei_pos:all_suitable_pos){

                if(this->global_index.at(nei_key).empty() || nei_pos > this->global_index.at(nei_key).size() - 1){
                    continue;
                }


                auto &nei_value = this->global_index.at(nei_key).at(nei_pos);


                if(nei_value.timestamp == value.timestamp){
                    continue;
                } else{
                    value.save_in_neighbor_link(nei_value.self_key, nei_value.self_pos);
                    nei_value.save_in_neighbor_link(value.self_key, value.self_pos);


                    value.update_self_status();
                    nei_value.update_self_status();
                }

            }
        } else if(value.self_key.qe.u_target_id == nei_key.qe.u_target_id){ //tt

            if(tc_g_index->g_index.find(value.v_target_id) == tc_g_index->g_index.end()){
                break;
            }

            if(tc_g_index->g_index.at(value.v_target_id).find(us_ut_el_sl_tl) == tc_g_index->g_index.at(value.v_target_id).end()){
                continue;
            }

            if(tc_g_index->g_index.at(value.v_target_id).at(us_ut_el_sl_tl).find("t") == tc_g_index->g_index.at(value.v_target_id).at(us_ut_el_sl_tl).end()){
                continue;
            }

            auto &all_suitable_pos = tc_g_index->g_index.at(value.v_target_id).at(us_ut_el_sl_tl).at("t");


            for(auto &nei_pos:all_suitable_pos){

                if(this->global_index.at(nei_key).empty() || nei_pos > this->global_index.at(nei_key).size() - 1){
                    continue;
                }


                auto &nei_value = this->global_index.at(nei_key).at(nei_pos);


                if(nei_value.timestamp == value.timestamp){
                    continue;
                } else{
                    value.save_in_neighbor_link(nei_value.self_key, nei_value.self_pos);
                    nei_value.save_in_neighbor_link(value.self_key, value.self_pos);


                    value.update_self_status();
                    nei_value.update_self_status();
                }

            }

        } else if(value.self_key.qe.u_target_id == nei_key.qe.u_source_id){ //ts

            if(tc_g_index->g_index.find(value.v_target_id) == tc_g_index->g_index.end()){
                break;
            }

            if(tc_g_index->g_index.at(value.v_target_id).find(us_ut_el_sl_tl) == tc_g_index->g_index.at(value.v_target_id).end()){
                continue;
            }

            if(tc_g_index->g_index.at(value.v_target_id).at(us_ut_el_sl_tl).find("s") == tc_g_index->g_index.at(value.v_target_id).at(us_ut_el_sl_tl).end()){
                continue;
            }

            auto &all_suitable_pos = tc_g_index->g_index.at(value.v_target_id).at(us_ut_el_sl_tl).at("s");



            for(auto &nei_pos:all_suitable_pos){

                if(this->global_index.at(nei_key).empty() || nei_pos > this->global_index.at(nei_key).size() - 1){
                    continue;
                }


                auto &nei_value = this->global_index.at(nei_key).at(nei_pos);


                if(nei_value.timestamp == value.timestamp){
                    continue;
                } else{
                    value.save_in_neighbor_link(nei_value.self_key, nei_value.self_pos);
                    nei_value.save_in_neighbor_link(value.self_key, value.self_pos);

                    value.update_self_status();
                    nei_value.update_self_status();
                }

            }

        }

        //===========================
    }
}

void GlobalIndex::copy_double_link(IndexValueItem &existed_value, IndexValueItem &new_value,
                                   OfflineIndex *tc_offline_index) {


    std::vector<IndexKeyItem> valid_key;

    for(auto &nei_key:tc_offline_index->q_edge_to_all_neighbor[existed_value.self_key]){

        if(std::find(tc_offline_index->q_edge_to_all_succ[existed_value.self_key].begin(), tc_offline_index->q_edge_to_all_succ[existed_value.self_key].end(), nei_key) == tc_offline_index->q_edge_to_all_succ[existed_value.self_key].end()){
            valid_key.emplace_back(nei_key);
        }
    }


    for(auto &nei_key:valid_key){
        auto &existed_nei_pos = existed_value.neighbor_link.at(nei_key);
        for(auto &pos:existed_nei_pos){
            auto &nei_value = this->global_index.at(nei_key).at(pos);


            new_value.save_in_neighbor_link(nei_value.self_key, nei_value.self_pos);
            nei_value.save_in_neighbor_link(new_value.self_key, new_value.self_pos);


            new_value.update_self_status();
            nei_value.update_self_status();
        }
    }
}

bool GlobalIndex::index_incremental_update(SEdge &edge_, OfflineIndex *tc_offline_index, Arg *tc_arg, GIndex *tc_g_index) {

    Label label{edge_.v_source_label, edge_.v_target_label, edge_.edge_label};

    auto finder = tc_offline_index->label_to_all_q_edge.find(label);

    if(finder != tc_offline_index->label_to_all_q_edge.end()){
        for(auto &key:finder->second){
            auto all_pre_keys = tc_offline_index->q_edge_to_all_prec_time_order[key];
            if(all_pre_keys.empty()){

                if(tc_arg->is_using_static_merge == "y"){

                    index_update_global_index_consider_duplication(key, edge_, tc_offline_index, tc_g_index, tc_arg);

                } else if(tc_arg->is_using_static_merge == "n"){

                    index_update_global_index_without_consider_duplication(key, edge_, tc_offline_index, tc_g_index,
                                                                           tc_arg);

                } else{
                    std::cerr << "Incorrect argument format : [is_using_static_merge] " << std::endl;
                    exit(1);
                }
            } else{
                if(is_satisfy_time_constrain(key, edge_, all_pre_keys, tc_offline_index)){

                    if(tc_arg->is_using_static_merge == "y"){

                        index_update_global_index_consider_duplication(key, edge_, tc_offline_index, tc_g_index,
                                                                       tc_arg);

                    }else if(tc_arg->is_using_static_merge == "n"){

                        index_update_global_index_without_consider_duplication(key, edge_, tc_offline_index, tc_g_index,
                                                                               tc_arg);

                    } else{
                        std::cerr << "Incorrect argument format : [is_using_static_merge] " << std::endl;
                        exit(1);
                    }
                }
            }
        }
    } else{
        return false;
    }


    return true;
}

void
GlobalIndex::index_update_global_index_consider_duplication(IndexKeyItem &key, SEdge &edge_,
                                                            OfflineIndex *tc_offline_index, GIndex *tc_g_index,
                                                            Arg *tc_arg) {

    std::string edge_to_str = std::to_string(edge_.v_source_id) + "_" + std::to_string(edge_.v_target_id) + "_" + std::to_string(edge_.edge_label) + "_" + std::to_string(edge_.v_source_label) + "_" + std::to_string(edge_.v_target_label);

    if(this->fht.at(key).find(edge_to_str) != this->fht.at(key).end()){

        this->duplicated_edge_count++;

        auto &existed_value_pos = this->fht.at(key).at(edge_to_str);
        auto &existed_value = this->global_index.at(key).at(existed_value_pos);

        auto &all_succ = tc_offline_index->q_edge_to_all_succ.at(existed_value.self_key);

        bool is_no_succ = true;

        for(auto &succ_it:all_succ){

            if(existed_value.neighbor_link.find(succ_it) != existed_value.neighbor_link.end()){
                if(!existed_value.neighbor_link.at(succ_it).empty()){
                    is_no_succ = false;
                    break;
                }
            }
        }


        if(is_no_succ){

            uit prec_nei_num = 0;

            for(auto &prec_it:tc_offline_index->q_edge_to_all_prec_time_order.at(existed_value.self_key)){

                if(existed_value.neighbor_link.find(prec_it) != existed_value.neighbor_link.end()){
                    prec_nei_num += existed_value.neighbor_link.at(prec_it).size();
                }

            }


            uit system_prec_nei_num = 0;

            for(auto &prec_it:tc_offline_index->q_edge_to_all_prec_time_order.at(existed_value.self_key)){

                std::string us_ut_el_sl_tl = std::to_string(prec_it.qe.u_source_id) + "_" + std::to_string(prec_it.qe.u_target_id) + "_" + std::to_string(prec_it.qe.u_edge_label) + "_" + std::to_string(prec_it.qe.u_source_label) + "_" + std::to_string(prec_it.qe.u_target_label);

                if(existed_value.self_key.qe.u_source_id == prec_it.qe.u_source_id){ //ss
                    if(tc_g_index->g_index.find(existed_value.v_source_id) != tc_g_index->g_index.end()){
                        if(tc_g_index->g_index.at(existed_value.v_source_id).find(us_ut_el_sl_tl) != tc_g_index->g_index.at(existed_value.v_source_id).end()){
                            if(tc_g_index->g_index.at(existed_value.v_source_id).at(us_ut_el_sl_tl).find("s") != tc_g_index->g_index.at(existed_value.v_source_id).at(us_ut_el_sl_tl).end()){
                                system_prec_nei_num += tc_g_index->g_index.at(existed_value.v_source_id).at(us_ut_el_sl_tl).at("s").size();
                            }
                        }
                    }
                } else if(existed_value.self_key.qe.u_source_id == prec_it.qe.u_target_id){ //st
                    if(tc_g_index->g_index.find(existed_value.v_source_id) != tc_g_index->g_index.end()){
                        if(tc_g_index->g_index.at(existed_value.v_source_id).find(us_ut_el_sl_tl) != tc_g_index->g_index.at(existed_value.v_source_id).end()){
                            if(tc_g_index->g_index.at(existed_value.v_source_id).at(us_ut_el_sl_tl).find("t") != tc_g_index->g_index.at(existed_value.v_source_id).at(us_ut_el_sl_tl).end()){
                                system_prec_nei_num += tc_g_index->g_index.at(existed_value.v_source_id).at(us_ut_el_sl_tl).at("t").size();
                            }
                        }
                    }

                } else if(existed_value.self_key.qe.u_target_id == prec_it.qe.u_source_id){  //ts
                    if(tc_g_index->g_index.find(existed_value.v_target_id) != tc_g_index->g_index.end()){
                        if(tc_g_index->g_index.at(existed_value.v_target_id).find(us_ut_el_sl_tl) != tc_g_index->g_index.at(existed_value.v_target_id).end()){
                            if(tc_g_index->g_index.at(existed_value.v_target_id).at(us_ut_el_sl_tl).find("s") != tc_g_index->g_index.at(existed_value.v_target_id).at(us_ut_el_sl_tl).end()){
                                system_prec_nei_num += tc_g_index->g_index.at(existed_value.v_target_id).at(us_ut_el_sl_tl).at("s").size();
                            }
                        }
                    }

                } else if(existed_value.self_key.qe.u_target_id == prec_it.qe.u_target_id){ //tt
                    if(tc_g_index->g_index.find(existed_value.v_target_id) != tc_g_index->g_index.end()){
                        if(tc_g_index->g_index.at(existed_value.v_target_id).find(us_ut_el_sl_tl) != tc_g_index->g_index.at(existed_value.v_target_id).end()){
                            if(tc_g_index->g_index.at(existed_value.v_target_id).at(us_ut_el_sl_tl).find("t") != tc_g_index->g_index.at(existed_value.v_target_id).at(us_ut_el_sl_tl).end()){
                                system_prec_nei_num += tc_g_index->g_index.at(existed_value.v_target_id).at(us_ut_el_sl_tl).at("t").size();
                            }
                        }
                    }

                }
            }


            if(system_prec_nei_num > prec_nei_num){

                this->global_index.at(key).emplace_back(edge_.v_source_id, edge_.v_target_id, edge_.edge_label,
                                                        edge_.v_source_label,edge_.v_target_label,  edge_.timestamp, key,
                                                        tc_offline_index->q_edge_to_all_neighbor[key]);
                IndexValueItem &last_insert = this->global_index.at(key).back();
                last_insert.self_pos = this->global_index.at(key).size() - 1;

                this->save_fht(last_insert);

                tc_g_index->save_g_edge(last_insert);


                create_double_link(last_insert, tc_offline_index, tc_g_index, tc_arg);

                last_insert.update_self_status();

            }

            else if(system_prec_nei_num == prec_nei_num){

                existed_value.duplicated_edge_timestamp.emplace_back(edge_.timestamp);


            } else{
                std::cerr << "error: system_prec_nei_num < prec_nei_num" << std::endl;
                exit(-1);
            }

            //==================================================

        } else{

            auto existed_value_ = existed_value;  //Make a copy

            this->global_index.at(key).emplace_back(edge_.v_source_id, edge_.v_target_id, edge_.edge_label,
                                                    edge_.v_source_label,edge_.v_target_label,  edge_.timestamp, key,
                                                    tc_offline_index->q_edge_to_all_neighbor[key]);
            IndexValueItem &last_insert = this->global_index.at(key).back();
            last_insert.self_pos = this->global_index.at(key).size() - 1;

            this->save_fht(last_insert);


            tc_g_index->save_g_edge(last_insert);

            //use the "existed_value"'s copy
            copy_double_link(existed_value_, last_insert, tc_offline_index);

        }

    } else{

        this->global_index.at(key).emplace_back(edge_.v_source_id, edge_.v_target_id, edge_.edge_label,
                                                edge_.v_source_label,edge_.v_target_label,  edge_.timestamp, key,
                                                tc_offline_index->q_edge_to_all_neighbor[key]);
        IndexValueItem &last_insert = this->global_index.at(key).back();
        last_insert.self_pos = this->global_index.at(key).size() - 1;

        this->save_fht(last_insert);

        tc_g_index->save_g_edge(last_insert);


        create_double_link(last_insert, tc_offline_index, tc_g_index, tc_arg);

        last_insert.update_self_status();

    }
}

void GlobalIndex::index_update_global_index_without_consider_duplication(IndexKeyItem &key, SEdge &edge_,
                                                                         OfflineIndex *tc_offline_index,
                                                                         GIndex *tc_g_index, Arg *tc_arg) {

    this->global_index.at(key).emplace_back(edge_.v_source_id, edge_.v_target_id, edge_.edge_label,
                                            edge_.v_source_label,edge_.v_target_label,  edge_.timestamp, key,
                                            tc_offline_index->q_edge_to_all_neighbor[key]);

    IndexValueItem &last_insert = this->global_index[key].back();
    last_insert.self_pos = this->global_index[key].size() - 1;

    tc_g_index->save_g_edge(last_insert);

    this->save_fht(last_insert);

    create_double_link(last_insert, tc_offline_index, tc_g_index, tc_arg);
    last_insert.update_self_status();

}
