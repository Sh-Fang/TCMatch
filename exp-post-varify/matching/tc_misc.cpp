#include "tc_misc.h"
#include "../utils/mem.h"



void Misc::write_result(IO *tc_io, Arg *tc_arg, GlobalIndex *tc_global_index, Search *tc_search, MatchOrder *tc_order,
                        Misc *tc_misc) {


    std::ofstream outfile(tc_arg->path_of_result + "tcm_" + tc_arg->streamFileNameWithoutExtension + "_" + tc_arg->queryType + "_" + tc_arg->queryFileNameWithoutExtension + ".txt", std::ios::out);


    outfile << "====================== Summary ======================" << std::endl;

    auto now = std::chrono::system_clock::now();

    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

    std::tm* localTime = std::localtime(&currentTime);

    outfile << "Runtime: " << std::asctime(localTime) << std::endl;


    outfile << "Stream File: " << tc_arg->path_of_stream << std::endl;

    outfile << "Query File: " << tc_arg->path_of_query << std::endl;

    outfile << std::endl;

    outfile << "Total Q Edges: " << tc_io->Q.size() << std::endl;

    outfile << "Total S Edges: " << tc_io->total_update_edge_num << std::endl;

    outfile << "Valid Edge: " << tc_io->valid_update_edge_num << std::endl;

    long double percentage = 100 * (long double)tc_io->valid_update_edge_num / tc_io->total_update_edge_num;
    outfile << "Valid Edge Percentage (valid edge / total S edges): " << std::fixed << std::setprecision(6) << percentage << "%" << std::endl;

    outfile << std::endl;

    outfile << "Execute Mode: " << tc_arg->execute_mode << std::endl;

    outfile << "Is Consider Duplication: " << tc_arg->is_using_static_merge << std::endl;

    outfile << "Data Percentage: " << tc_arg->data_percent << std::endl;


    outfile << std::endl;

    outfile << "====================== Time&Space Cost ======================" << std::endl;

    outfile << tc_misc->statistical_info.str() << std::endl;

    outfile << std::endl;

    outfile << "====================== Query Edge ======================" << std::endl;

    outfile << "All Query: " << std::endl;


    for(auto &it:tc_io->q_id_2_q_edge){
        outfile << it.first << "-(" << it.second.u_source_id << "," << it.second.u_target_id << "," << it.second.u_edge_label << "," << it.second.u_source_label << "," << it.second.u_target_label << ") ";
    }

    outfile << std::endl;
    outfile << std::endl;

    outfile << "All Time Constrain: " << std::endl;

    for(auto &it:tc_io->T){
        for(auto &it2:it){
            if(it2 == it.back()){
                for(auto &it3:tc_io->q_id_2_q_edge){
                    if(it3.second == it2){
                        outfile << it3.first <<"-(" << it2.u_source_id << "," << it2.u_target_id << "," << it2.u_edge_label << "," << it2.u_source_label << "," << it2.u_target_label << ")";
                    }
                }
            } else{
                for(auto &it3:tc_io->q_id_2_q_edge){
                    if(it3.second == it2){
                        outfile << it3.first << "-(" << it2.u_source_id << "," << it2.u_target_id << "," << it2.u_edge_label << "," << it2.u_source_label << "," << it2.u_target_label << ") < ";
                    }
                }
            }
        }
        outfile << std::endl;
    }

    outfile << std::endl;

    outfile << "All Search Order [qid]: " << std::endl;

    for(auto &map_it:tc_order->search_order){
        for(auto &id2edge_it:tc_io->q_id_2_q_edge){
            auto &q_edge = id2edge_it.second;
            auto &q_id = id2edge_it.first;
            auto &key = map_it.first;
            if(q_edge == key.qe){
                outfile << q_id << ": ";
                break;
            }
        }

        for(auto &key_it:map_it.second){
            for(auto &id2edge_it:tc_io->q_id_2_q_edge){
                if(id2edge_it.second == key_it.qe){
                    outfile << id2edge_it.first << " ";
                }
            }
        }
        outfile << std::endl;
    }

    outfile << std::endl;

    outfile << "Total Matching Count: " << tc_search->match_count << std::endl;

    outfile << "Total Duplicated Edge Count: " << tc_global_index->duplicated_edge_count << std::endl;

    outfile << "Total Union Edge Count: " << tc_search->union_match_count << std::endl;

    outfile << std::endl;

    outfile << "====================== All Matching ======================" << std::endl;

    uit max_result_number = 0;

    if(tc_arg->result_mode == "qid"){
        for (const auto &sub_vec : tc_search->match_result) {
            for(auto &it:tc_io->q_id_2_q_edge){
                uit q_id = it.first;
                for (const auto &elem : sub_vec) {
                    if(q_id == elem.q_id){
                        outfile << "" << elem.source_id  << " " << elem.target_id << " " << elem.edge_label << " " << elem.source_label << " " << elem.target_label << " " << elem.timestamp <<"  ";
                        break;
                    }
                }
            }
            outfile << std::endl;
            max_result_number += 1;

            if(max_result_number > 1000000){  //Avoiding excessively large match.result files.
                break;
            }
        }
    } else if (tc_arg->result_mode == "time"){
        std::vector<uit> time_stamp_all;
        for (const auto &sub_vec : tc_search->match_result) {
            for(auto &elem:sub_vec){
                time_stamp_all.emplace_back(elem.timestamp);
            }

            std::sort(time_stamp_all.begin(), time_stamp_all.end());

            for(auto &time_it:time_stamp_all){
                for(auto &elem:sub_vec){
                    if(elem.timestamp == time_it){
                        outfile << "(" << elem.source_id  << " " << elem.target_id << " " << elem.edge_label << " " << elem.source_label << " " << elem.target_label << " " << elem.timestamp <<") ";
                        break;
                    }
                }
            }

            outfile << std::endl;

            time_stamp_all.clear();
        }
    } else{
        for (const auto &sub_vec : tc_search->match_result) {
            for(auto &elem:sub_vec){
                outfile << "(" << elem.source_id  << " " << elem.target_id << " " << elem.edge_label << " " << elem.source_label << " " << elem.target_label << " " << elem.timestamp <<") ";
            }
            outfile << std::endl;
        }
    }


    outfile.close();
}


size_t Misc::get_current_RSS() {
    return getCurrentRSS();
}

long Misc::getMemoryUse() {
    int who = RUSAGE_SELF;
    struct rusage usage{};
    getrusage(who, &usage);
    return usage.ru_maxrss;
}


