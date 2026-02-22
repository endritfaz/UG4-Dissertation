import matplotlib.pyplot as plt
import pandas as pd
import psycopg2
import seaborn as sns
import shutil
from pathlib import Path
from sqlalchemy import create_engine
import numpy as np 

edax_versions = [6, 15, 21]
edax_plot_colours = ["green", "orange", "blue"]
colours = ["black", "white"]

def plot_forced_corner_capture(games, moves):
    pass

def plot_parity_training(games, moves):
    no_resign = games[games["resign"] == False]

    df = moves.merge(
        no_resign[["game_id", "az_version", "az_black", "edax_version", "az_won"]],
        on="game_id"
    )

    df["az_active"] = np.where((((df["az_black"] == True) & (df["black_active"] == True)) | ((df["az_black"] == False) & (df["black_active"] == False))), True, False)

    # For each game, aggregate average (same as proportion) of parity successes for both 
    df_az_active = df[df["az_active"] == True]
    df_az_inactive = df[df["az_active"] == False]

    per_game_t = (
    df.groupby(["game_id", "az_version", "edax_version", "az_black", "az_active"])
        .agg(avg_parity_success=("parity", "mean"))
        .reset_index()
    )
    
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

    plt.show()
    
def plot_frontier_training(games, moves):
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

            ax_current.axhline(0, color='firebrick')

            ax_current.set_xlabel("Training Iteration")
            ax_current.set_ylabel("Mean Frontier difference (per game)")
            ax_current.set_title(f"AZ ({colour.capitalize()}) vs Edax depth {edax_version} ({other_colour.capitalize()})")

    plt.show()



engine = create_engine(
    "postgresql+psycopg2://postgres:postgres@localhost:5432/ug4"
)

games = pd.read_sql("SELECT * FROM games;", engine)
moves = pd.read_sql("SELECT * FROM moves;", engine)


az_games = games[(games["black"] == "az") | (games["white"] == "az")]

# Create edax and az version columns in games df to make further analysis easier
az_games["az_black"] = (az_games["black"] == "az"); 

az_games["az_version"] = np.where(az_games["az_black"], az_games["black_version"], az_games["white_version"]).astype(int)

az_games["az_won"] = np.where(((az_games["az_black"] == True) & (az_games["winner"] == "black")) | ((az_games["az_black"] == False) & (az_games["winner"] == "white")), True, False)

az_games["edax_version"] = np.where(az_games["az_black"], az_games["white_version"], az_games["black_version"]).astype(int)

