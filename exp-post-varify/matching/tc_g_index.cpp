#include "tc_g_index.h"

void GIndex::save_g_edge(IndexValueItem &value) {
    auto &source_id = value.v_source_id;
    auto &target_id = value.v_target_id;

    std::string us_ut_el_sl_tl = std::to_string(value.self_key.qe.u_source_id) + "_" + std::to_string(value.self_key.qe.u_target_id) + "_" + std::to_string(value.self_key.qe.u_edge_label) + "_" + std::to_string(value.self_key.qe.u_source_label) + "_" + std::to_string(value.self_key.qe.u_target_label);
    auto &pos = value.self_pos;

    g_index[source_id][us_ut_el_sl_tl]["s"].emplace_back(pos);
    g_index[target_id][us_ut_el_sl_tl]["t"].emplace_back(pos);
}
