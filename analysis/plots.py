import matplotlib.pyplot as plt
import pandas as pd
import psycopg2
import seaborn as sns
import shutil
from pathlib import Path

DB_NAME = "ug4"
DB_USER = "postgres"
DB_PASS = "postgres"
DB_HOST = "127.0.0.1"
DB_PORT = "5432"
WIN_RATES_DIR = Path("game_data_plots/win_rates")
FRONTIER_DIR = Path("game_data_plots/frontier")
FORCED_CORNER_DIR = Path("game_data_plots/forced_corner")
AVG_GAME_FRONTIER_DIR = Path("game_data_plots/avg_game_frontier")

# Taken from: https://medium.com/@alestamm/importing-data-from-a-postgresql-database-to-a-pandas-dataframe-5f4bffcd8bb2
def sql_to_dataframe(conn, query, column_names):
    """
    Import data from a PostgreSQL database using a SELECT query 
    """
    cursor = conn.cursor()
   
    try:
        cursor.execute(query) 
    except (Exception, psycopg2.DatabaseError) as error:
        print("Error: %s" % error)
        cursor.close()
        return 1
     
    # The execute returns a list of tuples:
    tuples_list = cursor.fetchall()
    cursor.close()
    # Now we need to transform the list into a pandas DataFrame:
    df = pd.DataFrame(tuples_list, columns=column_names)
    return df


def build_az_edax_rates(games_df):
    """
    Build per-(az_version, edax_version) win/resign/draw rates from games_df.
    """
    df = games_df.copy()

    # Keep only AZ vs Edax games.
    df = df[
        ((df["black"] == "az") & (df["white"] == "edax"))
        | ((df["black"] == "edax") & (df["white"] == "az"))
    ].copy()

    # Normalize versions by player identity rather than color.
    df["az_version"] = df.apply(
        lambda row: row["black_version"] if row["black"] == "az" else row["white_version"],
        axis=1,
    )
    df["edax_version"] = df.apply(
        lambda row: row["black_version"] if row["black"] == "edax" else row["white_version"],
        axis=1,
    )

    df["az_version"] = pd.to_numeric(df["az_version"], errors="coerce")
    df["edax_version"] = pd.to_numeric(df["edax_version"], errors="coerce")
    df = df.dropna(subset=["az_version", "edax_version"])
    df["az_version"] = df["az_version"].astype(int)
    df["edax_version"] = df["edax_version"].astype(int)

    # Binary outcome flags from AZ perspective.
    df["az_win"] = (
        ((df["winner"] == "black") & (df["black"] == "az"))
        | ((df["winner"] == "white") & (df["white"] == "az"))
    ).astype(float)
    df["draw"] = (df["winner"] == "draw").astype(float)
    df["az_resign"] = (df["resign"] == True) & (df["az_win"] == False)

    rates = (
        df.groupby(["az_version", "edax_version"], as_index=False)[["az_win", "draw", "az_resign"]]
        .mean()
        .rename(
            columns={
                "az_win": "win_rate",
                "draw": "draw_rate",
                "az_resign": "resign_rate",
            }
        )
        .sort_values(["edax_version", "az_version"])
    )

    return rates


def prepare_output_dir(output_dir):
    """
    Clear and recreate output_dir.
    """
    if output_dir.exists():
        shutil.rmtree(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)


def plot_az_edax_rates(rates_df, output_dir):
    """
    Plot win/draw/resign rates with AZ version on x and one line per Edax version.
    """
    sns.set_theme(style="whitegrid")
    fig, axes = plt.subplots(1, 3, figsize=(20, 5), sharex=True, sharey=True)

    metrics = [
        ("win_rate", "Win Rate"),
        ("resign_rate", "Resign Rate"),
        ("draw_rate", "Draw Rate"),
    ]

    plot_df = rates_df.copy()
    plot_df["edax_version"] = plot_df["edax_version"].astype(str)

    for ax, (metric, title) in zip(axes, metrics):
        sns.lineplot(
            data=plot_df,
            x="az_version",
            y=metric,
            hue="edax_version",
            marker="o",
            ax=ax,
        )
        ax.set_title(title)
        ax.set_xlabel("AlphaZero Version")
        ax.set_ylabel("Rate")
        ax.set_ylim(0, 1)
        ax.legend(title="Edax Version")

    fig.tight_layout()
    fig.savefig(output_dir / "az_edax_rates.png", dpi=300, bbox_inches="tight")

    # Save a dedicated win-rate plot too.
    win_fig, win_ax = plt.subplots(figsize=(8, 5))
    sns.lineplot(
        data=plot_df,
        x="az_version",
        y="win_rate",
        hue="edax_version",
        marker="o",
        ax=win_ax,
    )
    win_ax.set_title("Win Rate")
    win_ax.set_xlabel("AlphaZero Version")
    win_ax.set_ylabel("Rate")
    win_ax.set_ylim(0, 1)
    win_ax.legend(title="Edax Version")
    win_fig.tight_layout()
    win_fig.savefig(output_dir / "win_rate.png", dpi=300, bbox_inches="tight")

    return fig, axes


def plot_avg_frontier_by_discs(frontier_df, output_dir):
    """
    Save one 2x3 figure per AZ version:
    - row 1/2: AZ as black / white
    - col 1/2/3: Edax version 6 / 15 / 21
    x: num_discs, y: avg frontier (AZ and Edax)
    """
    sns.set_theme(style="whitegrid")

    df = frontier_df.copy()
    df["az_version"] = pd.to_numeric(df["az_version"], errors="coerce")
    df["edax_version"] = pd.to_numeric(df["edax_version"], errors="coerce")
    df["num_discs"] = pd.to_numeric(df["num_discs"], errors="coerce")
    df["avg_az_frontier"] = pd.to_numeric(df["avg_az_frontier"], errors="coerce")
    df["avg_edax_frontier"] = pd.to_numeric(df["avg_edax_frontier"], errors="coerce")
    df["az_colour"] = df["az_colour"].astype(str).str.lower()
    df = df.dropna(
        subset=["az_version", "edax_version", "num_discs", "avg_az_frontier", "avg_edax_frontier"]
    )

    df["az_version"] = df["az_version"].astype(int)
    df["edax_version"] = df["edax_version"].astype(int)
    df["num_discs"] = df["num_discs"].astype(int)

    az_versions = sorted(df["az_version"].unique())
    az_colours = ["black", "white"]
    edax_versions = [6, 15, 21]

    for az_version in az_versions:
        per_version = df[df["az_version"] == az_version]
        fig, axes = plt.subplots(2, 3, figsize=(18, 10), sharex=True, sharey=True)
        fig.suptitle(f"Average Frontier by Discs (AZ v{az_version})")

        for row, az_colour in enumerate(az_colours):
            for col, edax_version in enumerate(edax_versions):
                ax = axes[row][col]
                panel = per_version[
                    (per_version["az_colour"] == az_colour)
                    & (per_version["edax_version"] == edax_version)
                ].sort_values("num_discs")

                if panel.empty:
                    ax.text(0.5, 0.5, "No data", ha="center", va="center", transform=ax.transAxes)
                else:
                    panel_long = pd.concat(
                        [
                            panel[["num_discs", "avg_az_frontier"]]
                            .rename(columns={"avg_az_frontier": "avg_frontier"})
                            .assign(player="AZ"),
                            panel[["num_discs", "avg_edax_frontier"]]
                            .rename(columns={"avg_edax_frontier": "avg_frontier"})
                            .assign(player="Edax"),
                        ],
                        ignore_index=True,
                    )
                    sns.lineplot(
                        data=panel_long,
                        x="num_discs",
                        y="avg_frontier",
                        hue="player",
                        marker="o",
                        ax=ax,
                    )

                ax.set_title(f"AZ {az_colour.capitalize()} vs Edax {edax_version}")
                ax.set_xlabel("Discs on Board")
                ax.set_ylabel("Avg Frontier")

        fig.tight_layout(rect=[0, 0.03, 1, 0.96])
        fig.savefig(output_dir / f"azv{az_version}_frontier_by_discs.png", dpi=300, bbox_inches="tight")
        plt.close(fig)


def plot_forced_corner_capture_rate(forced_df, output_dir):
    """
    Plot proportion of games where AZ had at least one forced corner capture.
    x: AZ version, y: proportion, hue: Edax version
    """
    sns.set_theme(style="whitegrid")

    df = forced_df.copy()
    df["az_version"] = pd.to_numeric(df["az_version"], errors="coerce")
    df["edax_version"] = pd.to_numeric(df["edax_version"], errors="coerce")
    df["forced_corner_capture_rate"] = pd.to_numeric(df["forced_corner_capture_rate"], errors="coerce")
    df = df.dropna(subset=["az_version", "edax_version", "forced_corner_capture_rate"])
    df["az_version"] = df["az_version"].astype(int)
    df["edax_version"] = df["edax_version"].astype(int).astype(str)

    fig, ax = plt.subplots(figsize=(10, 6))
    sns.lineplot(
        data=df.sort_values(["edax_version", "az_version"]),
        x="az_version",
        y="forced_corner_capture_rate",
        hue="edax_version",
        marker="o",
        ax=ax,
    )
    ax.set_title("Forced Corner Capture Rate (AZ)")
    ax.set_xlabel("AlphaZero Version")
    ax.set_ylabel("Proportion of Games")
    ax.set_ylim(0, 1)
    ax.legend(title="Edax Version")

    fig.tight_layout()
    fig.savefig(output_dir / "forced_corner_capture_rate.png", dpi=300, bbox_inches="tight")
    plt.close(fig)


def plot_avg_game_frontier_by_iteration(avg_game_frontier_df, output_dir):
    """
    Plot AZ iteration vs average per-game frontier, split by AZ colour and Edax version (2x3).
    """
    sns.set_theme(style="whitegrid")

    df = avg_game_frontier_df.copy()
    df["az_version"] = pd.to_numeric(df["az_version"], errors="coerce")
    df["edax_version"] = pd.to_numeric(df["edax_version"], errors="coerce")
    df["avg_game_frontier"] = pd.to_numeric(df["avg_game_frontier"], errors="coerce")
    df["az_colour"] = df["az_colour"].astype(str).str.lower()
    df = df.dropna(subset=["az_version", "edax_version", "avg_game_frontier"])
    df["az_version"] = df["az_version"].astype(int)
    df["edax_version"] = df["edax_version"].astype(int)

    fig, axes = plt.subplots(2, 3, figsize=(18, 10), sharex=True, sharey=True)
    fig.suptitle("Average Per-Game AZ Frontier vs AZ Iteration")

    az_colours = ["black", "white"]
    edax_versions = [6, 15, 21]

    for row, az_colour in enumerate(az_colours):
        for col, edax_version in enumerate(edax_versions):
            ax = axes[row][col]
            panel = df[
                (df["az_colour"] == az_colour)
                & (df["edax_version"] == edax_version)
            ].sort_values("az_version")

            if panel.empty:
                ax.text(0.5, 0.5, "No data", ha="center", va="center", transform=ax.transAxes)
            else:
                sns.lineplot(
                    data=panel,
                    x="az_version",
                    y="avg_game_frontier",
                    marker="o",
                    ax=ax,
                )

            ax.set_title(f"AZ {az_colour.capitalize()} vs Edax {edax_version}")
            ax.set_xlabel("AlphaZero Iteration")
            ax.set_ylabel("Avg Frontier Per Game")

    fig.tight_layout(rect=[0, 0.03, 1, 0.96])
    fig.savefig(output_dir / "avg_game_frontier_by_iteration.png", dpi=300, bbox_inches="tight")
    plt.close(fig)


try:
    conn = psycopg2.connect(database=DB_NAME, user=DB_USER, password=DB_PASS, host=DB_HOST, port=DB_PORT)

    print("Database connection succeeded!")

    games_query = "SELECT * FROM games"
    # TODO: Currently hardcoded, in future get columns directly from database
    games_columns = ["game_id", "black", "white", "black_version", "white_version", "winner", "resign"]

    games = sql_to_dataframe(conn, games_query, games_columns)

    # Plot win-rates, draw-rates and resign rates 
    rates = build_az_edax_rates(games)
   
    prepare_output_dir(WIN_RATES_DIR)
    plot_az_edax_rates(rates, WIN_RATES_DIR)
    
    """
    # Plot frontier size by move number for each az/edax version combination 
    
    frontier_query = "WITH az_games AS (SELECT g.game_id, CASE WHEN g.black = 'az' THEN CAST(g.black_version AS INT) ELSE CAST(g.white_version AS INT) END AS az_version, CASE WHEN g.black = 'az' THEN 'black' ELSE 'white' END AS az_colour, CASE WHEN g.black = 'edax' THEN CAST(g.black_version AS INT) ELSE CAST(g.white_version AS INT) END AS edax_version FROM games g WHERE (g.black = 'az' AND g.white = 'edax') OR (g.black = 'edax' AND g.white = 'az')) " + "SELECT a.az_version, a.az_colour, a.edax_version, m.num_discs, AVG(CASE WHEN a.az_colour = 'black' THEN m.num_frontier_black ELSE m.num_frontier_white END) AS avg_az_frontier, AVG(CASE WHEN a.az_colour = 'black' THEN m.num_frontier_white ELSE m.num_frontier_black END) AS avg_edax_frontier, COUNT(*) AS n FROM az_games a JOIN moves m ON m.game_id = a.game_id WHERE a.edax_version IN (6, 15, 21) GROUP BY 1,2,3,4 ORDER BY 1,2,3,4;"

    frontier_columns = ["az_version", "az_colour", "edax_version", "num_discs", "avg_az_frontier", "avg_edax_frontier", "n"]

    frontier = sql_to_dataframe(conn, frontier_query, frontier_columns)

    prepare_output_dir(FRONTIER_DIR)
    plot_avg_frontier_by_discs(frontier, FRONTIER_DIR)
    """
    
    forced_corner_query = (
        "WITH az_games AS ("
        "SELECT g.game_id, "
        "CASE WHEN g.black = 'az' THEN CAST(g.black_version AS INT) ELSE CAST(g.white_version AS INT) END AS az_version, "
        "CASE WHEN g.black = 'az' THEN 'black' ELSE 'white' END AS az_colour, "
        "CASE WHEN g.black = 'edax' THEN CAST(g.black_version AS INT) ELSE CAST(g.white_version AS INT) END AS edax_version "
        "FROM games g "
        "WHERE (g.black = 'az' AND g.white = 'edax') OR (g.black = 'edax' AND g.white = 'az')"
        "), az_game_flags AS ("
        "SELECT a.az_version, a.edax_version, a.game_id, "
        "MAX(CASE "
        "WHEN (((a.az_colour = 'black') AND (m.ply % 2 = 1)) OR ((a.az_colour = 'white') AND (m.ply % 2 = 0))) "
        "AND m.forced_corner_cap THEN 1 ELSE 0 END) AS az_forced_corner_game "
        "FROM az_games a "
        "JOIN moves m ON m.game_id = a.game_id "
        "WHERE a.edax_version IN (6, 15, 21) "
        "GROUP BY 1,2,3"
        ") "
        "SELECT az_version, edax_version, AVG(az_forced_corner_game::float) AS forced_corner_capture_rate, COUNT(*) AS games "
        "FROM az_game_flags "
        "GROUP BY 1,2 "
        "ORDER BY 2,1;"
    )
    forced_corner_columns = ["az_version", "edax_version", "forced_corner_capture_rate", "games"]
    forced_corner = sql_to_dataframe(conn, forced_corner_query, forced_corner_columns)

    prepare_output_dir(FORCED_CORNER_DIR)
    plot_forced_corner_capture_rate(forced_corner, FORCED_CORNER_DIR)

    avg_game_frontier_query = (
        "WITH az_games AS ("
        "SELECT g.game_id, "
        "CASE WHEN g.black = 'az' THEN CAST(g.black_version AS INT) ELSE CAST(g.white_version AS INT) END AS az_version, "
        "CASE WHEN g.black = 'az' THEN 'black' ELSE 'white' END AS az_colour, "
        "CASE WHEN g.black = 'edax' THEN CAST(g.black_version AS INT) ELSE CAST(g.white_version AS INT) END AS edax_version "
        "FROM games g "
        "WHERE (g.black = 'az' AND g.white = 'edax') OR (g.black = 'edax' AND g.white = 'az')"
        "), az_moves AS ("
        "SELECT a.az_version, a.az_colour, a.edax_version, a.game_id, "
        "CASE WHEN a.az_colour = 'black' THEN m.num_frontier_black ELSE m.num_frontier_white END AS az_frontier "
        "FROM az_games a "
        "JOIN moves m ON m.game_id = a.game_id "
        "WHERE a.edax_version IN (6, 15, 21) "
        "AND (((a.az_colour = 'black') AND (m.ply % 2 = 1)) OR ((a.az_colour = 'white') AND (m.ply % 2 = 0)))"
        "), per_game AS ("
        "SELECT az_version, az_colour, edax_version, game_id, AVG(az_frontier::float) AS game_avg_frontier "
        "FROM az_moves "
        "GROUP BY 1,2,3,4"
        ") "
        "SELECT az_version, az_colour, edax_version, AVG(game_avg_frontier) AS avg_game_frontier, COUNT(*) AS games "
        "FROM per_game "
        "GROUP BY 1,2,3 "
        "ORDER BY 2,3,1;"
    )
    avg_game_frontier_columns = ["az_version", "az_colour", "edax_version", "avg_game_frontier", "games"]
    avg_game_frontier = sql_to_dataframe(conn, avg_game_frontier_query, avg_game_frontier_columns)

    prepare_output_dir(AVG_GAME_FRONTIER_DIR)
    plot_avg_game_frontier_by_iteration(avg_game_frontier, AVG_GAME_FRONTIER_DIR)

except (Exception, psycopg2.DatabaseError) as error:
    print("Error: %s" % error)
