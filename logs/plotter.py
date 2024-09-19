import os
import pandas as pd
import matplotlib.pyplot as plt
import ast
import argparse

arg_parser = argparse.ArgumentParser()
arg_parser.add_argument('directory', type=str, help='CSV file to plot')
args = arg_parser.parse_args()

# Read the CSV file
df_mocap = pd.read_csv(os.path.join(os.getcwd(), args.directory, 'rigid_body_sentinel.csv'))
df_ble = pd.read_csv(os.path.join(os.getcwd(), args.directory, 'ble.csv'))

# Assuming the first column is for x-axis and the rest are for y-axis
t_mocap = df_mocap['ArrivalTimeUs'] / 1e6
t_ble = df_ble['ArrivalTimeUs'] / 1e6

tag_dataframes = {}
df_ble[['x', 'y', 'z']] = df_ble['location'].apply(ast.literal_eval).tolist()

df_ble['tag_number'] = df_ble['tagName'].str.extract('(\d+)$').astype(int)
# Separate data into different DataFrames based on tag number
for tag_num in df_ble['tag_number'].unique():
    tag_data = df_ble[df_ble['tag_number'] == tag_num]
    
    # Create separate DataFrames for x and y coordinates
    tag_dataframes[tag_num] = {
        'x': tag_data[['ArrivalTimeUs', 'x']].copy(),
        'y': tag_data[['ArrivalTimeUs', 'y']].copy()
    }

# Convert the location string to a list and extract x, y, z coordinates
df_ble[['x', 'y', 'z']] = df_ble['location'].apply(ast.literal_eval).tolist()

# Create the plot
fig, (ax1, ax2, ax3) = plt.subplots(3,1, figsize=(12, 8))
fig.suptitle('Location vs. Time')

ax1.plot(t_mocap, 6-df_mocap['X'], label='X(mocap)')
for i in range(3):
    tag_num = i+1
    x_data = tag_dataframes[tag_num]['x']
    ax1.plot(x_data['ArrivalTimeUs']/ 1e6, x_data['x'], label=f'X(ble, tag {tag_num})')


ax2.plot(t_mocap, df_mocap['Z']+5, label='Y(mocap)')
for i in range(3):
    tag_num = i+1
    y_data = tag_dataframes[tag_num]['y']
    ax2.plot(y_data['ArrivalTimeUs']/ 1e6, y_data['y'], label=f'Y(ble, tag {tag_num})')

ax3.plot(t_mocap, df_mocap['Y'], label='Height')

ax1.set_ylabel('X position')
ax2.set_ylabel('Y position')
ax1.set_xlabel('Time (s)')
ax2.set_xlabel('Time (s)')
ax3.set_ylabel('Z position')
ax3.set_xlabel('Time (s)')
ax1.grid(True)
ax2.grid(True)
ax3.grid(True)
ax1.legend()
ax2.legend()
ax3.legend()


print(tag_dataframes[2]['x']['ArrivalTimeUs']/ 1e6)


plt.tight_layout()
plt.show()