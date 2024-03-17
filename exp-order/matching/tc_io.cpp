#include "tc_io.h"


void IO::load_stream_2_data_graph_and_update_graph(Arg *tc_arg) {

    std::string file_path = tc_arg->path_of_stream;

    double data_percent = std::stod(tc_arg->data_percent);

    std::ifstream infile;

    infile.open(file_path);

    if(!infile){
        std::cerr << "Failed To Load Graph" << std::endl;
        exit(-1);
    }

    std::string	line;

    while (getline(infile, line)) {
        if (line == "\r" || line.empty()) continue;

        uit source_id{}, target_id{},edge_label{},source_label{}, target_label{},timestamp{};

        std::stringstream(line) >> source_id >> target_id >> edge_label >> source_label >> target_label >> timestamp;

        if(source_id == target_id) continue;

        S.emplace_back(source_id, target_id, edge_label, source_label, target_label, timestamp);

    }

    infile.close();


    //==============
    uit data_num = S.size() * data_percent;
    int edge_count = 0;
    for(auto &it:S){
        if(edge_count++ < data_num){
            S_data.emplace_back(it);
        } else{
            S_update.emplace_back(it);
        }
    }

    //===Counting the valid edges within the update graph.============
    for(auto &edges:S_update){
        for(auto &q_it:this->Q){
            if(edges.edge_label == q_it.u_edge_label && edges.v_source_label == q_it.u_source_label && edges.v_target_label == q_it.u_target_label){
                this->valid_update_edge_num++;
                break;
            }
        }
    }

    for(auto &edges:S_data){
        for(auto &q_it:this->Q){
            if(edges.edge_label == q_it.u_edge_label && edges.v_source_label == q_it.u_source_label && edges.v_target_label == q_it.u_target_label){
                this->valid_data_edge_num++;
                break;
            }
        }
    }

    this->total_data_edge_num = S_data.size();
    this->total_update_edge_num = S_update.size();
    //=====================

}

void IO::load_timing_query(Arg *tc_arg) {

    std::string file_path = tc_arg->path_of_query;

    std::ifstream infile;

    infile.open(file_path);

    if(!infile){
        std::cerr << "Failed To Load Query" << std::endl;
        exit(-1);
    }

    std::string	line;

    while (getline(infile, line)) {
        if(line == "\r" || line.empty()) continue;

        char type = line[0];
        line = line.substr(2);

        if (type == 'e') {
            uit id{},source_id{}, target_id{},source_label{}, target_label{},edge_label{};

            std::stringstream(line) >> id >> source_id >> target_id >> edge_label >> source_label >> target_label;

            QEdge q_edge{source_id, target_id, edge_label, source_label, target_label};
            this->Q.emplace_back(q_edge);
            this->q_id_2_q_edge[id] = q_edge;
        }
        else if (type == 'b'){
            std::vector<QEdge> row_id_2_q_edge;
            uit edge_id;
            std::stringstream ss(line);

            while (ss >> edge_id){
                row_id_2_q_edge.emplace_back(this->q_id_2_q_edge[edge_id]);
            }

            this->T.emplace_back(row_id_2_q_edge);
            row_id_2_q_edge.clear();
        } else{
            continue;
        }
    }

    infile.close();
}
