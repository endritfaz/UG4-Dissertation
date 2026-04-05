#!/usr/bin/bash

primary_bot="az"
secondary_bot="edax"
edax_versions=("6" "15" "21")
# opening_dir=$(python3 openings/openings.py 10)
opening_dir=/home/endret/UG4-Dissertation/openings/openings08018498750274616.json

for i in $(seq 0 1000 43000);
do
    for j in ${edax_versions[@]};
        do
            ./server $primary_bot $i $secondary_bot $j 5 0 true 5 false $opening_dir
        done
done