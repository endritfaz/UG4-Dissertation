import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from sqlalchemy import create_engine

engine = create_engine("postgresql+psycopg2://postgres:postgres@localhost:5432/ug4")

df = pd.read_sql("SELECT * FROM results ORDER BY iteration", engine)

def bytea_to_float(x):
    return np.frombuffer(b"".join(x), dtype=np.float64)[0]

for col in ["block_1_coeff", "block_2_coeff", "block_3_coeff"]:
    df[col] = df[col].apply(bytea_to_float)

concepts = df["concept"].unique()
fig, axes = plt.subplots(1, len(concepts), figsize=(6 * len(concepts), 4), squeeze=False)

for ax, concept in zip(axes[0], concepts):
    subset = df[df["concept"] == concept].copy()
    subset["iteration"] = pd.to_numeric(subset["iteration"], errors="coerce")
    subset = subset.sort_values("iteration")

    heatmap_data = subset[["block_1_coeff", "block_2_coeff", "block_3_coeff"]].T
    heatmap_data.index = ["Block 1", "Block 2", "Block 3"]
    heatmap_data.columns = subset["iteration"].astype(str).values

    sns.heatmap(heatmap_data, ax=ax, annot=True, fmt=".3f", cmap="viridis",
                vmin=0, vmax=1, cbar_kws={"label": "R² score"})
    ax.set_title(f"Concept: {concept}")
    ax.set_xlabel("Iteration")
    ax.set_ylabel("Block")

plt.tight_layout()
plt.savefig("results_heatmap.png", dpi=150)
plt.show()
