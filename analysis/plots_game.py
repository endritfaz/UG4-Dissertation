import matplotlib.pyplot as plt
import pandas as pd
import psycopg2
import seaborn as sns
import shutil
from pathlib import Path
from sqlalchemy import create_engine
import numpy as np 
import sys

def plot_stability(az_iteration, moves)
if __name__ == "__main__": 
    engine = create_engine("postgresql+psycopg2://postgres:postgres@localhost:5432/ug4")

    games = pd.read_sql("SELECT * FROM games;", engine)
    moves = pd.read_sql("SELECT * FROM moves;", engine)

    # TODO: Select games belonging only to AZ iteration specified by program argument
    iteration = sys.argv[1]

    az_edax_games = games[((games["black"] == "az") & (games["white"] == "edax")) | (((games["white"] == "az") & (games["black"] == "edax")))]

    az_games["az_black"] = np.where(az_games["black"] == "az", True, False); 

    az_games["edax_version"] = np.where(az_games["az_black"], az_games["white_version"], az_games["black_version"]).astype(int)

    iteration_moves = moves.merge([""])

    # TODO: Plot and save strategy profiles 