import matplotlib.pyplot as plt
import pandas as pd
import psycopg2
import seaborn as sns
from pathlib import Path
from sqlalchemy import create_engine
import numpy as np 
import shutil 
import scipy.stats as stats

plt.rcParams.update({"axes.titlesize": 13, "axes.labelsize": 12})

edax_versions = [6, 15, 21]
edax_plot_colours = ["green", "orange", "blue"]
colours = ["black", "white"]

def bootstrap_stats(data):
    res_stats = {} 

    mean = data.mean()
    res = stats.bootstrap((data,), np.mean, confidence_level=0.95, n_resamples=4000, method='percentile')
    lower, upper = res.confidence_interval

    res_stats["mean"] = mean
    res_stats["upper"] = upper
    res_stats["lower"] = lower

    return res_stats

def az_version_stats(df, col):
    result = {}
    for (az_version, az_black, edax_version), group in df.groupby(["az_version", "az_black", "edax_version"]):
        result[(az_version, az_black, edax_version)] = bootstrap_stats(group[col])
    return result

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

        edax21_fcc_black[str(edax_version)] = bootstrap_stats(black_active["forced_corner_capture_executed"])

        edax21_fcc_white[str(edax_version)] = bootstrap_stats(white_active["forced_corner_capture_executed"])

        if (edax_version == 15):
            edax15_fcc["black"] = bootstrap_stats(edax_version_black_active["forced_corner_capture_executed"])
            edax15_fcc["white"] = bootstrap_stats(edax_version_white_active["forced_corner_capture_executed"])

    no_resign = games[games["resign"] == False]

    df = moves.merge(
        no_resign[["game_id", "az_version", "az_black", "edax_version", "az_won"]],
        on="game_id"
    )

    df["az_active"] = np.where((((df["az_black"] == True) & (df["black_active"] == True)) | ((df["az_black"] == False) & (df["black_active"] == False))), True, False)

    df_az_active = df[df["az_active"] == True]
    df_az_inactive = df[df["az_active"] == False]

    black_data = df_az_active[df_az_active["az_black"] == True]
    white_data = df_az_active[df_az_active["az_black"] == False]

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
                y="forced_corner_capture_executed",
                ax=ax[i][j],
                color=edax_plot_colours[j], 
                marker="h",
            )

            ax_current.set_ylim(0, 0.1)
            ax_current.set_xlabel("Training Iteration")
            ax_current.set_ylabel("Average forced corner capture rate per game")
            ax_current.set_title(f"AZ ({colour.capitalize()}) vs Edax depth {edax_version} ({other_colour.capitalize()})")

            ax_current.margins(x=0)
            xmin, xmax = ax_current.get_xlim()
            ax_current.axhline(edax_data[str(edax_version)]["mean"], color="blue", linestyle='--', label="Edax 21 average forced corner capture")
           
            ax_current.fill_between([xmin, xmax], edax_data[str(edax_version)]["upper"], edax_data[str(edax_version)]["lower"], alpha=0.2, color="blue")
          

            if (edax_version == 21 and colour == "white"):
                ax_current.axhline(edax15_fcc[colour]["mean"], color="orange", linestyle='--', label="Edax 15 average forced corner capture vs Edax 21")
               
                ax_current.fill_between([xmin, xmax], edax15_fcc[colour]["upper"], edax15_fcc[colour]["lower"], alpha=0.2, color="orange")
               

            ax_current.legend()

    fig.suptitle("Average AZ forced corner capture rate against Edax versions 6, 15, 21", fontsize=16)
    fig.tight_layout(rect=[0, 0, 1, 0.97])
    fig.savefig(save_dir / save_filename, dpi=300, bbox_inches="tight")


def plot_parity_training(games, moves, save_dir, save_filename, all_games):
    edax21_parity_black = {}
    edax21_parity_white = {}
    edax15_parity = {}
    
    no_resign = games[games["resign"] == False]

    all_endgame_moves = moves[(moves["num_discs"] >= 48) & (moves["pass"] == False)]
    edax21_moves = separate_edax21_moves(all_games, all_endgame_moves)

    df = moves.merge(
        no_resign[["game_id", "az_version", "az_black", "edax_version", "az_won"]],
        on="game_id"
    )

    df = df[(df["num_discs"] >= 48) & (df["pass"] == False)]

    for edax_version in edax_versions: 
        black_moves = edax21_moves["black"][str(edax_version)]
        white_moves = edax21_moves["white"][str(edax_version)]

        black_active = black_moves[black_moves["black_active"] == True]
        white_active = white_moves[white_moves["black_active"] == False]
        edax_version_black_active = white_moves[white_moves["black_active"] == True]
        edax_version_white_active = black_moves[black_moves["black_active"] == False]

        edax21_parity_black[str(edax_version)] = bootstrap_stats(black_active["parity"])

        edax21_parity_white[str(edax_version)] = bootstrap_stats(white_active["parity"])

        if (edax_version == 15):
            edax15_parity["black"] = bootstrap_stats(edax_version_black_active["parity"])
            edax15_parity["white"] = bootstrap_stats(edax_version_white_active["parity"])


    df["az_active"] = np.where((((df["az_black"] == True) & (df["black_active"] == True)) | ((df["az_black"] == False) & (df["black_active"] == False))), True, False)

    # For each game, aggregate average (same as proportion) of parity successes for both 
    df_az_active = df[df["az_active"] == True]
    df_az_inactive = df[df["az_active"] == False]

    black_data = df_az_active[df_az_active["az_black"] == True]
    white_data = df_az_active[df_az_active["az_black"] == False]

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
                y="parity",
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

            ax_current.margins(x=0)
            xmin, xmax = ax_current.get_xlim()
            ax_current.axhline(edax_data[str(edax_version)]["mean"], color="blue", linestyle='--', label="Edax 21 average parity")
          
            ax_current.fill_between([xmin, xmax], edax_data[str(edax_version)]["upper"], edax_data[str(edax_version)]["lower"], alpha=0.2, color="blue")
         

            if (edax_version == 21 and colour == "white"):
                ax_current.axhline(edax15_parity[colour]["mean"], color="orange", linestyle='--', label="Edax 15 average parity vs Edax 21")
              
                ax_current.fill_between([xmin, xmax], edax15_parity[colour]["upper"], edax15_parity[colour]["lower"], alpha=0.2, color="orange")
                

            ax_current.set_xlabel("Training Iteration")
            ax_current.set_ylabel("Average proportion of parity successes")
            ax_current.set_title(f"AZ ({colour.capitalize()}) vs Edax depth {edax_version} ({other_colour.capitalize()})")
            ax_current.legend()

    fig.suptitle("Average AZ parity successes against Edax versions 6, 15, 21", fontsize=16)
    fig.tight_layout(rect=[0, 0, 1, 0.97])
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

        edax21_frontier_black[str(edax_version)] = bootstrap_stats(per_game_black["avg_black_frontier_diff"])

        edax21_frontier_white[str(edax_version)] = bootstrap_stats(per_game_white["avg_white_frontier_diff"])

        if (edax_version == 15):
            edax15_frontier["black"] = bootstrap_stats(per_game_edax_version_black["avg_black_frontier_diff"])
            edax15_frontier["white"] = bootstrap_stats(per_game_edax_version_white["avg_white_frontier_diff"])

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
      .agg(avg_az_frontier=("az_frontier", "mean"), avg_edax_frontier=("edax_frontier", "mean"), avg_frontier_diff=("az_frontier_diff", "mean"))
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

            ax_current.margins(x=0)
            xmin, xmax = ax_current.get_xlim()
            ax_current.axhline(edax_data[str(edax_version)]["mean"], color="blue", linestyle='--', label="Edax 21 average frontier count")
            ax_current.fill_between([xmin, xmax], edax_data[str(edax_version)]["upper"], edax_data[str(edax_version)]["lower"], alpha=0.2, color="blue")

            if (edax_version == 21 and colour == "white"):
                ax_current.axhline(edax15_frontier[colour]["mean"], color="orange", linestyle='--', label="Edax 15 average frontier difference vs Edax 21")
                ax_current.fill_between([xmin, xmax], edax15_frontier[colour]["upper"], edax15_frontier[colour]["lower"], alpha=0.2, color="orange")

            ax_current.set_xlabel("Training Iteration")
            ax_current.set_ylabel("Average Frontier difference (per game)")
            ax_current.set_title(f"AZ ({colour.capitalize()}) vs Edax depth {edax_version} ({other_colour.capitalize()})")
            ax_current.legend()

    fig.suptitle("AZ average frontier difference against Edax versions 6, 15, 21", fontsize=16)
    fig.tight_layout(rect=[0, 0, 1, 0.97])
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

    # Calculate win-rate and confidence interval for edax 15 vs edax 21
    data = edax15_vs_edax21["edax15_won"]
    edax15_winrate = data.mean() 
    res = stats.bootstrap((data,), np.mean, confidence_level=0.95, n_resamples=5000, method='percentile')
    lower, upper = res.confidence_interval

    # Plot win rates 
    fig, ax = plt.subplots(2, 1, figsize=(12, 14))

    for i, colour in enumerate(colours):
        ax_current = ax[i]
        az_black = (colour == "black")
        colour_data = games[games["az_black"] == az_black]
        sns.lineplot(
            data=colour_data,
            x="az_version",
            y="az_won",
            hue="edax_version",
            ax=ax_current,
            palette={6: 'green', 15: 'orange', 21: 'blue'},
            marker="h",
            errorbar=("ci", 95))
        """
        ax_current.axhline(edax15_winrate, color='firebrick', linestyle='--', label="Edax 15 winrate against Edax 21")

        ax_current.margins(x=0)
        xmin, xmax = ax_current.get_xlim()
       
        ax_current.fill_between([xmin, xmax], lower, upper, alpha=0.2, color="red")
       """

        ax_current.set_xlabel("Training Iteration")
        ax_current.set_ylabel("AZ Winrate")
        ax_current.set_title(f"AZ as {colour}")

        handles, labels = ax_current.get_legend_handles_labels()
        labels = [f"AZ winrate against Edax {l}" if l.lstrip('-').isdigit() else l for l in labels]
        ax_current.legend(handles, labels)
        ax_current.grid()

    fig.suptitle("AZ winrate against Edax versions 6, 15, 21", fontsize=16)
    fig.tight_layout(rect=[0, 0, 1, 0.97])
    fig.savefig(save_dir / save_filename, dpi=300)

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
            .agg(avg_black_stable=("num_stable_black", "mean"), avg_white_stable=("num_stable_white", "mean"))
        .reset_index())

        per_game_white = (
            white_moves.groupby(["black_version", "white_version", "game_id"])
            .agg(avg_black_stable=("num_stable_black", "mean"), avg_white_stable=("num_stable_white", "mean"))
        .reset_index())

        edax21_stability_black[str(edax_version)] = bootstrap_stats(per_game_black["avg_black_stable"])

        edax21_stability_white[str(edax_version)] = bootstrap_stats(per_game_white["avg_white_stable"])

        if (edax_version == 15):
            edax15_stability["black"] = bootstrap_stats(per_game_white["avg_black_stable"])
            edax15_stability["white"] = bootstrap_stats(per_game_black["avg_white_stable"])
      
    no_resign = games[games["resign"] == False]
    df = moves.merge(
        no_resign[["game_id", "az_version", "az_black", "edax_version", "az_won", "draw"]],
        on="game_id"
    )

    df["az_stable"] = np.where(df["az_black"], df["num_stable_black"], df["num_stable_white"])

    df["edax_stable"] = np.where(df["az_black"], df["num_stable_white"], df["num_stable_black"])

    per_game = (
    df.groupby(["az_version", "az_black", "edax_version", "game_id"])
      .agg(avg_az_stable=("az_stable", "mean"), avg_edax_stable=("edax_stable", "mean"))
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
            edax_data = edax_data[str(edax_version)]

            ax_current = ax[i][j]
            sns.lineplot(
                data=data,
                x="az_version",
                y="avg_az_stable",
                ax=ax[i][j],
                color=edax_plot_colours[j],
                marker="h",)

            ax_current.axhline(edax_data["mean"], color="blue", linestyle='--', label="Edax 21 average stable disc count vs Edax 21")

            ax_current.margins(x=0)
            xmin, xmax = ax_current.get_xlim()
            ax_current.fill_between([xmin, xmax], edax_data["upper"], edax_data["lower"], alpha=0.2, color="blue")

            ax_current.set_xlabel("Training Iteration")
            ax_current.set_ylabel("Average AZ stable disc count (per game)")
            ax_current.set_title(f"AZ ({colour.capitalize()}) vs Edax depth {edax_version} ({other_colour.capitalize()})")
            ax_current.set_ylim(0, 8)

            if (edax_version == 21 and colour == "white"):
                ax_current.axhline(edax15_stability[colour]["mean"], color="orange", linestyle='--', label="Edax 15 average stable disc count vs Edax 21")
                ax_current.fill_between([xmin, xmax], edax15_stability[colour]["upper"], edax15_stability[colour]["lower"], alpha=0.2, color="orange")

            ax_current.legend()
    
    fig.suptitle("AZ average stable disc count against Edax versions 6, 15, 21", fontsize=16)
    fig.tight_layout(rect=[0, 0, 1, 0.97])
    fig.savefig(save_dir / save_filename, dpi=300, bbox_inches="tight")
    

def plot_mobility_training(games, moves, save_dir, save_filename, all_games):
    edax21_mobility_black = {}
    edax21_mobility_white = {}
    edax15_mobility = {}
    
    edax21_moves = separate_edax21_moves(all_games, moves)

    for edax_version in edax_versions: 
        black_moves = edax21_moves["black"][str(edax_version)]
        white_moves = edax21_moves["white"][str(edax_version)]

        black_moves["black_moves_diff"] = black_moves["num_moves_black"] - black_moves["num_moves_white"]

        white_moves["white_moves_diff"] = white_moves["num_moves_white"] - white_moves["num_moves_black"]
        white_moves["black_moves_diff"] = white_moves["num_moves_black"] - white_moves["num_moves_white"]
        black_moves["white_moves_diff"] = black_moves["num_moves_white"] - black_moves["num_moves_black"]

        per_game_black = (
            black_moves.groupby(["black_version", "white_version", "game_id"])
            .agg(avg_black_moves_diff=("black_moves_diff", "mean"))
        .reset_index())

        per_game_white = (
            white_moves.groupby(["black_version", "white_version", "game_id"])
            .agg(avg_white_moves_diff=("white_moves_diff", "mean"))
        .reset_index())

        per_game_edax_version_black = (
            white_moves.groupby(["black_version", "white_version", "game_id"])
            .agg(avg_black_moves_diff=("black_moves_diff", "mean"))
        .reset_index())

        per_game_edax_version_white = (
            black_moves.groupby(["black_version", "white_version", "game_id"])
            .agg(avg_white_moves_diff=("white_moves_diff", "mean"))
        .reset_index())

        edax21_mobility_black[str(edax_version)] = bootstrap_stats(per_game_black["avg_black_moves_diff"])

        edax21_mobility_white[str(edax_version)] = bootstrap_stats(per_game_white["avg_white_moves_diff"])

        if (edax_version == 15):
            edax15_mobility["black"] = bootstrap_stats(per_game_edax_version_black["avg_black_moves_diff"])
            edax15_mobility["white"] = bootstrap_stats(per_game_edax_version_white["avg_white_moves_diff"])

    # Filter for only games with no resign to make mobility average calculation fair. Resign games have less moves, so they only average mobility over the early/mid game where other games also include the end game
    no_resign = games[games["resign"] == False]
    df = moves.merge(
        no_resign[["game_id", "az_version", "az_black", "edax_version", "az_won"]],
        on="game_id"
    )

    # Create columns for AZ/opponent moves, and moves difference from perspective of AZ
    df["az_moves"] = np.where(df["az_black"], df["num_moves_black"], df["num_moves_white"])

    df["edax_moves"] = np.where(df["az_black"], df["num_moves_white"], df["num_moves_black"])

    df["az_moves_diff"] = df["az_moves"] - df["edax_moves"]

    # Average moves and moves diff by game
    per_game = (
    df.groupby(["az_version", "az_black", "edax_version", "game_id"])
      .agg(avg_az_moves=("az_moves", "mean"), avg_edax_moves=("edax_moves", "mean"), avg_moves_diff=("az_moves_diff", "mean"))
      .reset_index()
    )

    black_data = per_game[per_game["az_black"] == True]
    white_data = per_game[per_game["az_black"] == False]

    # Create plots of AZ version against moves
    fig, ax = plt.subplots(2, 3, figsize=(18, 10), sharex=True)

    for i, colour in enumerate(colours):
        for j, edax_version in enumerate(edax_versions):
            other_colour = "black" if colour == "white" else "white"

            data = black_data if colour == "black" else white_data
            data = data[data["edax_version"] == edax_version]

            edax_data = edax21_mobility_black if colour == "black" else edax21_mobility_white;

            ax_current = ax[i][j]
            sns.lineplot(
                data=data,
                x="az_version",
                y="avg_moves_diff",
                ax=ax[i][j],
                color=edax_plot_colours[j],
                marker="h",)

            ax_current.set_ylim(-4, 8)
            ax_current.axhline(0, color='firebrick')

            ax_current.margins(x=0)
            xmin, xmax = ax_current.get_xlim()
            ax_current.axhline(edax_data[str(edax_version)]["mean"], color="blue", linestyle='--', label="Edax 21 average move count difference vs Edax 21")
            ax_current.fill_between([xmin, xmax], edax_data[str(edax_version)]["upper"], edax_data[str(edax_version)]["lower"], alpha=0.2, color="blue")

            if (edax_version == 21 and colour == "white"):
                ax_current.axhline(edax15_mobility[colour]["mean"], color="orange", linestyle='--', label="Edax 15 average move count difference vs Edax 21")
                ax_current.fill_between([xmin, xmax], edax15_mobility[colour]["upper"], edax15_mobility[colour]["lower"], alpha=0.2, color="orange")

            ax_current.set_xlabel("Training Iteration")
            ax_current.set_ylabel("Mean Move Count difference (per game)")
            ax_current.set_title(f"AZ ({colour.capitalize()}) vs Edax depth {edax_version} ({other_colour.capitalize()})")
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


# Create and save frontier plot
mobility_save_dir = Path(base_save_dir + "/mobility")
prepare_save_dir(mobility_save_dir)

plot_mobility_training(az_games, moves, mobility_save_dir, "mobility_difference.png", games)



# Create and save stable disc plot 

stable_disc_save_dir = Path(base_save_dir + "/stable")
prepare_save_dir(stable_disc_save_dir)

plot_stable(az_games, moves, stable_disc_save_dir, "stable.png", games)



# Create and save win rate plot

win_rate_save_dir = Path(base_save_dir + "/win_rate")
prepare_save_dir(win_rate_save_dir)

plot_win_rate(az_games, moves, win_rate_save_dir, "win_rate.png", games)




# Create and save frontier plot
frontier_save_dir = Path(base_save_dir + "/frontier")
prepare_save_dir(frontier_save_dir)

plot_frontier_training(az_games, moves, frontier_save_dir, "frontier_difference.png", games)



# Create and save forced corner capture plots
fcc_save_dir = Path(base_save_dir + "/forced_corner_capture")
prepare_save_dir(fcc_save_dir)

plot_forced_corner_capture(az_games, moves, fcc_save_dir, "forced_corner_capture.png", games)



# Create and save parity plot 
parity_save_dir = Path(base_save_dir + "/parity")
prepare_save_dir(parity_save_dir)

plot_parity_training(az_games, moves, parity_save_dir, "parity.png", games)
