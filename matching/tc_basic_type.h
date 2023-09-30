#ifndef TCM_TC_BASIC_TYPE_H
#define TCM_TC_BASIC_TYPE_H

#include <iostream>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <chrono>
#include <iomanip>

//==============================
#define H_TIME std::chrono::high_resolution_clock::time_point
#define T_NOW std::chrono::high_resolution_clock::now()
#define T_SPAN std::chrono::duration<double, std::milli>

typedef std::uint32_t uit;



//========================================================================================

class SEdge{
public:
    uit v_source_id{},v_target_id{},edge_label{},v_source_label{},v_target_label{},timestamp{};

    SEdge(uit vSourceId, uit vTargetId, uit edgeLabel, uit vSourceLabel, uit vTargetLabel, uit timestamp) : v_source_id(
            vSourceId), v_target_id(vTargetId), edge_label(edgeLabel), v_source_label(vSourceLabel), v_target_label(
            vTargetLabel), timestamp(timestamp) {}
};

//========================================================================================

class QEdge{
public:
    uit u_source_id{},u_target_id{},u_edge_label{},u_source_label{},u_target_label{};


    QEdge() = default;

    QEdge(uit uSourceId, uit uTargetId, uit uEdgeLabel, uit uSourceLabel, uit uTargetLabel) : u_source_id(uSourceId),
                                                                                              u_target_id(uTargetId),
                                                                                              u_edge_label(uEdgeLabel),
                                                                                              u_source_label(
                                                                                                      uSourceLabel),
                                                                                              u_target_label(
                                                                                                      uTargetLabel) {}


    bool operator==(const QEdge& other) const {
        return u_source_id == other.u_source_id &&
               u_source_label == other.u_source_label &&
               u_target_id == other.u_target_id &&
               u_target_label == other.u_target_label &&
               u_edge_label == other.u_edge_label;
    }

    bool operator!=(const QEdge& other) const {
        return !(*this == other);
    }
};


//========================================================================================

class IndexKeyItem{
public:
    QEdge qe;

    IndexKeyItem() = default;

    IndexKeyItem(uit uSourceId, uit uTargetId, uit uEdgeLabel, uit uSourceLabel, uit uTargetLabel)
            : qe(uSourceId,uTargetId,uEdgeLabel,uSourceLabel,uTargetLabel){};


    bool operator==(const IndexKeyItem& other) const {
        return qe.u_source_id == other.qe.u_source_id &&
               qe.u_source_label == other.qe.u_source_label &&
               qe.u_target_id == other.qe.u_target_id &&
               qe.u_target_label == other.qe.u_target_label &&
               qe.u_edge_label == other.qe.u_edge_label;
    }

    bool operator!=(const IndexKeyItem& other) const {
        return !(*this == other);
    }
};


struct IndexKeyItemHash {
    std::size_t operator()(const IndexKeyItem& item) const {
        std::size_t h1 = std::hash<uit>{}(item.qe.u_source_id);
        std::size_t h2 = std::hash<uit>{}(item.qe.u_target_id);
        std::size_t h3 = std::hash<uit>{}(item.qe.u_edge_label);
        std::size_t h4 = std::hash<uit>{}(item.qe.u_target_label);
        std::size_t h5 = std::hash<uit>{}(item.qe.u_source_label);

        return (h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4));
    }
};


//========================================================================================

class IndexValueItem{
public:
    uit v_source_id{},v_target_id{},edge_label{},v_source_label{},v_target_label{},timestamp{};

    bool status = false;

    uit self_pos = 0;

    IndexKeyItem self_key;

    std::unordered_map<IndexKeyItem, std::vector<int>, IndexKeyItemHash> neighbor_link;

    std::vector<uit> duplicated_edge_timestamp;


    IndexValueItem() = default;

    IndexValueItem(uit source_id, uit target_id, uit edge_label, uit source_label, uit target_label, uit timestamp, IndexKeyItem &key, std::vector<IndexKeyItem> &all_nei_keys)
            :v_source_id(source_id),v_target_id(target_id),edge_label(edge_label),v_source_label(source_label),v_target_label(target_label),timestamp(timestamp)
    {
        self_key = key;

        for(auto &nei_it:all_nei_keys){
            if(this->neighbor_link.find(nei_it) == this->neighbor_link.end()){
                this->neighbor_link[nei_it];
            }
        }
    }


    void save_in_neighbor_link(IndexKeyItem &save_key, uit vec_position);

    void save_in_neighbor_link(IndexKeyItem &save_key);

    void update_self_status();


    bool operator==(const IndexValueItem& other) const {
        return v_source_id == other.v_source_id &&
               v_source_label == other.v_source_label &&
               v_target_id == other.v_target_id &&
               v_target_label == other.v_target_label &&
               edge_label == other.edge_label;
    }


    bool operator!=(const IndexValueItem& other) const {
        return !(*this == other);
    }
};


//========================================================================================

class Label{
public:
    uit source_label{},target_label{},edge_label{};

    Label() = default;

    Label(uit sourceLabel, uit targetLabel, uit edgeLabel) : source_label(sourceLabel),
                                                                     target_label(targetLabel),
                                                                     edge_label(edgeLabel) {}

    bool operator==(const Label &rhs) const {
        return source_label == rhs.source_label &&
               target_label == rhs.target_label &&
               edge_label == rhs.edge_label;
    }

    bool operator!=(const Label &rhs) const {
        return !(rhs == *this);
    }

};



struct LabelHash {
    std::size_t operator()(const Label& item) const {
        std::size_t h1 = std::hash<uit>{}(item.source_label);
        std::size_t h2 = std::hash<uit>{}(item.target_label);
        std::size_t h3 = std::hash<uit>{}(item.edge_label);

        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

//========================================================================================

class ResultEdge{
public:
    uit q_id{};
    uit source_id{};
    uit target_id{};
    uit edge_label{};
    uit source_label{};
    uit target_label{};
    uit timestamp{};
};


//============================================

#endif //TCM_TC_BASIC_TYPE_H
