#!/usr/bin/bash

primary_bot="az"
secondary_bot="edax"
edax_versions=("6" "15" "21")

for i in $(seq 0 1000 43000);
do
    for j in ${edax_versions[@]};
        do
            ./server $primary_bot $i $secondary_bot $j 25 0.5 true 5
        done
done

# /server edax 21 edax 21 200 0.5 true 40