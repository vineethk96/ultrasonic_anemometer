import pandas as pd
import os
import glob

def combine_wind_speed_csvs():
    """
    Combines multiple CSV files containing wind speed data into a single summary file.
    Each CSV should have columns: timestamp, topic, wind_speed, wind_direction
    """
    
    # Get directory path from user
    directory_path = input("Enter the directory path containing the CSV files: ").strip()
    
    # Check if directory exists
    if not os.path.exists(directory_path):
        print(f"Error: Directory '{directory_path}' does not exist.")
        return
    
    # Find all CSV files in the directory
    csv_pattern = os.path.join(directory_path, "*.csv")
    csv_files = glob.glob(csv_pattern)
    
    if not csv_files:
        print(f"No CSV files found in directory '{directory_path}'")
        return
    
    print(f"Found {len(csv_files)} CSV files:")
    for file in csv_files:
        print(f"  - {os.path.basename(file)}")
    
    # Dictionary to store wind speed data from each file
    combined_data = {}
    
    # Process each CSV file
    for file_path in csv_files:
        try:
            # Read the CSV file
            df = pd.read_csv(file_path)
            
            # Verify required columns exist
            required_columns = ['timestamp', 'topic', 'wind_speed', 'wind_direction']
            if not all(col in df.columns for col in required_columns):
                print(f"Warning: {os.path.basename(file_path)} missing required columns. Skipping.")
                continue
            
            # Extract filename without extension
            filename = os.path.splitext(os.path.basename(file_path))[0]
            
            # Create new column name
            column_name = f"{filename}_wind_speed"
            
            # Store wind speed data
            combined_data[column_name] = df['wind_speed'].tolist()
            
            print(f"Processed: {os.path.basename(file_path)} -> {column_name}")
            
        except Exception as e:
            print(f"Error processing {os.path.basename(file_path)}: {str(e)}")
            continue
    
    if not combined_data:
        print("No valid data was processed.")
        return
    
    # Create the combined DataFrame
    summary_df = pd.DataFrame(combined_data)
    
    # Add index column (1-based)
    summary_df.insert(0, 'index', range(1, len(summary_df) + 1))
    
    # Create output file path
    output_path = os.path.join(directory_path, "wind_speed_summary.csv")
    
    # Save to CSV
    summary_df.to_csv(output_path, index=False)
    
    print(f"\nSummary complete!")
    print(f"Combined data from {len(combined_data)} files")
    print(f"Output saved to: {output_path}")
    print(f"Summary contains {len(summary_df)} rows and {len(summary_df.columns)} columns")
    
    # Display first few rows as preview
    print("\nPreview of combined data:")
    print(summary_df.head())

if __name__ == "__main__":
    combine_wind_speed_csvs()