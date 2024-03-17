#!/bin/sh

# dataset_list=("wikitalk" "caida" "livejournal" "lsbench")
# shellcheck disable=SC2039
dataset_list=("wikitalk")


query_type_list=("sparse" "dense")


for dataset in "${dataset_list[@]}"; do
  for query_type in "${query_type_list[@]}"; do
    nohup bash ./tcm.sh "${dataset}" "${query_type}" >> log_tcm.txt 2>&1 &
  done
done