#include <iostream>
#include <csignal>
#include <unistd.h>

#include "matching/tc_basic_type.h"
#include "matching/tc_arg.h"
#include "matching/tc_io.h"
#include "matching/tc_offline_index.h"
#include "matching/tc_match_order.h"
#include "matching/tc_global_index.h"
#include "matching/tc_search.h"
#include "matching/tc_misc.h"
#include "matching/tc_g_index.h"



//==Signal processing function==============================
void signal_handler_index(int signum) {
    std::cout << "tcm: out of index time" << std::endl;
    exit(1);
}

void signal_handler_online(int signum) {
    std::cout << "tcm: out of online time" << std::endl;
    exit(1);
}
//================================





int main(int argc, char * argv[]){

    auto* tc_arg = new Arg();
    auto* tc_io = new IO();
    auto* tc_offline_index = new OfflineIndex();
    auto* tc_order = new MatchOrder();
    auto* tc_global_index = new GlobalIndex();
    auto* tc_search = new Search();
    auto* tc_misc = new Misc();
    auto* tc_g_index = new GIndex();

    //============Parsing parameters====================================
    if(argc > 1){
        tc_arg->parse_arg(argc, argv);
    } else{   //Customizable parameters
        std::string dataset = "caida";
        std::string query_type = "dense4";
        std::string query_name = "15_2_3";
#ifdef _WIN32
        tc_arg->path_of_stream = R"(C:\Users\Oasis\Desktop\TimeCostCompare\dataset\)" + dataset + R"(\)" + dataset + R"(.txt)";
        tc_arg->path_of_query = R"(C:\Users\Oasis\Desktop\TimeCostCompare\dataset\)" + dataset + R"(\)" + query_type + R"(\)" + query_name + R"(.txt)";
#elif __linux__
        tc_arg->path_of_stream = R"(/mnt/c/Users/Oasis/Desktop/TimeCostCompare/dataset/)" + dataset + R"(/)" + dataset + R"(.txt)";
        tc_arg->path_of_query = R"(/mnt/c/Users/Oasis/Desktop/TimeCostCompare/dataset/)"+ dataset + R"(/)" + query_type + R"(/)" + query_name + R"(.txt)";
#endif

        tc_arg->path_of_result = R"(./)";
        tc_arg->result_mode = "qid";
        tc_arg->execute_mode = "enum";
        tc_arg->is_using_static_merge = "y";
        tc_arg->data_percent = "0.6";
        tc_arg->index_time_limit = "36000";
        tc_arg->online_time_limit = "3600";
        tc_arg->is_using_dynamic_merge = "y";

    }

    //=======================================================
    tc_arg->parse_dataset_name();

    auto now = std::chrono::system_clock::now();

    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

    std::tm* localTime = std::localtime(&currentTime);

    //============The program starts.====================================

    //Loading data
    tc_io->load_timing_query(tc_arg);
    tc_io->load_stream_2_data_graph_and_update_graph(tc_arg);

    std::cerr << "tcm: ";
    std::cerr << "dataset: " << tc_arg->streamFileNameWithoutExtension << " / ";
    std::cerr << "query type: " << tc_arg->queryType << " / ";
    std::cerr << "query: " << tc_arg->queryFileNameWithoutExtension << " / ";
    std::cerr << "runtime: " << std::asctime(localTime) << " / ";
    std::cerr << "update valid: " << tc_io->valid_update_edge_num << " / ";
    std::cerr << "total update: " << tc_io->total_update_edge_num << " / ";
    std::cerr << "valid percentage: " << ((double)tc_io->valid_update_edge_num / (double)tc_io->total_update_edge_num) * 100.0 << "% / ";

    //=====Starting the build phase.======
#ifdef __linux__
    signal(SIGALRM, signal_handler_index);
    alarm(std::stoi(tc_arg->index_time_limit));
#endif

    //build offline index
    tc_offline_index->build_offline_index(tc_io);

    //match order
    tc_order->build_search_order(tc_io, tc_offline_index);

    //===Starting the indexing phase.======================
    tc_global_index->index_RSS_size_start = tc_misc->get_current_RSS();
    tc_global_index->index_time_start = T_NOW;

    //==Initializing the global index.=========
    tc_global_index->index_process(tc_io, tc_offline_index, tc_arg, tc_g_index);

    tc_global_index->index_time_end = T_NOW;
    tc_global_index->index_time_span = tc_global_index->index_time_end - tc_global_index->index_time_start;

    tc_global_index->index_RSS_size_end = tc_misc->get_current_RSS();
    tc_global_index->index_RSS_size_span = tc_global_index->index_RSS_size_end - tc_global_index->index_RSS_size_start;

#ifdef __linux__
    alarm(0);
#endif

    std::cerr << "index time: "  << tc_global_index->index_time_span.count()  << " ms / ";
    std::cerr << "index memory: " << (tc_global_index->index_RSS_size_span / 1024) / 1024    << " MB / " << (tc_global_index->index_RSS_size_span / 1024) << " KB / " << (tc_global_index->index_RSS_size_span ) << " B / ";


    //=====Starting the online phase.======
    tc_global_index->online_RSS_size_start = tc_misc->get_current_RSS();
    tc_global_index->online_time_start = T_NOW;

#ifdef __linux__
    signal(SIGALRM, signal_handler_online);
    alarm(std::stoi(tc_arg->online_time_limit));
#endif

    tc_global_index->online_process(tc_offline_index, tc_search, tc_order, tc_arg, tc_g_index, tc_io);

#ifdef __linux__
    alarm(0);
#endif

    tc_global_index->online_time_end = T_NOW;
    tc_global_index->online_time_span = tc_global_index->online_time_end - tc_global_index->online_time_start;
    tc_global_index->online_RSS_size_end = tc_misc->get_current_RSS();
    tc_global_index->online_RSS_size_span = tc_global_index->online_RSS_size_end - tc_global_index->online_RSS_size_start;


    std::cerr << "online time: " << tc_global_index->online_time_span.count() << " ms / ";
    std::cerr << "update time: " << tc_global_index->online_time_span.count() - tc_global_index->search_time_span.count() << " ms / ";
    std::cerr << "search time: " << tc_global_index->search_time_span.count() << " ms / ";
    std::cerr << "data percent: "  << tc_arg->data_percent << " / ";
    std::cerr << "match count: " << tc_search->match_count << std::endl;
    std::cerr << std::endl;

    //=====Starting the print phase.======
    tc_misc->statistical_info << "=============== summary ===============" <<std::endl;
    tc_misc->statistical_info << "online time: " << tc_global_index->online_time_span.count() << " ms / "<< std::endl;
    tc_misc->statistical_info << "index time: "  << tc_global_index->index_time_span.count()  << " ms / "<< std::endl;
    tc_misc->statistical_info << "update time: " << tc_global_index->online_time_span.count() - tc_global_index->search_time_span.count() << " ms / "<< std::endl;
    tc_misc->statistical_info << "search time: " << tc_global_index->search_time_span.count() << " ms / "<< std::endl;
    {
        tc_misc->statistical_info << "connection time: " << tc_global_index->connection_time_span.count() << " / "<< std::endl;
        tc_misc->statistical_info << "search check time: " << tc_search->check_time_span.count() << " ms / "<< std::endl;
        tc_misc->statistical_info << "search enum time: " << tc_search->enum_time_span.count() << " ms / "<< std::endl;
    }
    tc_misc->statistical_info << "getMemoryUse: " << (tc_misc->getMemoryUse() / 1024)     << " MB / " << tc_misc->getMemoryUse() << " KB / " << std::endl;
    tc_misc->statistical_info << "index memory: " << (tc_global_index->index_RSS_size_span / 1024) / 1024    << " MB / " << (tc_global_index->index_RSS_size_span / 1024) << " KB / " << (tc_global_index->index_RSS_size_span ) << " B / "<< std::endl;
    tc_misc->statistical_info << "online memory: " << (tc_global_index->online_RSS_size_span / 1024) / 1024    << " MB / "<< std::endl;
    tc_misc->statistical_info << "data percent: "  << tc_arg->data_percent << " / "<< std::endl;
    {

        tc_misc->statistical_info << "duplicated edge count: " << tc_global_index->duplicated_edge_count << " / "<< std::endl;
        tc_misc->statistical_info << "because copy link count: " << tc_global_index->because_copy_link_count << " / "<< std::endl;
        tc_misc->statistical_info << "because succ edge count: " << tc_global_index->because_succ_edge_count << " / "<< std::endl;
        tc_misc->statistical_info << "data valid: " << tc_io->valid_data_edge_num << " / "<< std::endl;
        tc_misc->statistical_info << "total data: " << tc_io->total_data_edge_num << " / "<< std::endl;
        tc_misc->statistical_info << "update valid: " << tc_io->valid_update_edge_num << " / "<< std::endl;
        tc_misc->statistical_info << "total update: " << tc_io->total_update_edge_num << " / "<< std::endl;
        tc_misc->statistical_info << "valid percentage: " << ((double)tc_io->valid_update_edge_num / (double)tc_io->total_update_edge_num) * 100.0 << "% / "<< std::endl;
    }
    tc_misc->statistical_info << "match count: " << tc_search->match_count<< std::endl;

    std::cout << tc_misc->statistical_info.str() << std::endl;

    //============write result====================================
    tc_misc->write_result(tc_io, tc_arg, tc_global_index, tc_search, tc_order, tc_misc);

    //============Clearing the memory====================================
    delete tc_arg;
    delete tc_io;
    delete tc_offline_index;
    delete tc_order;
    delete tc_global_index;
    delete tc_search;
    delete tc_misc;


    return 0;
}