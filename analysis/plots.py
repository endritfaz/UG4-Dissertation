import matplotlib.pyplot as plt
import pandas as pd
import psycopg2
import seaborn as sns
import shutil
from pathlib import Path
from sqlalchemy import create_engine
import numpy as np 
import shutil 

edax_versions = [6, 15, 21]
edax_plot_colours = ["green", "orange", "blue"]
colours = ["black", "white"]

def plot_forced_corner_capture(games, moves, save_dir, save_filename):
    no_resign = games[games["resign"] == False]

    df = moves.merge(
        no_resign[["game_id", "az_version", "az_black", "edax_version", "az_won"]],
        on="game_id"
    )

    df["az_active"] = np.where((((df["az_black"] == True) & (df["black_active"] == True)) | ((df["az_black"] == False) & (df["black_active"] == False))), True, False)

    df_az_active = df[df["az_active"] == True]
    df_az_inactive = df[df["az_active"] == False]

    per_game_az_active = (
    df_az_active.groupby(["game_id", "az_version", "edax_version", "az_black"])
        .agg(avg_az_fcp=("forced_corner_capture_executed", "mean"))
        .reset_index()
    )

    per_game_az_inactive = (
    df_az_inactive.groupby(["game_id", "az_version", "edax_version", "az_black"])
        .agg(avg_edax_fcp=("forced_corner_capture_executed", "mean"))
        .reset_index()
    )

    per_game = per_game_az_active.merge(per_game_az_inactive, on=["game_id", "az_version", "edax_version", "az_black"])

    black_data = per_game[per_game["az_black"] == True]
    white_data = per_game[per_game["az_black"] == False]

    # Create plots of AZ version against average forced corner capture rate 
    fig, ax = plt.subplots(2, 3, figsize=(18, 10), sharex=True)

    for i, colour in enumerate(colours):
        for j, edax_version in enumerate(edax_versions): 
            other_colour = "black" if colour == "white" else "white"

            data = black_data if colour == "black" else white_data 
            data = data[data["edax_version"] == edax_version]

            ax_current = ax[i][j]
            sns.lineplot(
                data=data,
                x="az_version",  
                y="avg_az_fcp",
                ax=ax[i][j],
                color=edax_plot_colours[j], 
                marker="h",
            )

            ax_current.set_xlabel("Training Iteration")
            ax_current.set_ylabel("Average forced corner capture rate per game")
            ax_current.set_title(f"AZ ({colour.capitalize()}) vs Edax depth {edax_version} ({other_colour.capitalize()})")

    fig.savefig(save_dir / save_filename, dpi=300, bbox_inches="tight")

def plot_forced_corner_capture_conditional(games, moves, save_dir, save_filename):
    df = moves.merge(
        games[["game_id", "az_version", "az_black", "edax_version", "az_won"]],
        on="game_id"
    )

    df["az_active"] = np.where((((df["az_black"] == True) & (df["black_active"] == True)) | ((df["az_black"] == False) & (df["black_active"] == False))), True, False)

    df_az_active = df[df["az_active"] == True]
    df_az_inactive = df[df["az_active"] == False]

    per_version = (
        df_az_active.groupby(["az_version", "edax_version", "az_black"]).agg(total_fcc_possibilities=("forced_corner_capture_possible", "sum"), total_fcc_executions=("forced_corner_capture_executed", "sum"))
        .reset_index()
    )

    per_version = per_version[per_version["total_fcc_possibilities"] > 0]
    per_version["ratio_fcc_taken"] = per_version["total_fcc_executions"] / per_version["total_fcc_possibilities"]


    black_data = per_version[per_version["az_black"] == True]
    white_data = per_version[per_version["az_black"] == False]
    
    # Create plots of AZ version against proportion of forced corner capture opportunities taken
    fig, ax = plt.subplots(2, 3, figsize=(18, 10), sharex=True)

    for i, colour in enumerate(colours):
        for j, edax_version in enumerate(edax_versions): 
            other_colour = "black" if colour == "white" else "white"

            data = black_data if colour == "black" else white_data 
            data = data[data["edax_version"] == edax_version]

            ax_current = ax[i][j]
            sns.lineplot(
                data=data,
                x="az_version",  
                y="ratio_fcc_taken",
                ax=ax[i][j],
                color=edax_plot_colours[j], 
                marker="h",
            )

            ax_current.set_ylim(0, 1.2)
            ax_current.set_xlabel("Training Iteration")
            ax_current.set_ylabel("Proportion of forced corner capture opportunities taken")
            ax_current.set_title(f"AZ ({colour.capitalize()}) vs Edax depth {edax_version} ({other_colour.capitalize()})")

    fig.savefig(save_dir / save_filename, dpi=300, bbox_inches="tight")
    

def plot_parity_training(games, moves, save_dir, save_filename):
    no_resign = games[games["resign"] == False]

    df = moves.merge(
        no_resign[["game_id", "az_version", "az_black", "edax_version", "az_won"]],
        on="game_id"
    )

    df["az_active"] = np.where((((df["az_black"] == True) & (df["black_active"] == True)) | ((df["az_black"] == False) & (df["black_active"] == False))), True, False)

    # For each game, aggregate average (same as proportion) of parity successes for both 
    df_az_active = df[df["az_active"] == True]
    df_az_inactive = df[df["az_active"] == False]

    per_game_az_active = (
    df_az_active.groupby(["game_id", "az_version", "edax_version", "az_black"])
        .agg(avg_az_parity_success=("parity", "mean"))
        .reset_index()
    )

    per_game_az_inactive = (
    df_az_inactive.groupby(["game_id", "az_version", "edax_version", "az_black"])
        .agg(avg_edax_parity_success=("parity", "mean"))
        .reset_index()
    )

    per_game = per_game_az_active.merge(per_game_az_inactive, on=["game_id", "az_version", "edax_version", "az_black"])

    black_data = per_game[per_game["az_black"] == True]
    white_data = per_game[per_game["az_black"] == False]

    # Create plots of AZ version against average parity success
    fig, ax = plt.subplots(2, 3, figsize=(18, 10), sharex=True)

    for i, colour in enumerate(colours):
        for j, edax_version in enumerate(edax_versions): 
            other_colour = "black" if colour == "white" else "white"

            data = black_data if colour == "black" else white_data 
            data = data[data["edax_version"] == edax_version]

            ax_current = ax[i][j]
            sns.lineplot(
                data=data,
                x="az_version",  
                y="avg_az_parity_success",
                ax=ax[i][j],
                color=edax_plot_colours[j], 
                marker="h",
                label="AZ parity success")

            sns.lineplot(
                data=data,
                x="az_version",  
                y="avg_edax_parity_success",
                ax=ax[i][j],
                color=edax_plot_colours[j], 
                marker="P",
                linestyle="--",
                label="Edax parity success")

            ax_current.axhline(0.5, color='firebrick')

            ax_current.set_xlabel("Training Iteration")
            ax_current.set_ylabel("Average proportion of parity successes")
            ax_current.set_title(f"AZ ({colour.capitalize()}) vs Edax depth {edax_version} ({other_colour.capitalize()})")

    fig.savefig(save_dir / save_filename, dpi=300, bbox_inches="tight")
    
def plot_frontier_training(games, moves, save_dir, save_filename, all_games):
    edax15_vs_edax21 = all_games[
    (all_games["black"] == "edax")
    & (all_games["white"] == "edax")
    & (all_games["white_version"].isin(["15", "21"]))
    & (all_games["black_version"].isin(["15", "21"]))
    & (all_games["white_version"] != all_games["black_version"])
    ]

    df = moves.merge(all_games[["game_id", "black_version", "white_version"]],on="game_id")
    
    print(df)
    # Filter for only games with no resign to make frontier average calculation fair. Resign games have less moves, so they only average frontiers over the early/mid game where other games also include the end game
    no_resign = games[games["resign"] == False]
    df = moves.merge(
        no_resign[["game_id", "az_version", "az_black", "edax_version", "az_won"]],
        on="game_id"
    )

    
    # Create columns for AZ/opponent frontier, and frontier difference from perspective of AZ
    df["az_frontier"] = np.where(df["az_black"], df["num_frontier_black"], df["num_frontier_white"])

    df["edax_frontier"] = np.where(df["az_black"], df["num_frontier_white"], df["num_frontier_black"])

    df["az_frontier_diff"] = df["az_frontier"] - df["edax_frontier"]

    # Average frontier and frontier diff by game 
    per_game = (
    df.groupby(["az_version", "az_black", "edax_version", "game_id"])
      .agg(avg_az_frontier=("az_frontier", "mean"), avg_edax_frontier= ("edax_frontier", "mean"), avg_frontier_diff=("az_frontier_diff", "mean"))
      .reset_index()
    )

    # Average these averages across AZ/edax version
    by_az_version = (
    per_game.groupby(["az_version", "az_black", "edax_version"])
            .agg(mean_az_frontier=("avg_az_frontier", "mean"),                  mean_edax_frontier=("avg_edax_frontier", "mean"), mean_frontier_diff=("avg_frontier_diff", "mean"),
            num_games=("game_id", "nunique"))
            .reset_index()
            .sort_values("az_version")
    )

    black_data = per_game[per_game["az_black"] == True]
    white_data = per_game[per_game["az_black"] == False]
    
    # Create plots of AZ version against frontier 
    fig, ax = plt.subplots(2, 3, figsize=(18, 10), sharex=True)

    for i, colour in enumerate(colours):
        for j, edax_version in enumerate(edax_versions): 
            other_colour = "black" if colour == "white" else "white"

            data = black_data if colour == "black" else white_data 
            data = data[data["edax_version"] == edax_version]

            ax_current = ax[i][j]
            sns.lineplot(
                data=data,
                x="az_version",  
                y="avg_frontier_diff",
                ax=ax[i][j],
                color=edax_plot_colours[j], 
                marker="h",)

            ax_current.set_ylim(-4, 8)
            ax_current.axhline(0, color='firebrick')

            ax_current.set_xlabel("Training Iteration")
            ax_current.set_ylabel("Mean Frontier difference (per game)")
            ax_current.set_title(f"AZ ({colour.capitalize()}) vs Edax depth {edax_version} ({other_colour.capitalize()})")

    # fig.savefig(save_dir / save_filename, dpi=300, bbox_inches="tight")

def plot_win_rate(games, moves, save_dir, save_filename, all_games):
    edax15_vs_edax21 = all_games[
    (all_games["black"] == "edax")
    & (all_games["white"] == "edax")
    & (all_games["white_version"].isin(["15", "21"]))
    & (all_games["black_version"].isin(["15", "21"]))
    & (all_games["white_version"] != all_games["black_version"])
    ]

    edax15_vs_edax21["edax15_won"] = (
    ((edax15_vs_edax21["black_version"] == "15") & (edax15_vs_edax21["winner"] == "black")) |
    ((edax15_vs_edax21["white_version"] == "15") & (edax15_vs_edax21["winner"] == "white"))).astype(int)

    edax15_winrate = edax15_vs_edax21["edax15_won"].sum() / len(edax15_vs_edax21); 

    # Filter for only games with no resign to make frontier average calculation fair. Resign games have less moves, so they only average frontiers over the early/mid game where other games also include the end game
    no_resign = games[games["resign"] == False]
    df = moves.merge(
        no_resign[["game_id", "az_version", "az_black", "edax_version", "az_won", "draw"]],
        on="game_id"
    )

    per_game = (
    df.groupby(["az_version", "az_black", "edax_version", "game_id"])
      .agg(avg_winrate=("az_won", "mean"))
      .reset_index()
    )

    data = per_game

    # Plot win rates 
    fig, ax = plt.subplots(1, 1, figsize=(18, 10))

    ax_current = ax
    sns.lineplot(
        data=data,
        x="az_version",  
        y="avg_winrate",
        hue="edax_version",
        ax=ax_current,
        palette={6: 'green', 15: 'orange', 21: 'blue'}, 
        marker="h",)

    ax_current.axhline(edax15_winrate, color='firebrick', linestyle='--', label="Edax 15 winrate against Edax 21")

    ax_current.set_xlabel("Training Iteration")
    ax_current.set_ylabel("AZ Winrate")
    ax_current.set_title("AZ winrate against Edax versions 6, 15, 21")
    plt.legend()
    plt.grid()

    fig.savefig(save_dir / save_filename, dpi=300, bbox_inches="tight")

def prepare_save_dir(output_dir):
    """
    Clear and recreate output_dir.
    """
    if output_dir.exists():
        shutil.rmtree(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)


base_save_dir = "game_data_plots"

engine = create_engine(
    "postgresql+psycopg2://postgres:postgres@localhost:5432/ug4"
)

games = pd.read_sql("SELECT * FROM games;", engine)
moves = pd.read_sql("SELECT * FROM moves;", engine)


az_games = games[(games["black"] == "az") | (games["white"] == "az")]

# Create edax and az version columns in games df to make further analysis easier
az_games["az_black"] = np.where(az_games["black"] == "az", True, False); 

az_games["az_version"] = np.where(az_games["az_black"], az_games["black_version"], az_games["white_version"]).astype(int)

az_games["az_won"] = np.where(((az_games["az_black"] == True) & (az_games["winner"] == "black")) | ((az_games["az_black"] == False) & (az_games["winner"] == "white")), True, False)

az_games["draw"] = np.where(az_games["winner"] == "draw", True, False); 

az_games["edax_version"] = np.where(az_games["az_black"], az_games["white_version"], az_games["black_version"]).astype(int)

# Create and save win rate plot
"""
win_rate_save_dir = Path(base_save_dir + "/win_rate")
prepare_save_dir(win_rate_save_dir)

plot_win_rate(az_games, moves, win_rate_save_dir, "win_rate.png", games)
"""

# Create and save frontier plot
frontier_save_dir = Path(base_save_dir + "/frontier")
prepare_save_dir(frontier_save_dir)

plot_frontier_training(az_games, moves, frontier_save_dir, "frontier_difference.png", games)

"""
# Create and save forced corner capture plots
fcc_save_dir = Path(base_save_dir + "/forced_corner_capture")
prepare_save_dir(fcc_save_dir)

plot_forced_corner_capture(az_games, moves, fcc_save_dir, "forced_corner_capture.png")

fcc_cond_save_dir = Path(base_save_dir + "/forced_corner_capture")

plot_forced_corner_capture_conditional(az_games, moves, fcc_cond_save_dir, "forced_corner_capture_conditional.png")

# Create and save parity plot 
parity_save_dir = Path(base_save_dir + "/parity")
prepare_save_dir(parity_save_dir)

plot_parity_training(az_games, moves, parity_save_dir, "parity.png")
"""


