#ifndef TCM_TC_G_INDEX_H
#define TCM_TC_G_INDEX_H

#include "tc_basic_type.h"


class GIndex{
public:

    std::unordered_map<uit,std::unordered_map<std::string,std::unordered_map<std::string,std::vector<uit>>>> g_index;


    void save_g_edge(IndexValueItem &value);
};

#endif //TCM_TC_G_INDEX_H
