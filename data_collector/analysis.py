import os
import pandas as pd
import statsmodels.api as sm
from statsmodels.formula.api import ols
from itertools import product

# Path to your extracted readings folder
extract_path = "readings"

# Store average wind speeds
avg_results = []

# Define expected combinations of width and obstacles
widths = ['open', 'half', 'closed']
obstacles = ['none', 'building', 'park']
expected_combos = set(product(widths, obstacles))

import os
import pandas as pd
import statsmodels.api as sm
from statsmodels.formula.api import ols

# Path to your extracted readings folder
extract_path = "readings"

records = []
for height_folder in os.listdir(extract_path):
    print(f"Processing height folder: {height_folder}")
    height_path = os.path.join(extract_path, height_folder)
    if not os.path.isdir(height_path):
        continue
    for fname in os.listdir(height_path):
        if not fname.endswith(".csv"):
            continue

        print(f"  Processing file: {fname}")

        parts = fname[:-4].split("_")
        width = parts[0]                      # open | half | closed
        obstacle = "none" if len(parts) == 1 else "_".join(parts[1:])
        if obstacle == "building_park":
            continue  # we are removing this factor level entirely

        df = pd.read_csv(os.path.join(height_path, fname))

        # keep only needed column(s)
        if "wind_speed" not in df.columns:
            raise ValueError(f"{fname} missing 'wind_speed' column")

        tmp = df[["wind_speed"]].copy()
        tmp["height"] = height_folder        # aboveTreeLevel | treeLevel | groundLevel
        tmp["width"] = width                 # open | half | closed
        tmp["obstacles"] = obstacle          # none | building | park
        tmp["file"] = f"{height_folder}/{fname}"  # identifier for clustering if needed
        records.append(tmp)

data = pd.concat(records, ignore_index=True)
data = data.dropna(subset=["wind_speed"])

# 1) Make sure there are no NaNs and plenty of rows
print(data.info())
print(data.head())

# 2) Confirm replication counts per cell (should be >> 1)
print(data.groupby(["height","width","obstacles"]).size().unstack(["width","obstacles"]))

# 3) Quick descriptive stats per cell
cell_stats = data.groupby(["height","width","obstacles"])["wind_speed"].agg(["count","mean","std"])
print(cell_stats)

for h in data["height"].unique():
    print(f"\n=== TWO-WAY ANOVA for {h} ===")
    subset = data[data["height"] == h]

    # Fit OLS with interaction
    model = ols("wind_speed ~ C(width) * C(obstacles)", data=subset).fit()

    # Standard ANOVA table (Type II is fine here)
    anova_table = sm.stats.anova_lm(model, typ=2)
    print(anova_table)


# 4) Run a three-way ANOVA across all heights
print("\n=== THREE-WAY ANOVA ===")
# Fit OLS with interaction for all heights
model3 = ols("wind_speed ~ C(height) * C(width) * C(obstacles)", data=data).fit()
print(sm.stats.anova_lm(model3, typ=2))
