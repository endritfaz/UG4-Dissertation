from sqlalchemy import create_engine, insert, Table, MetaData
import numpy as np
import pandas as pd 
from sklearn.linear_model import LassoCV
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler

# X is an array of activations (inputs), y is an array of the concept values to predict for each activation 
def fit_linear_model(X, y): 
    model = LassoCV(cv=5).fit(X, y)
    return model

def test_linear_model(model, X, y):
    return model.score(X, y)


def format_blocks(activations):
    activations["block_1"] = activations["block_1"].apply(lambda x: np.frombuffer(b"".join(x), dtype=np.float32))
    activations["block_2"] = activations["block_2"].apply(lambda x: np.frombuffer(b"".join(x), dtype=np.float32))
    activations["block_3"] = activations["block_3"].apply(lambda x: np.frombuffer(b"".join(x), dtype=np.float32))

    return activations

if __name__ == "__main__": 
    engine = create_engine(
    "postgresql+psycopg2://postgres:postgres@localhost:5432/ug4"
    )
    results_table = Table("results", MetaData(), autoload_with=engine)

    iteration = "0"
    activations = pd.read_sql(f"SELECT * FROM activations WHERE iteration='{iteration}'", engine)
    stable_total = pd.read_sql("SELECT * FROM label_stable_discs", engine)

    activations = format_blocks(activations)

    # Merge activations and label datasets
    df = activations.merge(stable_total, on="position_id")
    
    # Random_state ensures consistent training/test sets between runs given the same input 
    scaler = StandardScaler()
    res = {} 
    for i in range(1,4):
        X = np.stack(df[f"block_{i}"].values)
        y = df["total_stable"]

        X_train, X_test, y_train, y_test = train_test_split(X, y, random_state=104,test_size=0.25, shuffle=True)

        X_train_scaled = scaler.fit_transform(X_train)
        X_test_scaled = scaler.transform(X_test)

        model = fit_linear_model(X_train_scaled, y_train)
        score = test_linear_model(model, X_test_scaled, y_test)

        res[f"block_{i}_coeff"] = score
    
    insert_stmt = insert(results_table).values(iteration=iteration, block_1_coeff=res["block_1_coeff"], block_2_coeff=res["block_2_coeff"], block_3_coeff=res["block_3_coeff"], concept="stable_total")

    with engine.begin() as conn:
        result = conn.execute(insert_stmt)

    

    

    