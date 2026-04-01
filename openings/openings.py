import json 
import csv 
import sys 
import random 
import time
import random 

def reflect_row(move): 
    return "" + move[0] + str(9 - (ord(move[1]) - ord('0')))

def opening_to_list(opening):
    opening_list = []
    # Eight moves in an opening
    for i in range(8):
        opening_list.append(reflect_row(opening[i*2: i*2 + 2]))
    
    return opening_list


if __name__ == '__main__':
    n = int(sys.argv[1])

    all_openings_path = "/home/endret/UG4-Dissertation/openings/openingslarge.txt"

    raw_openings = [] 

    with open(all_openings_path) as f: 
        for opening in f: 
            raw_openings.append(opening.strip())

    
    list_openings = [(i, opening_to_list(opening)) for i, opening in enumerate(raw_openings)] 

    opening_sample = random.sample(list_openings, n)

    openings = [] 

    for (i, sample_opening) in opening_sample:
        opening = {} 
        opening["id"] = i
        opening["opening"] = sample_opening

        openings.append(opening)

    openings_json = {} 
    openings_json["openings"] = openings

    seed = time.time() 
    random.seed(seed)
    rand = random.random()
    filename = "openings" + str(rand).replace(".", "")
    
    json_dir = f"/home/endret/UG4-Dissertation/openings/{filename}.json"
 
    with open(json_dir, "w") as f:
        json.dump(openings_json, f, indent=4)

    print(json_dir)