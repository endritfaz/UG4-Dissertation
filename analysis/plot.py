import json 
import matplotlib.pyplot as plt

with open("avg-frontier-az60000-edax21.json", "r") as f: 
    data = json.load(f)

avg_black = data["avg_frontier_per_move_black"]
avg_white = data["avg_frontier_per_move_white"]

x = range(61)

plt.plot(x, data["avg_frontier_per_move_black"], label="Black")
plt.plot(x, data["avg_frontier_per_move_white"], label="White")

plt.xlim(0, 60)
plt.legend()
plt.title("Average frontier per move (100 games), Black (AlphaZero 60k), White (Edax depth=21)")
plt.savefig("plot2.png", dpi=300, bbox_inches="tight")