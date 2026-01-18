import json 
import matplotlib.pyplot as plt

with open("avg.json", "r") as f: 
    data = json.load(f)

avg_black = data["avg_discs_per_move_black"]
avg_white = data["avg_discs_per_move_white"]

x = range(61)

plt.plot(x, data["avg_discs_per_move_black"], label="Black")
plt.plot(x, data["avg_discs_per_move_white"], label="White")

plt.xlim(0, 60)
plt.legend()
plt.savefig("plot5.png", dpi=300, bbox_inches="tight")