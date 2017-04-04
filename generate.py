import numpy as np
import matplotlib.pyplot as plt
from matplotlib import colors as mcolors
import matplotlib.patches as mpatches

robotPath = np.genfromtxt('gt.txt')
obstaclesPath = np.genfromtxt('points.txt')
# print data
red_patch = mpatches.Patch(color='red', label='Robot')
green_patch = mpatches.Patch(color='green', label='Obstacle')
plt.style.use('plotConfig.mplstyle')
plt.legend(handles=[red_patch,green_patch])
plt.plot(robotPath[:, 0],robotPath[:, 1], 'ro', color = 'r', linewidth = 1)
plt.plot(obstaclesPath[:, 0],obstaclesPath[:, 1], 'ro', color = 'g', linewidth = 1)
plt.show()
