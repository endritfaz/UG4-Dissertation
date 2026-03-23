#!/usr/bin/bash

start_iter=$1
end_iter=$2
table=$3
concept=$4

echo "Fitting models from iterations $start_iter to $end_iter"
for i in $(seq $start_iter 1000 $end_iter);
do
    echo "Iteration: $i"
    python3 /home/endret/UG4-Dissertation/probing/probing.py $i $table $concept 
done
