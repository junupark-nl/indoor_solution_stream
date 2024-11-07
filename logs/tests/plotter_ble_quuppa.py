import os
import pandas as pd
import matplotlib.pyplot as plt
import ast
import argparse

arg_parser = argparse.ArgumentParser()
arg_parser.add_argument('directory', type=str, help='CSV file to plot')
args = arg_parser.parse_args()

def translate_mocap_data(x, y, z):
    return 6-x, z+5, y

# Read the CSV file
df_mocap = pd.read_csv(os.path.join(os.getcwd(), args.directory, 'rigid_body_sentinel.csv'))
df_ble = pd.read_csv(os.path.join(os.getcwd(), args.directory, 'ble.csv'))

df_mocap['X'], df_mocap['Y'], df_mocap['Z'] = translate_mocap_data(df_mocap['X'], df_mocap['Y'], df_mocap['Z'])
    
# Assuming the first column is for x-axis and the rest are for y-axis
t_mocap = df_mocap['ArrivalTimeUs'] / 1e6
t_ble = df_ble['ArrivalTimeUs'] / 1e6

# Convert the location string to a list and extract x, y, z coordinates, and append them to the DataFrame
df_ble[['x', 'y', 'z']] = df_ble['location'].apply(ast.literal_eval).tolist()
df_ble['tag'] = df_ble['tagName'].str.extract('(\d+)$').astype(int)

# Separate data into different DataFrames based on tag number
tag_dataframes = {}
tags = sorted(df_ble['tag'].unique())
for tag in tags:
    tag_data = df_ble[df_ble['tag'] == tag]
    
    # Create separate DataFrame, each tag
    tag_dataframes[tag] = {
        'x': tag_data[['ArrivalTimeUs', 'x']].copy().rename(columns={'x': 'value'}),
        'y': tag_data[['ArrivalTimeUs', 'y']].copy().rename(columns={'y': 'value'}),
        'z': tag_data[['ArrivalTimeUs', 'z']].copy().rename(columns={'z': 'value'})
    }

# Create the plot
coordinates = ['X', 'Y', 'Z']
fig, axes = plt.subplots(len(tags), 1, figsize=(12, 8))
fig.suptitle('Location vs. Time')

for i, axis in enumerate(axes):
    # Plot the mocap data
    axis.plot(t_mocap, df_mocap[coordinates[i]], label=f'{coordinates[i]}(mocap)')
    # Plot the BLE data, differentiating by tag number
    for tag in tags:
        data = tag_dataframes[tag][coordinates[i].lower()]
        axis.plot(data['ArrivalTimeUs']/ 1e6, data['value'], label=f'{coordinates[i]}(ble, tag {tag})')
    # Set the title, labels, and legend
    axis.set_title(f'{coordinates[i]} position')
    axis.set_xlabel('Time (s)')
    axis.grid(True)
    axis.legend()

plt.tight_layout()
plt.show()