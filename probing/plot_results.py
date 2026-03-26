import sys
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from sqlalchemy import create_engine

engine = create_engine("postgresql+psycopg2://postgres:postgres@localhost:5432/ug4")

concept = sys.argv[1] if len(sys.argv) > 1 else None

query = "SELECT * FROM results ORDER BY iteration"
if concept:
    query = f"SELECT * FROM results WHERE concept='{concept}' ORDER BY iteration"

df = pd.read_sql(query, engine)

def bytea_to_float(x):
    return np.frombuffer(b"".join(x), dtype=np.float64)[0]

coeff_cols = ["block_1_coeff", "block_2_coeff", "block_3_coeff",
              "block_1_control_coeff", "block_2_control_coeff", "block_3_control_coeff"]

for col in coeff_cols:
    df[col] = df[col].apply(bytea_to_float)


subset = df[df["concept"] == concept].copy()
subset["iteration"] = pd.to_numeric(subset["iteration"], errors="coerce")
subset = subset.sort_values("iteration")
subset = subset[(subset["iteration"] <= 10000) & (subset["iteration"] % 1000 == 0)]
iterations = subset["iteration"].astype(str).values
blocks = ["1", "2", "3"]

score_data = subset[["block_1_coeff", "block_2_coeff", "block_3_coeff"]].T
score_data.index = blocks
score_data.columns = iterations

control_data = subset[["block_1_control_coeff", "block_2_control_coeff", "block_3_control_coeff"]].T
control_data.index = blocks
control_data.columns = iterations

diff_data = score_data.values - control_data.values
diff_df = pd.DataFrame(diff_data, index=blocks, columns=iterations)

fig, axes = plt.subplots(1, 3, figsize=(16, 3))
fig.suptitle(f"Concept: {concept}", fontsize=12)

annot_kws = {"size": 8}
sns.heatmap(score_data, ax=axes[0], annot=True, fmt=".2f",
            vmin=0, vmax=1, cbar_kws={"label": "R² score"}, annot_kws=annot_kws)
axes[0].set_title("Task scores")
axes[0].set_xlabel("Iteration")
axes[0].set_ylabel("Block")

sns.heatmap(control_data, ax=axes[1], annot=True, fmt=".2f",
            vmin=0, vmax=1, cbar_kws={"label": "R² score"}, annot_kws=annot_kws)
axes[1].set_title("Control scores")
axes[1].set_xlabel("Iteration")

sns.heatmap(diff_df, ax=axes[2], annot=True, fmt=".2f", vmin=-1, vmax=1,
            cbar_kws={"label": "task - control"}, annot_kws=annot_kws)
axes[2].set_title("Selectivity scores")
axes[2].set_xlabel("Iteration")


plt.tight_layout()
filename = f"results_{concept}.png"
plt.savefig(filename, dpi=150)
print(f"Saved {filename}")
plt.show()
