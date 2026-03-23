from sqlalchemy import create_engine, insert, Table, MetaData
import numpy as np
import pandas as pd 
from sklearn.linear_model import LassoCV
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
import sys

# X is an array of activations (inputs), y is an array of the concept values to predict for each activation 
def fit_linear_model(X, y): 
    model = LassoCV(cv=5).fit(X, y)
    return model

def test_linear_model(model, X, y):
    return model.score(X, y)


def format_blocks(activations):
    for i in range(1, 4):
        activations[f"block_{i}"] = activations[f"block_{i}"].apply(lambda x: np.frombuffer(b"".join(x), dtype=np.float32))

        activations[f"block_{i}_control"] = activations[f"block_{i}_control"].apply(lambda x: np.frombuffer(b"".join(x), dtype=np.float32))
 
    return activations

if __name__ == "__main__": 
    engine = create_engine(
    "postgresql+psycopg2://postgres:postgres@localhost:5432/ug4"
    )
    results_table = Table("results", MetaData(), autoload_with=engine)

    iteration = sys.argv[1]
    table = sys.argv[2]
    concept = sys.argv[3]

    activations = pd.read_sql(f"SELECT * FROM activations WHERE iteration='{iteration}'", engine)
    label = pd.read_sql(f"SELECT * FROM {table}", engine)

    activations = format_blocks(activations)

    # Merge activations and label datasets
    df = activations.merge(label, on="position_id")
    
    # Random_state ensures consistent training/test sets between runs given the same input 
    scaler = StandardScaler()
    res = {} 
    for i in range(1,4):
        X = np.stack(df[f"block_{i}"].values)
        X_control = np.stack(df[f"block_{i}_control"].values)
        y = df[concept]

        indices = np.arange(len(X))
        train_idx, test_idx = train_test_split(indices, random_state=104, test_size=0.25, shuffle=True)

        X_train, X_test = X[train_idx], X[test_idx]
        X_control_train, X_control_test = X_control[train_idx], X_control[test_idx]
        y_train, y_test = y.iloc[train_idx], y.iloc[test_idx]

        X_train_scaled = scaler.fit_transform(X_train)
        X_test_scaled = scaler.transform(X_test)

        X_control_train_scaled = scaler.fit_transform(X_control_train)
        X_control_test_scaled = scaler.transform(X_control_test)

        model = fit_linear_model(X_train_scaled, y_train)
        model_control = fit_linear_model(X_control_train_scaled, y_train)

        score = test_linear_model(model, X_test_scaled, y_test)
        score_control = test_linear_model(model_control, X_control_test_scaled, y_test)

        res[f"block_{i}_coeff"] = score
        res[f"block_{i}_control_coeff"] = score_control

    insert_stmt = insert(results_table).values(iteration=iteration, block_1_coeff=res["block_1_coeff"], block_2_coeff=res["block_2_coeff"], block_3_coeff=res["block_3_coeff"], block_1_control_coeff=res["block_1_control_coeff"], block_2_control_coeff=res["block_2_control_coeff"], block_3_control_coeff=res["block_3_control_coeff"], concept=concept)

    with engine.begin() as conn:
        result = conn.execute(insert_stmt)

    

    

    