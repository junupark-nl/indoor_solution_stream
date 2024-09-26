import os
import pandas as pd
import matplotlib.pyplot as plt
import ast
import argparse

arg_parser = argparse.ArgumentParser()
arg_parser.add_argument('directory', type=str, help='CSV file to plot')
args = arg_parser.parse_args()

# Read the CSV file
df_mocap = pd.read_csv(os.path.join(os.getcwd(), args.directory, 'rigid_body_calibration_bar.csv'))
df_uwb = pd.read_csv(os.path.join(os.getcwd(), args.directory, 'uwb.csv'))

t_mocap = df_mocap['ArrivalTimeUs'] / 1e6
t_uwb = df_uwb['ArrivalTimeUs'] / 1e6

fig, (ax1, ax2, ax3) = plt.subplots(3,1, figsize=(12, 8))
fig.suptitle('Location vs. Time')

ax1.plot(t_mocap, 6-df_mocap['X'], label='X(mocap)')
ax1.plot(t_uwb, df_uwb['X'], label=f'X(UWB)')

ax2.plot(t_mocap, df_mocap['Z']+5, label='Y(mocap)')
ax2.plot(t_uwb, df_uwb['Y'], label=f'Y(UWB)')

ax3.plot(t_mocap, df_mocap['Y'], label='Height')
ax3.plot(t_uwb, df_uwb['Z'], label=f'Z(UWB)')

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

plt.tight_layout()
plt.show()