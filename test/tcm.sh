#!/bin/sh

# ========setting start====================
current_path=$(pwd)
bin_path="./"

# general setting
data_percent="0.6"

# tcm setting
tcm_result_path=${current_path}/result-tcm/
result_mode="time"
execute_mode="count"
is_using_static_merge="y"


# if path not exist, mkdir
if [ ! -d "$tcm_result_path" ]; then
  mkdir "$tcm_result_path"
fi


# ========run start====================
# get stream file path
stream_file_full_path="${1}"

# get query file path
query_file_full_path="${2}"

# execute tcm
${bin_path}/tcm -s "${stream_file_full_path}" -q "${query_file_full_path}" -r "${tcm_result_path}" --rm "${result_mode}" --em "${execute_mode}" --ism "${is_using_static_merge}" --pct "${data_percent}"
