#include "tc_search.h"
#include "tc_arg.h"

void
Search::direct_search_matching(IndexValueItem &value, MatchOrder *tc_order, OfflineIndex *tc_offline_index, Arg *tc_arg,
                               GlobalIndex *tc_global_index) {


    if(tc_order->search_order.find(value.self_key) == tc_order->search_order.end()){
        return;
    }

    auto &_search_order = tc_order->search_order.at(value.self_key);

    std::unordered_map<IndexKeyItem, IndexValueItem, IndexKeyItemHash> one_result;


    if(tc_arg->is_using_dynamic_merge == "n" || (tc_arg->is_using_dynamic_merge == "y" && tc_global_index->duplicated_edge_count == 0)){
        recursive_search(value, one_result, _search_order, tc_global_index->global_index, tc_offline_index, tc_arg);
    } else if(tc_arg->is_using_dynamic_merge == "y"){
        recursive_search_with_dynamic_merge(value, one_result, _search_order, tc_global_index->global_index, tc_offline_index, tc_arg);
    } else{
        std::cerr << "Incorrect argument format : [is_using_static_merge] " << std::endl;
        exit(1);
    }




}

void Search::direct_search_matching_with_specified_timestamp(IndexValueItem &value, uit duplicated_timestamp,
                                                             MatchOrder *tc_order, OfflineIndex *tc_offline_index,
                                                             Arg *tc_arg, GlobalIndex *tc_global_index) {




    if(tc_order->search_order.find(value.self_key) == tc_order->search_order.end()){
        return;
    }


    auto &_search_order = tc_order->search_order.at(value.self_key);


    std::unordered_map<IndexKeyItem, IndexValueItem, IndexKeyItemHash> one_result;


    IndexValueItem copy_value = value;
    copy_value.timestamp = duplicated_timestamp;


    if(tc_arg->is_using_dynamic_merge == "n" || (tc_arg->is_using_dynamic_merge == "y" && tc_global_index->duplicated_edge_count == 0)){
        recursive_search(copy_value, one_result, _search_order, tc_global_index->global_index, tc_offline_index, tc_arg);
    } else if(tc_arg->is_using_dynamic_merge == "y"){
        recursive_search_with_dynamic_merge(copy_value, one_result, _search_order, tc_global_index->global_index, tc_offline_index, tc_arg);
    } else{
        std::cerr << "Incorrect argument format : [is_using_static_merge] " << std::endl;
        exit(1);
    }

}

void Search::recursive_search(IndexValueItem &value,
                              std::unordered_map<IndexKeyItem, IndexValueItem, IndexKeyItemHash> &one_result,
                              std::vector<IndexKeyItem> &_search_order,
                              std::unordered_map<IndexKeyItem, std::vector<IndexValueItem>, IndexKeyItemHash> &global_index,
                              OfflineIndex *tc_offline_index, Arg *tc_arg) {


    if(value.self_key == _search_order.front()){ // If it is the starting point.

        one_result[value.self_key] = value;

        IndexKeyItem* next_order = get_next_order(value.self_key, _search_order);

        for(auto &nei_pos:value.neighbor_link[*next_order]){
            auto &nei_value = global_index[*next_order].at(nei_pos);
            if(nei_value.status){
                recursive_search(nei_value, one_result, _search_order, global_index, tc_offline_index, tc_arg);
            }
        }

    }else if(value.self_key == _search_order.back()) { // If it is the endpoint.

        IndexKeyItem* pre_order = get_pre_order(value.self_key, _search_order);

        //===========
        this->check_time_start = T_NOW;
        //===========


        if(tc_offline_index->nc_edge_to_pre.find(value.self_key) != tc_offline_index->nc_edge_to_pre.end()){
            for(auto &pre_key:tc_offline_index->nc_edge_to_pre[value.self_key]){

                if(one_result.find(pre_key) != one_result.end()){
                    if(one_result[pre_key].timestamp < value.timestamp){
                        continue;
                    } else{
                        one_result.erase(value.self_key);
                        return;
                    }
                }
            }
        }


        for(auto &it: tc_offline_index->nc_pair_vec){
            auto &nc_later_key = it.first;
            auto &nc_pre_key = it.second;
            if(value.self_key == nc_pre_key){

                if(one_result.find(nc_later_key) != one_result.end()){
                    if(one_result[nc_later_key].timestamp > value.timestamp){
                        continue;
                    } else{
                        one_result.erase(value.self_key);
                        return;
                    }
                }
            }
        }


        for(auto &nei_it:value.neighbor_link){
            auto &nei_key = nei_it.first;

            if(one_result.find(nei_key) != one_result.end()){

                if(is_existed_connection(value,one_result.at(nei_key))){
                    continue;
                } else{
                    one_result.erase(value.self_key);
                    return;
                }


            }

            else{
                one_result.erase(value.self_key);
                return;
            }
        }


        //===========
        this->check_time_end = T_NOW;
        this->check_time_span += this->check_time_end - this->check_time_start;
        //===========



        one_result[value.self_key] = value; //Save endpoint.

        this->union_match_count += 1;

        //==========================================

        //==================
        this->enum_time_start = T_NOW;
        //==================

        std::unordered_map<IndexKeyItem,std::vector<uit>,IndexKeyItemHash> key_to_time;

        for(auto &result_it:one_result){
            auto &result_key = result_it.first;
            auto &result_value = result_it.second;

            if(result_key == _search_order.front()){
                continue;
            }

            if(!result_value.duplicated_edge_timestamp.empty()){
                key_to_time[result_key] = result_value.duplicated_edge_timestamp;
            }
        }

        //===========
        this->enum_time_end = T_NOW;
        this->enum_time_span += this->enum_time_end - this->enum_time_start;
        //===========


        std::vector<std::unordered_map<IndexKeyItem, IndexValueItem, IndexKeyItemHash>> vec_one_result;


        vec_one_result.emplace_back(one_result);

        if(tc_arg->execute_mode == "enum"){

            for(auto &map_it:key_to_time){
                auto &key = map_it.first;
                auto &times = map_it.second;

                std::vector<std::unordered_map<IndexKeyItem, IndexValueItem, IndexKeyItemHash>> new_vec_one_result;

                for(auto result_it:vec_one_result){
                    for(auto &time_it:times){
                        result_it.at(key).timestamp = time_it;
                        new_vec_one_result.emplace_back(result_it);
                    }
                }


                for(auto &new_map_it:new_vec_one_result){
                    vec_one_result.emplace_back(new_map_it);
                }

            }


            std::vector<ResultEdge> result_vec;
            for(auto &one_result_it:vec_one_result){
                for(auto &it:one_result_it){
                    ResultEdge re;
                    re.q_id = tc_offline_index->q_edge_to_q_id.at(it.second.self_key);
                    re.source_id = it.second.v_source_id;
                    re.source_label = it.second.v_source_label;
                    re.target_id = it.second.v_target_id;
                    re.target_label = it.second.v_target_label;
                    re.edge_label = it.second.edge_label;
                    re.timestamp = it.second.timestamp;
                    result_vec.emplace_back(re);
                }

                this->match_result.emplace_back(result_vec);
                this->match_count += 1;

                result_vec.clear();
            }

        } else if(tc_arg->execute_mode == "count"){
            uit duplicated_result_count = 1;

            for(auto &map_it:key_to_time){
                auto &times = map_it.second;
                duplicated_result_count *= times.size() + 1;
            }

            this->match_count = this->match_count + duplicated_result_count;
        }


        one_result.erase(value.self_key);
        return;
    } else{

        IndexKeyItem* pre_order = get_pre_order(value.self_key, _search_order);
        IndexKeyItem* next_order = get_next_order(value.self_key, _search_order);

        //===========
        this->check_time_start = T_NOW;
        //===========


        if(tc_offline_index->nc_edge_to_pre.find(value.self_key) != tc_offline_index->nc_edge_to_pre.end()){
            for(auto &pre_key:tc_offline_index->nc_edge_to_pre[value.self_key]){

                if(one_result.find(pre_key) != one_result.end()){
                    if(one_result[pre_key].timestamp < value.timestamp){
                        continue;
                    } else{
                        one_result.erase(value.self_key);
                        return;
                    }
                }
            }
        }


        for(auto &it: tc_offline_index->nc_pair_vec){
            auto &nc_later_key = it.first;
            auto &nc_pre_key = it.second;
            if(value.self_key == nc_pre_key){

                if(one_result.find(nc_later_key) != one_result.end()){
                    if(one_result[nc_later_key].timestamp > value.timestamp){
                        continue;
                    } else{
                        one_result.erase(value.self_key);
                        return;
                    }
                }
            }
        }

        for(auto &nei_it:value.neighbor_link){
            auto &nei_key = nei_it.first;


            if(one_result.find(nei_key) != one_result.end()){

                if(is_existed_connection(value,one_result.at(nei_key))){
                    continue;
                } else{
                    one_result.erase(value.self_key);
                    return;
                }

            }
        }

        //===========
        this->check_time_end = T_NOW;
        this->check_time_span += this->check_time_end - this->check_time_start;
        //===========

        one_result[value.self_key] = value;


        if(value.neighbor_link.find(*next_order) != value.neighbor_link.end()){
            for(auto &nei_pos:value.neighbor_link.at(*next_order)){
                auto &nei_value = global_index.at(*next_order).at(nei_pos);
                if(nei_value.status){
                    recursive_search(nei_value, one_result, _search_order, global_index, tc_offline_index, tc_arg);
                }
            }
        } else{
            for(auto &it:one_result){
                auto &existed_key = it.first;
                auto &existed_value = it.second;

                if(existed_key.qe.u_source_id == next_order->qe.u_source_id || existed_key.qe.u_source_id == next_order->qe.u_target_id || existed_key.qe.u_target_id == next_order->qe.u_source_id || existed_key.qe.u_target_id == next_order->qe.u_target_id){

                    if(existed_value.neighbor_link.find(*next_order) != existed_value.neighbor_link.end()){
                        for(auto &nei_pos:existed_value.neighbor_link.at(*next_order)){
                            auto &nei_value = global_index.at(*next_order).at(nei_pos);

                            if(nei_value.status){
                                recursive_search(nei_value, one_result, _search_order, global_index, tc_offline_index,
                                                 tc_arg);
                            }
                        }
                        break;
                    }
                }
            }
        }


        one_result.erase(value.self_key);


    }
}






void Search::recursive_search_with_dynamic_merge(IndexValueItem &value,
                                                 std::unordered_map<IndexKeyItem, IndexValueItem, IndexKeyItemHash> &one_result,
                                                 std::vector<IndexKeyItem> &_search_order,
                                                 std::unordered_map<IndexKeyItem, std::vector<IndexValueItem>, IndexKeyItemHash> &global_index,
                                                 OfflineIndex *tc_offline_index, Arg *tc_arg) {

    if(value.self_key == _search_order.front()){

        one_result[value.self_key] = value;

        std::unordered_map<IndexKeyItem,std::vector<IndexValueItem>, IndexKeyItemHash> local_search_space;

        for(auto &nei_map:value.neighbor_link){

            auto nei_key = nei_map.first;
            auto &all_nei_pos = nei_map.second;

            std::unordered_map<std::string,std::vector<uit>> s_t_l_sl_tl_str_2_search_space_pos;

            bool is_this_nei_key_have_any_true_value = false;

            for(auto &nei_pos:all_nei_pos){

                IndexValueItem &nei_value = global_index.at(nei_key).at(nei_pos);

                if(!nei_value.status){
                    continue;
                }

                is_this_nei_key_have_any_true_value = true;

                std::string s_t_l_sl_tl_str = std::to_string(nei_value.v_source_id) + "_" + std::to_string(nei_value.v_target_id)+ "_" + std::to_string(nei_value.edge_label)+ "_" + std::to_string(nei_value.v_source_label)+ "_" + std::to_string(nei_value.v_target_label);


                if(s_t_l_sl_tl_str_2_search_space_pos.find(s_t_l_sl_tl_str) == s_t_l_sl_tl_str_2_search_space_pos.end()){
                    local_search_space[nei_key].emplace_back(nei_value);
                    s_t_l_sl_tl_str_2_search_space_pos[s_t_l_sl_tl_str].emplace_back(local_search_space[nei_key].size() - 1);
                } else{

                    bool is_any_merge = false;

                    for(auto &search_space_pos:s_t_l_sl_tl_str_2_search_space_pos.at(s_t_l_sl_tl_str)){

                        auto &existed_value = local_search_space.at(nei_key).at(search_space_pos);

                        auto finder = std::find(_search_order.begin(), _search_order.end(), value.self_key);
                        std::vector<IndexKeyItem> nei_all_succ_order(++finder,_search_order.end());

                        bool is_all_succ_order_equal = true;

                        for(auto &nei_succ_order:nei_all_succ_order){

                            if(existed_value.neighbor_link.find(nei_succ_order) != existed_value.neighbor_link.end() && nei_value.neighbor_link.find(nei_succ_order) != nei_value.neighbor_link.end()) {
                                if(existed_value.neighbor_link.at(nei_succ_order) != nei_value.neighbor_link.at(nei_succ_order)){
                                    is_all_succ_order_equal = false;
                                    break;
                                }
                            }
                        }

                        if(is_all_succ_order_equal){

                            is_any_merge = true;

                            existed_value.duplicated_edge_timestamp.emplace_back(nei_value.timestamp);
                            existed_value.duplicated_edge_timestamp.insert(existed_value.duplicated_edge_timestamp.end(),nei_value.duplicated_edge_timestamp.begin(),nei_value.duplicated_edge_timestamp.end());
                        }
                    }

                    if(!is_any_merge){
                        local_search_space[nei_key].emplace_back(nei_value);
                        s_t_l_sl_tl_str_2_search_space_pos[s_t_l_sl_tl_str].emplace_back(local_search_space[nei_key].size() - 1);
                    }

                }
            }


            if(!is_this_nei_key_have_any_true_value){
                return;
            }

        }



        IndexKeyItem* next_order = get_next_order(value.self_key, _search_order);

        for(auto &nei_value:local_search_space.at(*next_order)){
            recursive_search_with_dynamic_merge(nei_value, one_result, _search_order, global_index, tc_offline_index,
                                                tc_arg);
        }

    }else if(value.self_key == _search_order.back()) {
        IndexKeyItem* pre_order = get_pre_order(value.self_key, _search_order);

        if(tc_offline_index->nc_edge_to_pre.find(value.self_key) != tc_offline_index->nc_edge_to_pre.end()){
            for(auto &pre_key:tc_offline_index->nc_edge_to_pre[value.self_key]){
                if(pre_key == *pre_order) continue;
                if(one_result.find(pre_key) != one_result.end()){
                    if(one_result[pre_key].timestamp < value.timestamp){
                        continue;
                    } else{
                        one_result.erase(value.self_key);
                        return;
                    }
                }
            }
        }


        for(auto &it: tc_offline_index->nc_pair_vec){
            auto &nc_later_key = it.first;
            auto &nc_pre_key = it.second;
            if(value.self_key == nc_pre_key){

                if(one_result.find(nc_later_key) != one_result.end()){
                    if(one_result[nc_later_key].timestamp > value.timestamp){
                        continue;
                    } else{
                        one_result.erase(value.self_key);
                        return;
                    }
                }
            }
        }


        for(auto &nei_it:value.neighbor_link){
            auto &nei_key = nei_it.first;

            if(one_result.find(nei_key) != one_result.end()){


                if(is_existed_connection(value,one_result.at(nei_key))){
                    continue;
                } else{
                    one_result.erase(value.self_key);
                    return;
                }


            }

            else{
                one_result.erase(value.self_key);
                return;
            }
        }


        one_result[value.self_key] = value;

        this->union_match_count += 1;


        std::unordered_map<IndexKeyItem,std::vector<uit>,IndexKeyItemHash> key_to_time;

        for(auto &result_it:one_result){
            auto &result_key = result_it.first;
            auto &result_value = result_it.second;

            if(result_key == _search_order.front()){
                continue;
            }

            if(!result_value.duplicated_edge_timestamp.empty()){
                key_to_time[result_key] = result_value.duplicated_edge_timestamp;
            }
        }



        std::vector<std::unordered_map<IndexKeyItem, IndexValueItem, IndexKeyItemHash>> vec_one_result;


        vec_one_result.emplace_back(one_result);

        if(tc_arg->execute_mode == "enum"){


            for(auto &map_it:key_to_time){
                auto &key = map_it.first;
                auto &times = map_it.second;

                std::vector<std::unordered_map<IndexKeyItem, IndexValueItem, IndexKeyItemHash>> new_vec_one_result;

                for(auto result_it:vec_one_result){
                    for(auto &time_it:times){
                        result_it.at(key).timestamp = time_it;
                        new_vec_one_result.emplace_back(result_it);
                    }
                }

                for(auto &new_map_it:new_vec_one_result){
                    vec_one_result.emplace_back(new_map_it);
                }

            }

            std::vector<ResultEdge> result_vec;
            for(auto &one_result_it:vec_one_result){
                for(auto &it:one_result_it){
                    ResultEdge re;
                    re.q_id = tc_offline_index->q_edge_to_q_id.at(it.second.self_key);
                    re.source_id = it.second.v_source_id;
                    re.source_label = it.second.v_source_label;
                    re.target_id = it.second.v_target_id;
                    re.target_label = it.second.v_target_label;
                    re.edge_label = it.second.edge_label;
                    re.timestamp = it.second.timestamp;
                    result_vec.emplace_back(re);
                }

                this->match_result.emplace_back(result_vec);
                this->match_count += 1;

                result_vec.clear();
            }


        } else if(tc_arg->execute_mode == "count"){

            uit duplicated_result_count = 1;

            for(auto &map_it:key_to_time){
                auto &times = map_it.second;
                duplicated_result_count *= times.size() + 1;
            }

            this->match_count = this->match_count + duplicated_result_count;

        }


        one_result.erase(value.self_key);
        return;

    }else{
        IndexKeyItem* pre_order = get_pre_order(value.self_key, _search_order);
        IndexKeyItem* next_order = get_next_order(value.self_key, _search_order);


        if(tc_offline_index->nc_edge_to_pre.find(value.self_key) != tc_offline_index->nc_edge_to_pre.end()){
            for(auto &pre_key:tc_offline_index->nc_edge_to_pre[value.self_key]){


                if(one_result.find(pre_key) != one_result.end()){
                    if(one_result[pre_key].timestamp < value.timestamp){
                        continue;
                    } else{
                        one_result.erase(value.self_key);
                        return;
                    }
                }
            }
        }


        for(auto &it: tc_offline_index->nc_pair_vec){
            auto &nc_later_key = it.first;
            auto &nc_pre_key = it.second;
            if(value.self_key == nc_pre_key){

                if(one_result.find(nc_later_key) != one_result.end()){
                    if(one_result[nc_later_key].timestamp > value.timestamp){
                        continue;
                    } else{
                        one_result.erase(value.self_key);
                        return;
                    }
                }
            }
        }

        for(auto &nei_it:value.neighbor_link){
            auto &nei_key = nei_it.first;

            if(one_result.find(nei_key) != one_result.end()){

                if(is_existed_connection(value,one_result.at(nei_key))){
                    continue;
                } else{
                    one_result.erase(value.self_key);
                    return;
                }

            }
        }

        one_result[value.self_key] = value;

        //======================

        std::unordered_map<IndexKeyItem, std::vector<IndexValueItem>, IndexKeyItemHash> local_search_space;

        for(auto &nei_map:value.neighbor_link){

            auto nei_key = nei_map.first;
            auto &all_nei_pos = nei_map.second;

            std::unordered_map<std::string,std::vector<uit>> s_t_l_sl_tl_str_2_search_space_pos;

            bool is_this_nei_key_have_any_true_value = false;

            for(auto &nei_pos:all_nei_pos){

                IndexValueItem &nei_value = global_index.at(nei_key).at(nei_pos);

                if(!nei_value.status){
                    continue;
                }

                is_this_nei_key_have_any_true_value = true;

                std::string s_t_l_sl_tl_str = std::to_string(nei_value.v_source_id) + "_" + std::to_string(nei_value.v_target_id)+ "_" + std::to_string(nei_value.edge_label)+ "_" + std::to_string(nei_value.v_source_label)+ "_" + std::to_string(nei_value.v_target_label);


                if(s_t_l_sl_tl_str_2_search_space_pos.find(s_t_l_sl_tl_str) == s_t_l_sl_tl_str_2_search_space_pos.end()){
                    local_search_space[nei_key].emplace_back(nei_value);
                    s_t_l_sl_tl_str_2_search_space_pos[s_t_l_sl_tl_str].emplace_back(local_search_space[nei_key].size() - 1);
                } else{

                    bool is_any_merge = false;

                    for(auto &search_space_pos:s_t_l_sl_tl_str_2_search_space_pos.at(s_t_l_sl_tl_str)){
                        auto &existed_value = local_search_space.at(nei_key).at(search_space_pos);


                        auto finder = std::find(_search_order.begin(), _search_order.end(), value.self_key);
                        std::vector<IndexKeyItem> nei_all_succ_order(++finder,_search_order.end());

                        bool is_all_succ_order_equal = true;


                        for(auto &nei_succ_order:nei_all_succ_order){

                            if(existed_value.neighbor_link.find(nei_succ_order) != existed_value.neighbor_link.end() && nei_value.neighbor_link.find(nei_succ_order) != nei_value.neighbor_link.end()) {
                                if(existed_value.neighbor_link.at(nei_succ_order) != nei_value.neighbor_link.at(nei_succ_order)){
                                    is_all_succ_order_equal = false;
                                    break;
                                }
                            }
                        }


                        if(is_all_succ_order_equal){

                            is_any_merge = true;

                            existed_value.duplicated_edge_timestamp.emplace_back(nei_value.timestamp);
                            existed_value.duplicated_edge_timestamp.insert(existed_value.duplicated_edge_timestamp.end(),nei_value.duplicated_edge_timestamp.begin(),nei_value.duplicated_edge_timestamp.end());
                        }

                    }

                    if(!is_any_merge){
                        local_search_space[nei_key].emplace_back(nei_value);
                        s_t_l_sl_tl_str_2_search_space_pos[s_t_l_sl_tl_str].emplace_back(local_search_space[nei_key].size() - 1);
                    }

                }
            }

            if(!is_this_nei_key_have_any_true_value){
                one_result.erase(value.self_key);
                return;
            }

        }



        if(local_search_space.find(*next_order) != local_search_space.end()){
            if(!local_search_space.at(*next_order).empty()){
                for(auto &nei_value:local_search_space.at(*next_order)){
                    recursive_search_with_dynamic_merge(nei_value, one_result, _search_order, global_index,tc_offline_index, tc_arg);
                }
            }
        } else{
            for(auto &it:one_result){
                auto &existed_key = it.first;
                auto &existed_value = it.second;

                if(existed_key.qe.u_source_id == next_order->qe.u_source_id || existed_key.qe.u_source_id == next_order->qe.u_target_id || existed_key.qe.u_target_id == next_order->qe.u_source_id || existed_key.qe.u_target_id == next_order->qe.u_target_id){

                    if(existed_value.neighbor_link.find(*next_order) != existed_value.neighbor_link.end()){

                        for(auto &nei_pos:existed_value.neighbor_link.at(*next_order)){
                            auto &nei_value = global_index.at(*next_order).at(nei_pos);

                            if(nei_value.status){
                                recursive_search_with_dynamic_merge(nei_value, one_result, _search_order, global_index,tc_offline_index, tc_arg);
                            }
                        }

                        break;
                    }
                }
            }
        }


        one_result.erase(value.self_key);

    }

}




IndexKeyItem *Search::get_pre_order(const IndexKeyItem &key, std::vector<IndexKeyItem> &_search_order) {
    auto finder = std::find(_search_order.begin(), _search_order.end(), key);
    return &(*(--finder));
}



IndexKeyItem *Search::get_next_order(const IndexKeyItem &key, std::vector<IndexKeyItem> &_search_order) {
    auto finder = std::find(_search_order.begin(), _search_order.end(), key);
    return &(*(++finder));
}

bool Search::is_existed_connection(const IndexValueItem &value1, const IndexValueItem &value2) {

    if(value1.neighbor_link.find(value2.self_key) != value1.neighbor_link.end()){
        if(!value1.neighbor_link.at(value2.self_key).empty()){

            if(std::find(value1.neighbor_link.at(value2.self_key).begin(),
                         value1.neighbor_link.at(value2.self_key).end(),value2.self_pos) != value1.neighbor_link.at(value2.self_key).end()){
                return true;
            }
        }
    }
    return false;
}




