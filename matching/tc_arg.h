#ifndef TCM_TC_ARG_H
#define TCM_TC_ARG_H

#include <iostream>
#include "../utils/CLI11.hpp"

class Arg{
public:


    std::string path_of_stream;
    std::string path_of_query;
    std::string path_of_result;
    std::string result_mode;   // [time]: Sorting the results by timestamp; [qid]: Sorting the results by qid
    std::string execute_mode;  //[count]: Only count the results; [enum]: Enumerate the results.
    std::string is_using_static_merge; //[y][n]
    std::string data_percent; //[0.6 default] 60% edge as data graph, 40% edge as update graph
    std::string index_time_limit;  //[36000 : 10 (hour)]
    std::string online_time_limit; //[3600 : 1 (hour)]
    std::string is_using_dynamic_merge; //[y][n]


    int parse_arg(int argc, char * argv[]);



    std::string queryFileName;
    std::string streamFileName;
    std::string queryType;
    std::string queryFileNameWithoutExtension;
    std::string streamFileNameWithoutExtension;


    void parse_dataset_name();


};

#endif //TCM_TC_ARG_H
