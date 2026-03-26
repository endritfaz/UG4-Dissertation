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

def plot_binary_feature(az_iteration, moves, feature, feature_str, save_dir, save_name): 
    moves["az_active"] = np.where((((moves["az_black"] == True) & (moves["black_active"] == True)) | ((moves["az_black"] == False) & (moves["black_active"] == False))), True, False)

    per_ply = moves.groupby(["ply", "edax_version", "az_black", "az_active"]).agg(avg_feature=(feature, "mean")).reset_index()

    black_data = per_ply[per_ply["az_black"] == True]
    white_data = per_ply[per_ply["az_black"] == False]

    # Create plots of AZ version against average forced corner capture rate 
    fig, ax = plt.subplots(2, 3, figsize=(18, 10), sharex=True)

    for i, colour in enumerate(colours):
        for j, edax_version in enumerate(edax_versions): 
            other_colour = "black" if colour == "white" else "white"

            data = black_data if colour == "black" else white_data 
            data = data[data["edax_version"] == edax_version]

            ax_current = ax[i][j]
            sns.lineplot(
                data=data[data["az_active"] == True],
                x="ply",  
                y="avg_feature",
                ax=ax[i][j],
                color=edax_plot_colours[j], 
                marker="h",
                label=f"AZ average {feature}")
            
            sns.lineplot(
                data=data[data["az_active"] == False],
                x="ply",  
                y="avg_feature",
                ax=ax[i][j],
                color=edax_plot_colours[j], 
                marker="P",
                linestyle="--",
                label= f"Edax average {feature}")

            ax_current.set_xlabel("Ply")
            ax_current.set_ylabel(f"Average {feature}")
            ax_current.set_title(f"AZ ({colour.capitalize()}) vs Edax depth {edax_version} ({other_colour.capitalize()})")

            ax_current.legend()

    fig.savefig(save_dir / save_name, dpi=300, bbox_inches="tight")

def plot_disc_feature(az_iteration, moves, feature, feature_str, save_dir, save_name):
    # 6 plots, one for every edax version and colour

    moves[f"num_{feature}_az"] = np.where(moves["az_black"], moves[f"num_{feature}_black"], moves[f"num_{feature}_white"])

    moves[f"num_{feature}_edax"] = np.where(moves["az_black"], moves[f"num_{feature}_white"], moves[f"num_{feature}_black"])

    
    # per_ply = moves.groupby(["ply", "edax_version", "az_black"]).agg(avg_feature_az=(f"num_{feature}_az", "mean"), avg_feature_edax=(f"num_{feature}_edax", "mean")).reset_index()

    black_data = moves[moves["az_black"] == True]
    white_data = moves[moves["az_black"] == False]

    fig, ax = plt.subplots(2, 3, figsize=(18, 10), sharex=True)

    for i, colour in enumerate(colours):
        for j, edax_version in enumerate(edax_versions): 
            other_colour = "black" if colour == "white" else "white"

            data = black_data if colour == "black" else white_data 
            data = data[data["edax_version"] == edax_version]

            ax_current = ax[i][j]
            sns.lineplot(
                data=data,
                x="num_discs",  
                y=f"num_{feature}_az",
                ax=ax[i][j],
                color=edax_plot_colours[j], 
                marker="h",
                label=f"AZ average {feature_str} discs")
            
            sns.lineplot(
                data=data,
                x="num_discs",  
                y=f"num_{feature}_edax",
                ax=ax[i][j],
                color=edax_plot_colours[j], 
                marker="P",
                linestyle="--",
                label= f"Edax average {feature_str} discs")

            ax_current.set_xlabel("Number of discs")
            ax_current.set_ylabel(f"Average {feature_str} discs")
            ax_current.set_title(f"AZ ({colour.capitalize()}) vs Edax depth {edax_version} ({other_colour.capitalize()})")

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

if __name__ == "__main__": 
    engine = create_engine("postgresql+psycopg2://postgres:postgres@localhost:5432/ug4")

    games = pd.read_sql("SELECT * FROM games;", engine)
    moves = pd.read_sql("SELECT * FROM moves;", engine)

    # Select games belonging only to AZ iteration specified by program argument
    iteration = sys.argv[1]

    az_iteration_games = games[((games["black"] == "az") & (games["black_version"] == iteration)) | (((games["white"] == "az") & (games["white_version"] == iteration)))]

    az_edax_games = az_iteration_games[((az_iteration_games["black"] == "az") & (az_iteration_games["white"] == "edax")) | (((az_iteration_games["white"] == "az") & (az_iteration_games["black"] == "edax")))]

    az_edax_games["az_black"] = np.where(az_edax_games["black"] == "az", True, False) 

    az_edax_games["edax_version"] = np.where(az_edax_games["az_black"], az_edax_games["white_version"], az_edax_games["black_version"]).astype(int)

    iteration_moves = moves.merge(az_edax_games[["game_id", "az_black", "edax_version"]], on="game_id")


    # Stable discs
    save_dir = frontier_save_dir = Path(base_save_dir + "/stable")
    prepare_save_dir(save_dir)
    save_name = f"stable-{iteration}.png"
    
    plot_disc_feature(iteration, iteration_moves, "stable", "stable", save_dir, save_name)

    # Frontier discs
    save_dir = frontier_save_dir = Path(base_save_dir + "/frontier")
    prepare_save_dir(save_dir)
    save_name = f"frontier-{iteration}.png"

    plot_disc_feature(iteration, iteration_moves, "frontier", "frontier", save_dir, save_name)

    # Forced corner capture
    save_dir = frontier_save_dir = Path(base_save_dir + "/fcc")
    prepare_save_dir(save_dir)
    save_name = f"fcc-{iteration}.png"
    
    plot_binary_feature(iteration, iteration_moves, "forced_corner_capture_executed", "FCC executions", save_dir, save_name)

    # Parity
    save_dir = frontier_save_dir = Path(base_save_dir + "/parity")
    prepare_save_dir(save_dir)
    save_name = f"parity-{iteration}.png"
    
    plot_binary_feature(iteration, iteration_moves, "parity", "parity successes", save_dir, save_name)
