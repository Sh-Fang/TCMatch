#include "tc_arg.h"


int Arg::parse_arg(int argc, char **argv) {

    CLI::App app{"App description"};


    //====Parsing parameters============
    app.add_option("-s", path_of_stream, "stream graph path")->required();
    app.add_option("-q", path_of_query, "query graph path")->required();
    app.add_option("-r", path_of_result, "result file path [default=./]")->default_val("./");
    app.add_option("--em", execute_mode, "the mode of executing search [count/enum] [default=count]")->default_val("count");
    app.add_option("--ism", is_using_static_merge, "is consider using static merge [y/n] [default=y]")->default_val("y");
    app.add_option("--idm", is_using_dynamic_merge, "is consider duplication [y/n] [default=y]")->default_val("y");
    app.add_option("--pct", data_percent, "regard how much percentage stream edges as the data graph [default=0.6]")->default_val("0.6");
    app.add_option("--itl", index_time_limit, "max index time [default=36000]")->default_val("36000");
    app.add_option("--otl", online_time_limit, "max online time [default=3600]")->default_val("3600");
    app.add_option("--rm", result_mode, "the mode of showing result [time/qid] [default=time]")->default_val("time");



    CLI11_PARSE(app, argc, argv)

    return 0;
}




void Arg::parse_dataset_name() {

#ifdef _WIN32
    size_t q_pos = this->path_of_query.find_last_of('\\');
    size_t s_pos = this->path_of_stream.find_last_of('\\');
#elif __linux__
    size_t q_pos = this->path_of_query.find_last_of('/');
    size_t s_pos = this->path_of_stream.find_last_of('/');
#endif

    if (q_pos != std::string::npos) {
        queryFileName = this->path_of_query.substr(q_pos + 1);
    } else {
        queryFileName = this->path_of_query;
    }

    if (s_pos != std::string::npos) {
        streamFileName = this->path_of_stream.substr(s_pos + 1);
    } else {
        streamFileName = this->path_of_stream;
    }

    // Get query file name without extension.
    q_pos = queryFileName.find_last_of('.');
    if (q_pos != std::string::npos) {
        queryFileNameWithoutExtension = queryFileName.substr(0, q_pos);
    } else {
        queryFileNameWithoutExtension = queryFileName;
    }

    // Get stream file name without extension.
    s_pos = streamFileName.find_last_of('.');
    if (s_pos != std::string::npos) {
        streamFileNameWithoutExtension = streamFileName.substr(0, s_pos);
    } else {
        streamFileNameWithoutExtension = streamFileName;
    }

    //Get query type.
#ifdef _WIN32
    size_t last_pos = this->path_of_query.rfind('\\');
#elif __linux__
    size_t last_pos = this->path_of_query.rfind('/');
#endif

    if (last_pos != std::string::npos) {
        size_t second_last_pos = this->path_of_query.rfind('/', last_pos - 1);
        if (second_last_pos != std::string::npos) {
            auto first_substr = this->path_of_query.substr(second_last_pos + 1);

            size_t type_pos = first_substr.rfind('/');
            if (type_pos != std::string::npos) {
                queryType = first_substr.substr(0, type_pos);
            }
        }
    }
}
