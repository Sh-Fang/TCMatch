#!/bin/sh

# ========setting start====================
current_path=$(pwd)
bin_path="./"
# shellcheck disable=SC2034
compare_result_pre_path=${current_path}/compare_result_tcm/


# general setting
data_percent="0.6"

# tcm setting
tcm_result_path=${current_path}/result-tcm/
result_mode="time"
execute_mode="count"
is_using_static_merge="y"
is_using_dynamic_merge="y"
is_print_output="n"

# if path not exist, mkdir
if [ ! -d "$tcm_result_path" ]; then
  mkdir "$tcm_result_path"
fi

if [ ! -d "$compare_result_pre_path" ]; then
  mkdir "$compare_result_pre_path"
fi


# ========run start====================
# get stream file path
stream_file_full_path="/mnt/c/Users/Oasis/Desktop/new-dataset/${1}/${1}.txt"

# get query file path
query_file_folder_path="/mnt/c/Users/Oasis/Desktop/new-dataset/${1}/${2}"

compare_result_path="${compare_result_pre_path}compare_${1}_${2}.txt"


for file in "$query_file_folder_path"/*; do
    # shellcheck disable=SC2039
    if [ -f "$file" ]&& [[ "$file" == *.txt ]]; then
        query_file_full_path="$file"

        # shellcheck disable=SC2129
        echo "=========================================" >> "${compare_result_path}"
        echo "$stream_file_full_path" >> "${compare_result_path}"
        echo "$query_file_full_path" >> "${compare_result_path}"

        ${bin_path}/tcm -s "${stream_file_full_path}" -q "${query_file_full_path}" -r "${tcm_result_path}" --rm "${result_mode}" --em "${execute_mode}" --ism "${is_using_static_merge}" --idm "${is_using_dynamic_merge}" --pct "${data_percent}" --ipo "${is_print_output}"  2>&1 | tee -a "${compare_result_path}"

        echo "=========================================" >> "${compare_result_path}"
    fi
done

