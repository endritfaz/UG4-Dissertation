import matplotlib.pyplot as plt
import pandas as pd
import psycopg2
import seaborn as sns
import shutil
from pathlib import Path
from sqlalchemy import create_engine
import numpy as np 
import sys

edax_versions = [6, 15, 21]
edax_plot_colours = ["green", "orange", "blue"]
colours = ["black", "white"]

def plot_binary_feature(az_iterations, move_list, colour, edax_version, feature, feature_str, save_dir, save_name): 
    data_list = list(zip(az_iterations, move_list))

    for i, (iteration, moves) in enumerate(data_list):
        if (feature == "parity"):
            moves = moves[moves["pass"] == False]

        moves["az_active"] = np.where((((moves["az_black"] == True) & (moves["black_active"] == True)) | ((moves["az_black"] == False) & (moves["black_active"] == False))), True, False)

        data_list[i] = (iteration, moves)

    fig, ax = plt.subplots(1, len(az_iterations), figsize=(18, 4), sharey=True)

    edax_line_colour = edax_plot_colours[edax_versions.index(edax_version)]

    for i, (iteration, moves) in enumerate(data_list):
        black_data = moves[moves["az_black"] == True]
        white_data = moves[moves["az_black"] == False]

        data = black_data if colour == "black" else white_data
        data = data[data["edax_version"] == edax_version]

        data = black_data if colour == "black" else white_data 
        data = data[data["edax_version"] == edax_version]

        ax_current = ax[i]
        sns.lineplot(
            data=data[data["az_active"] == True],
            x="ply",  
            y=feature,
            ax=ax_current,
            color=edax_line_colour, 
            marker="h",
            label=f"AZ average {feature}")
        
        """
        sns.lineplot(
            data=data[data["az_active"] == False],
            x="ply",  
            y=feature,
            ax=ax_current,
            color=edax_line_colour, 
            marker="P",
            linestyle="--",
            label= f"Edax average {feature}")
        """

        ax_current.set_xlabel("Ply")
        ax_current.set_ylabel(f"Average {feature}")
        ax_current.set_title(f"AZ {iteration} ({colour.capitalize()}) vs Edax depth {edax_version} ({other_colour.capitalize()})")

        ax_current.legend()

    fig.savefig(save_dir / save_name, dpi=300, bbox_inches="tight")

def plot_disc_feature(az_iterations, move_list, colour, edax_version, feature, feature_str, save_dir, save_name):
    data_list = list(zip(az_iterations, move_list))

    for (iteration, moves) in data_list:
        moves[f"num_{feature}_az"] = np.where(moves["az_black"], moves[f"num_{feature}_black"], moves[f"num_{feature}_white"])

        moves[f"num_{feature}_edax"] = np.where(moves["az_black"], moves[f"num_{feature}_white"], moves[f"num_{feature}_black"])

    
    fig, ax = plt.subplots(1, len(az_iterations), figsize=(18, 4), sharey=True)

    edax_line_colour = edax_plot_colours[edax_versions.index(edax_version)]

    for i, (iteration, moves) in enumerate(data_list):
        black_data = moves[moves["az_black"] == True]
        white_data = moves[moves["az_black"] == False]

        data = black_data if colour == "black" else white_data
        data = data[data["edax_version"] == edax_version]

        ax_current = ax[i]
        sns.lineplot(
            data=data,
            x="num_discs",  
            y=f"num_{feature}_az",
            ax=ax_current,
            color=edax_line_colour, 
            marker="h",
            label=f"AZ average {feature_str} discs")
        
        sns.lineplot(
            data=data,
            x="num_discs",  
            y=f"num_{feature}_edax",
            ax=ax_current,
            color=edax_line_colour, 
            marker="P",
            linestyle="--",
            label= f"Edax average {feature_str} discs")

        ax_current.set_xlabel("Number of discs")
        ax_current.set_ylabel(f"Average {feature_str} discs")
        ax_current.set_title(f"AZ training step {iteration} ({colour.capitalize()}) vs Edax depth {edax_version} ({other_colour.capitalize()})")

        ax_current.legend()

    fig.savefig(save_dir / save_name, dpi=300, bbox_inches="tight")

def prepare_save_dir(output_dir):
    """
    Clear and recreate output_dir.
    """
    if output_dir.exists():
        return; 
    output_dir.mkdir(parents=True, exist_ok=True)


base_save_dir = "game_data_plots/game"

# Usage: python3 plots_game.py iteration_1,...,iteration_n edax_version colour
if __name__ == "__main__": 
    iterations = sys.argv[1].split(",")
    edax_version = sys.argv[2]
    colour = sys.argv[3]
    other_colour = "black" if colour == "white" else "white"

    engine = create_engine("postgresql+psycopg2://postgres:postgres@localhost:5432/ug4")

    games = pd.read_sql("SELECT * FROM games;", engine)
    moves = pd.read_sql("SELECT * FROM moves;", engine)

    iteration_dfs = [] 
    
    for iteration in iterations: 
        az_iteration_games = games[((games[colour] == "az") & (games[f"{colour}_version"] == iteration))]

        az_edax_games = az_iteration_games[az_iteration_games[f"{other_colour}_version"] == edax_version]

        az_edax_games["az_black"] = np.where(az_edax_games["black"] == "az", True, False) 

        az_edax_games["edax_version"] = np.where(az_edax_games["az_black"], az_edax_games["white_version"], az_edax_games["black_version"]).astype(int)

        iteration_moves = moves.merge(az_edax_games[["game_id", "az_black", "edax_version"]], on="game_id")

        iteration_dfs.append(iteration_moves)

  
    # Stable discs
    save_dir = Path(base_save_dir + "/stable")
    prepare_save_dir(save_dir)
    save_name = f"stable-{iteration}.png"
    
    plot_disc_feature(iterations, iteration_dfs, colour, int(edax_version), "stable", "stable", save_dir, save_name)
    
    
    # Frontier discs
    save_dir = Path(base_save_dir + "/frontier")
    prepare_save_dir(save_dir)
    save_name = f"frontier-{iteration}.png"

    plot_disc_feature(iterations, iteration_dfs, colour, int(edax_version), "frontier", "frontier", save_dir, save_name)


    # Forced corner capture
    save_dir = Path(base_save_dir + "/fcc")
    prepare_save_dir(save_dir)
    save_name = f"fcc-{iteration}.png"
    
    plot_binary_feature(iterations, iteration_dfs, colour, int(edax_version),"forced_corner_capture_executed", "FCC executions", save_dir, save_name)


    # Parity
    save_dir = Path(base_save_dir + "/parity")
    prepare_save_dir(save_dir)
    save_name = f"parity-{iteration}.png"
    
    plot_binary_feature(iterations, iteration_dfs, colour, int(edax_version),"parity", "parity successes", save_dir, save_name)
    