import os
import pandas as pd
import statsmodels.api as sm
from statsmodels.formula.api import ols
from itertools import product
import numpy as np

def calculate_omega_squared(anova_table):
    """
    Calculate omega squared (O_sq) effect sizes from ANOVA table.
    Omega squared is a less biased estimate than eta squared.
    
    O_sq = (SS_effect - df_effect * MS_error) / (SS_total + MS_error)
    
    Returns a dictionary with omega squared values for each factor.
    """
    # print("ANOVA table columns:", anova_table.columns.tolist())
    # print("ANOVA table index:", anova_table.index.tolist())
    print("ANOVA table:\n", anova_table)
    
    # Check for correct column names in statsmodels ANOVA output
    # Common column names: 'sum_sq', 'df', 'F', 'PR(>F)'
    if 'sum_sq' not in anova_table.columns:
        raise KeyError(f"Expected 'sum_sq' column not found. Available columns: {anova_table.columns.tolist()}")
    
    # Get residual/error row (typically labeled 'Residual')
    residual_idx = None
    for idx in anova_table.index:
        if 'Residual' in str(idx) or 'residual' in str(idx).lower():
            residual_idx = idx
            break
    
    if residual_idx is None:
        # If no explicit residual row, assume last row is residual
        residual_idx = anova_table.index[-1]
        print(f"Warning: No 'Residual' row found. Using last row '{residual_idx}' as residual.")
    
    # Calculate MS_error = SS_residual / df_residual
    ss_residual = anova_table.loc[residual_idx, 'sum_sq']
    df_residual = anova_table.loc[residual_idx, 'df']
    ms_error = ss_residual / df_residual
    
    ss_total = anova_table['sum_sq'].sum()
    
    omega_squared = {}
    
    # Calculate omega squared for each effect (excluding residual)
    for idx in anova_table.index:
        if idx == residual_idx:
            continue  # Skip residual row
            
        ss_effect = anova_table.loc[idx, 'sum_sq']
        df_effect = anova_table.loc[idx, 'df']
        
        omega_sq = (ss_effect - df_effect * ms_error) / (ss_total + ms_error)
        # Ensure omega squared is not negative (can happen with very small effects)
        omega_squared[idx] = max(0, omega_sq)
    
    return omega_squared

def print_anova_with_effect_sizes(anova_table, title="ANOVA Results"):
    """Print ANOVA table with omega squared effect sizes."""
    print(f"\n{title}")
    print("=" * len(title))
    print(anova_table)
    
    # Calculate and display effect sizes
    omega_sq = calculate_omega_squared(anova_table)
    
    print("\nOmega Squared (O_sq) Effect Sizes:")
    print("-" * 35)
    for factor, effect_size in omega_sq.items():
        # Interpret effect size magnitude
        if effect_size < 0.01:
            magnitude = "negligible"
        elif effect_size < 0.06:
            magnitude = "small"
        elif effect_size < 0.14:
            magnitude = "medium"
        else:
            magnitude = "large"
        
        print(f"{factor:30}: O_sq = {effect_size:.4f} ({magnitude})")

# Path to your extracted readings folder
extract_path = "readings"

# Store average wind speeds
avg_results = []

# Define expected combinations of width and obstacles
widths = ['open', 'half', 'closed']
obstacles = ['none', 'building', 'park']
expected_combos = set(product(widths, obstacles))

print("Loading and processing data...")
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
        width = parts[0]  # open | half | closed
        obstacle = "none" if len(parts) == 1 else "_".join(parts[1:])
        if obstacle == "building_park":
            continue  # we are removing this factor level entirely
        
        df = pd.read_csv(os.path.join(height_path, fname))
        
        # keep only needed column(s)
        if "wind_speed" not in df.columns:
            raise ValueError(f"{fname} missing 'wind_speed' column")
        
        tmp = df[["wind_speed"]].copy()
        tmp["height"] = height_folder    # aboveTreeLevel | treeLevel | groundLevel
        tmp["width"] = width             # open | half | closed
        tmp["obstacles"] = obstacle      # none | building | park
        tmp["file"] = f"{height_folder}/{fname}"  # identifier for clustering if needed
        records.append(tmp)

# Combine all data
data = pd.concat(records, ignore_index=True)
data = data.dropna(subset=["wind_speed"])

# Data quality checks
print("\n" + "="*50)
print("DATA QUALITY CHECKS")
print("="*50)

print("\n1) Dataset Info:")
print(data.info())
print("\n2) First few rows:")
print(data.head())

print("\n3) Sample sizes per experimental cell:")
cell_counts = data.groupby(["height", "width", "obstacles"]).size().unstack(["width", "obstacles"])
print(cell_counts)

print("\n4) Descriptive statistics per cell:")
cell_stats = data.groupby(["height", "width", "obstacles"])["wind_speed"].agg(["count", "mean", "std"])
print(cell_stats)

# Two-way ANOVAs for each height level
print("\n" + "="*50)
print("TWO-WAY ANOVA ANALYSES BY HEIGHT")
print("="*50)

for h in data["height"].unique():
    subset = data[data["height"] == h]
    
    # Fit OLS with interaction
    model = ols("wind_speed ~ C(width) * C(obstacles)", data=subset).fit()
    
    # Standard ANOVA table (Type II)
    anova_table = sm.stats.anova_lm(model, typ=2)
    
    print_anova_with_effect_sizes(anova_table, f"Two-Way ANOVA for {h}")

# Three-way ANOVA across all heights
print("\n" + "="*50)
print("THREE-WAY ANOVA ANALYSIS")
print("="*50)

model3 = ols("wind_speed ~ C(height) * C(width) * C(obstacles)", data=data).fit()
anova_table_3way = sm.stats.anova_lm(model3, typ=2)

print_anova_with_effect_sizes(anova_table_3way, "Three-Way ANOVA (All Heights)")

##############################################################################################
# Two-way ANOVAs: Park vs Buildings only
print("\n" + "="*50)
print("TWO-WAY ANOVAS: PARKS VS BUILDINGS ONLY")
print("="*50)

# Filter dataset: only park and building obstacles
tb_data = data[data["obstacles"].isin(["park", "building"])]


for h in tb_data["height"].unique():
    subset = tb_data[tb_data["height"] == h]
    
    # Model: Wind speed explained by Obstacle (park vs building), Width, and their interaction
    model_tb = ols("wind_speed ~ C(obstacles) * C(width)", data=subset).fit()
    anova_tb = sm.stats.anova_lm(model_tb, typ=2)
    
    print_anova_with_effect_sizes(anova_tb, f"Parks vs Buildings ANOVA at {h}")

# # Three-way ANOVA for parks vs buildings only
# print("\n" + "="*50)
# print("THREE-WAY ANOVA: PARKS VS BUILDINGS ONLY")
# print("="*50)

# model_tb3 = ols("wind_speed ~ C(height) * C(width) * C(obstacles)", data=tb_data).fit()
# anova_tb3 = sm.stats.anova_lm(model_tb3, typ=2)

# print_anova_with_effect_sizes(anova_tb3, "Three-Way ANOVA (Parks vs Buildings)")


# Additional summary statistics
print("\n" + "="*50)
print("SUMMARY STATISTICS")
print("="*50)

print(f"\nTotal observations: {len(data)}")
print(f"Heights analyzed: {sorted(data['height'].unique())}")
print(f"Width conditions: {sorted(data['width'].unique())}")
print(f"Obstacle conditions: {sorted(data['obstacles'].unique())}")

print(f"\nOverall wind speed statistics:")
print(f"Mean: {data['wind_speed'].mean():.3f}")
print(f"Std: {data['wind_speed'].std():.3f}")
print(f"Min: {data['wind_speed'].min():.3f}")
print(f"Max: {data['wind_speed'].max():.3f}")