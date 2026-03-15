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

def separate_edax21_games(all_games):
    edax21_black = {}; 
    edax21_white = {}; 

    for edax_version in edax_versions:
        edax21_black[str(edax_version)] = all_games[
        (all_games["black"] == "edax")
        & (all_games["white"] == "edax")
        & ((all_games["white_version"] == str(edax_version)) & (all_games["black_version"] == "21")) 
        ]

        edax21_white[str(edax_version)] = all_games[
        (all_games["black"] == "edax")
        & (all_games["white"] == "edax")
        & ((all_games["white_version"] == "21") & (all_games["black_version"] == str(edax_version)))
        ]

    edax21_by_colour_version = {}; 

    edax21_by_colour_version["black"] = edax21_black
    edax21_by_colour_version["white"] = edax21_white

    return edax21_by_colour_version 

def separate_edax21_moves(all_games, all_moves):
    moves = {}
    moves_black = {} 
    moves_white = {}

    edax21_by_colour_version = separate_edax21_games(all_games)

    for edax_version in edax_versions: 
        moves_black[str(edax_version)] = all_moves.merge(edax21_by_colour_version["black"][str(edax_version)], on="game_id")

        moves_white[str(edax_version)] = all_moves.merge(edax21_by_colour_version["white"][str(edax_version)], on="game_id")


    moves["black"] = moves_black; 
    moves["white"] = moves_white; 

    return moves

def plot_forced_corner_capture(games, moves, save_dir, save_filename, all_games):
    edax21_fcc_black = {}
    edax21_fcc_white = {}
    edax15_fcc = {}
    
    edax21_moves = separate_edax21_moves(all_games, moves)

    for edax_version in edax_versions: 
        black_moves = edax21_moves["black"][str(edax_version)]
        white_moves = edax21_moves["white"][str(edax_version)]

        black_active = black_moves[black_moves["black_active"] == True]
        white_active = white_moves[white_moves["black_active"] == False]
        edax_version_black_active = white_moves[white_moves["black_active"] == True]
        edax_version_white_active = black_moves[black_moves["black_active"] == False]

        per_game_black = (
            black_active.groupby(["black_version", "white_version", "game_id"])
            .agg(avg_black_fcc=("forced_corner_capture_executed", "mean"))
        .reset_index())

        per_game_white = (
            white_active.groupby(["black_version", "white_version", "game_id"])
            .agg(avg_white_fcc=("forced_corner_capture_executed", "mean"))
        .reset_index())

        per_game_edax_version_black = (
            edax_version_black_active.groupby(["black_version", "white_version", "game_id"])
            .agg(avg_black_fcc=("forced_corner_capture_executed", "mean"))
        .reset_index())

        per_game_edax_version_white = (
            edax_version_white_active.groupby(["black_version", "white_version", "game_id"])
            .agg(avg_white_fcc=("forced_corner_capture_executed", "mean"))
        .reset_index())

        edax21_fcc_black[str(edax_version)] = per_game_black["avg_black_fcc"].mean()

        edax21_fcc_white[str(edax_version)] = per_game_white["avg_white_fcc"].mean()

        if (edax_version == 15):
            edax15_fcc["black"] = per_game_edax_version_black["avg_black_fcc"].mean()
            edax15_fcc["white"] = per_game_edax_version_white["avg_white_fcc"].mean()

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

            edax_data = edax21_fcc_black if colour == "black" else edax21_fcc_white

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

            ax_current.axhline(edax_data[str(edax_version)], color="blue", linestyle='--', label="Edax 21 average forced corner capture")

            if (edax_version == 21):
                ax_current.axhline(edax15_fcc[colour], color="orange", linestyle='--', label="Edax 15 average forced corner capture vs Edax 21")

            ax_current.legend()

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
    

def plot_parity_training(games, moves, save_dir, save_filename, all_games):
    edax21_parity_black = {}
    edax21_parity_white = {}
    edax15_parity = {}
    
    moves = moves[moves["ply"] > 30]

    edax21_moves = separate_edax21_moves(all_games, moves)

    for edax_version in edax_versions: 
        black_moves = edax21_moves["black"][str(edax_version)]
        white_moves = edax21_moves["white"][str(edax_version)]

        black_active = black_moves[black_moves["black_active"] == True]
        white_active = white_moves[white_moves["black_active"] == False]
        edax_version_black_active = white_moves[white_moves["black_active"] == True]
        edax_version_white_active = black_moves[black_moves["black_active"] == False]

        per_game_black = (
            black_active.groupby(["black_version", "white_version", "game_id"])
            .agg(avg_black_parity=("parity", "mean"))
        .reset_index())

        per_game_white = (
            white_active.groupby(["black_version", "white_version", "game_id"])
            .agg(avg_white_parity=("parity", "mean"))
        .reset_index())

        per_game_edax_version_black = (
            edax_version_black_active.groupby(["black_version", "white_version", "game_id"])
            .agg(avg_black_parity=("parity", "mean"))
        .reset_index())

        per_game_edax_version_white = (
            edax_version_white_active.groupby(["black_version", "white_version", "game_id"])
            .agg(avg_white_parity=("parity", "mean"))
        .reset_index())

        edax21_parity_black[str(edax_version)] = per_game_black["avg_black_parity"].mean()

        edax21_parity_white[str(edax_version)] = per_game_white["avg_white_parity"].mean()

        if (edax_version == 15):
            edax15_parity["black"] = per_game_edax_version_black["avg_black_parity"].mean()
            edax15_parity["white"] = per_game_edax_version_white["avg_white_parity"].mean()

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

            edax_data = edax21_parity_black if colour == "black" else edax21_parity_white; 

            ax_current = ax[i][j]
            sns.lineplot(
                data=data,
                x="az_version",  
                y="avg_az_parity_success",
                ax=ax[i][j],
                color=edax_plot_colours[j], 
                marker="h",
                label="AZ parity success")
            """
            sns.lineplot(
                data=data,
                x="az_version",  
                y="avg_edax_parity_success",
                ax=ax[i][j],
                color=edax_plot_colours[j], 
                marker="P",
                linestyle="--",
                label="Edax parity success")
            """

            ax_current.axhline(0.5, color='firebrick')

            ax_current.axhline(edax_data[str(edax_version)], color="blue", linestyle='--', label="Edax 21 average parity")

            if (edax_version == 21):
                ax_current.axhline(edax15_parity[colour], color="orange", linestyle='--', label="Edax 15 average parity vs Edax 21")

            ax_current.set_xlabel("Training Iteration")
            ax_current.set_ylabel("Average proportion of parity successes")
            ax_current.set_title(f"AZ ({colour.capitalize()}) vs Edax depth {edax_version} ({other_colour.capitalize()})")
            ax_current.legend()

    fig.savefig(save_dir / save_filename, dpi=300, bbox_inches="tight")
    
def plot_frontier_training(games, moves, save_dir, save_filename, all_games):
    edax21_frontier_black = {}
    edax21_frontier_white = {}
    edax15_frontier = {}
    
    edax21_moves = separate_edax21_moves(all_games, moves)

    for edax_version in edax_versions: 
        black_moves = edax21_moves["black"][str(edax_version)]
        white_moves = edax21_moves["white"][str(edax_version)]

        black_moves["black_frontier_diff"] = black_moves["num_frontier_black"] - black_moves["num_frontier_white"]

        white_moves["white_frontier_diff"] = white_moves["num_frontier_white"] - white_moves["num_frontier_black"]
        white_moves["black_frontier_diff"] = white_moves["num_frontier_black"] - white_moves["num_frontier_white"]
        black_moves["white_frontier_diff"] = black_moves["num_frontier_white"] - black_moves["num_frontier_black"]

        per_game_black = (
            black_moves.groupby(["black_version", "white_version", "game_id"])
            .agg(avg_black_frontier_diff=("black_frontier_diff", "mean"))
        .reset_index())

        per_game_white = (
            white_moves.groupby(["black_version", "white_version", "game_id"])
            .agg(avg_white_frontier_diff=("white_frontier_diff", "mean"))
        .reset_index())

        per_game_edax_version_black = (
            white_moves.groupby(["black_version", "white_version", "game_id"])
            .agg(avg_black_frontier_diff=("black_frontier_diff", "mean"))
        .reset_index())

        per_game_edax_version_white = (
            black_moves.groupby(["black_version", "white_version", "game_id"])
            .agg(avg_white_frontier_diff=("white_frontier_diff", "mean"))
        .reset_index())

        edax21_frontier_black[str(edax_version)] = per_game_black["avg_black_frontier_diff"].mean()

        edax21_frontier_white[str(edax_version)] = per_game_white["avg_white_frontier_diff"].mean()

        if (edax_version == 15):
            edax15_frontier["black"] = per_game_edax_version_black["avg_black_frontier_diff"].mean()
            edax15_frontier["white"] = per_game_edax_version_white["avg_white_frontier_diff"].mean()

    df = moves.merge(all_games[["game_id", "black_version", "white_version"]],on="game_id")
    
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

    black_data = per_game[per_game["az_black"] == True]
    white_data = per_game[per_game["az_black"] == False]
    
    # Create plots of AZ version against frontier 
    fig, ax = plt.subplots(2, 3, figsize=(18, 10), sharex=True)

    for i, colour in enumerate(colours):
        for j, edax_version in enumerate(edax_versions): 
            other_colour = "black" if colour == "white" else "white"

            data = black_data if colour == "black" else white_data 
            data = data[data["edax_version"] == edax_version]

            edax_data = edax21_frontier_black if colour == "black" else edax21_frontier_white; 

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

            ax_current.axhline(edax_data[str(edax_version)], color="blue", linestyle='--', label="Edax 21 average frontier count")

            if (edax_version == 21):
                ax_current.axhline(edax15_frontier[colour], color="orange", linestyle='--', label="Edax 15 average frontier difference vs Edax 21")

            ax_current.set_xlabel("Training Iteration")
            ax_current.set_ylabel("Mean Frontier difference (per game)")
            ax_current.set_title(f"AZ ({colour.capitalize()}) vs Edax depth {edax_version} ({other_colour.capitalize()})")
            ax_current.legend()

    fig.savefig(save_dir / save_filename, dpi=300, bbox_inches="tight")

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

def plot_stable(games, moves, save_dir, save_filename, all_games):
    # Calculate average game stability for edax 21 and against edax 6, 15, and 21 and plot these too
    edax21_stability_black = {}
    edax21_stability_white = {}
    
    edax15_stability = {}

    edax21_moves = separate_edax21_moves(all_games, moves)

    for edax_version in edax_versions: 
        black_moves = edax21_moves["black"][str(edax_version)]
        white_moves = edax21_moves["white"][str(edax_version)]

        per_game_black = (
            black_moves.groupby(["black_version", "white_version", "game_id"])
            .agg(avg_black_stable=("num_stable_black", "mean"), avg_white_stable= ("num_stable_white", "mean"))
        .reset_index())

        per_game_white = (
            white_moves.groupby(["black_version", "white_version", "game_id"])
            .agg(avg_black_stable=("num_stable_black", "mean"), avg_white_stable= ("num_stable_white", "mean"))
        .reset_index())

        edax21_stability_black[str(edax_version)] = per_game_black["avg_black_stable"].mean()

        edax21_stability_white[str(edax_version)] = per_game_white["avg_white_stable"].mean()

        if (edax_version == 15):
            edax15_stability["black"] = per_game_white["avg_black_stable"].mean()
            edax15_stability["white"] = per_game_black["avg_white_stable"].mean()
      
    no_resign = games[games["resign"] == False]
    df = moves.merge(
        no_resign[["game_id", "az_version", "az_black", "edax_version", "az_won", "draw"]],
        on="game_id"
    )

    df["az_stable"] = np.where(df["az_black"], df["num_stable_black"], df["num_stable_white"])

    df["edax_stable"] = np.where(df["az_black"], df["num_stable_white"], df["num_stable_black"])

    per_game = (
    df.groupby(["az_version", "az_black", "edax_version", "game_id"])
      .agg(avg_az_stable=("az_stable", "mean"), avg_edax_stable= ("edax_stable", "mean"))
      .reset_index()
    )

    black_data = per_game[per_game["az_black"] == True]
    white_data = per_game[per_game["az_black"] == False]

    fig, ax = plt.subplots(2, 3, figsize=(18, 10), sharex=True)

    for i, colour in enumerate(colours):
        for j, edax_version in enumerate(edax_versions): 
            other_colour = "black" if colour == "white" else "white"

            data = black_data if colour == "black" else white_data 
            data = data[data["edax_version"] == edax_version]

            edax_data = edax21_stability_black if colour == "black" else edax21_stability_white; 

            ax_current = ax[i][j]
            sns.lineplot(
                data=data,
                x="az_version",  
                y="avg_az_stable",
                ax=ax[i][j],
                color=edax_plot_colours[j], 
                marker="h",)

            ax_current.axhline(edax_data[str(edax_version)], color="blue", linestyle='--', label="Edax 21 average stable disc count vs Edax 21")

            ax_current.set_xlabel("Training Iteration")
            ax_current.set_ylabel("Mean AZ stable disc (per game)")
            ax_current.set_title(f"AZ ({colour.capitalize()}) vs Edax depth {edax_version} ({other_colour.capitalize()})")
            ax_current.set_ylim(0, 8)
            
            if (edax_version == 21):
                ax_current.axhline(edax15_stability[colour], color="orange", linestyle='--', label="Edax 15 average stable disc count vs Edax 21")

            ax_current.legend()
            
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


az_games = games[((games["black"] == "az") & (games["white"] == "edax")) | (((games["white"] == "az") & (games["black"] == "edax")))]

# Create edax and az version columns in games df to make further analysis easier
az_games["az_black"] = np.where(az_games["black"] == "az", True, False); 

az_games["az_version"] = np.where(az_games["az_black"], az_games["black_version"], az_games["white_version"]).astype(int)

az_games["az_won"] = np.where(((az_games["az_black"] == True) & (az_games["winner"] == "black")) | ((az_games["az_black"] == False) & (az_games["winner"] == "white")), True, False)

az_games["draw"] = np.where(az_games["winner"] == "draw", True, False); 

az_games["edax_version"] = np.where(az_games["az_black"], az_games["white_version"], az_games["black_version"]).astype(int)

# Create and save stable disc plot 

stable_disc_save_dir = Path(base_save_dir + "/stable")
prepare_save_dir(stable_disc_save_dir)

plot_stable(az_games, moves, stable_disc_save_dir, "stable.png", games)


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


# Create and save forced corner capture plots
fcc_save_dir = Path(base_save_dir + "/forced_corner_capture")
prepare_save_dir(fcc_save_dir)

plot_forced_corner_capture(az_games, moves, fcc_save_dir, "forced_corner_capture.png", games)


fcc_cond_save_dir = Path(base_save_dir + "/forced_corner_capture")

plot_forced_corner_capture_conditional(az_games, moves, fcc_cond_save_dir, "forced_corner_capture_conditional.png")


# Create and save parity plot 
parity_save_dir = Path(base_save_dir + "/parity")
prepare_save_dir(parity_save_dir)

plot_parity_training(az_games, moves, parity_save_dir, "parity.png", games)

