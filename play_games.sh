#!/usr/bin/bash

primary_bot="az"
secondary_bot="edax"
edax_versions=("6" "15" "21")
opening_dir=$(python3 openings/openings.py 10)

for i in $(seq 0 1000 43000);
do
    for j in ${edax_versions[@]};
        do
            ./server $primary_bot $i $secondary_bot $j 10 0.5 true 10 true $opening_dir
        done
done

