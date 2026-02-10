#!/usr/bin/bash

primary_bot="az"
secondary_bot="edax"
edax_versions=("6" "15" "21")

for i in $(seq 0 1000 28000);
do
    for j in ${edax_versions[@]};
        do
            ./server $primary_bot $i $secondary_bot $j 10 0.5 true 5
        done
done
