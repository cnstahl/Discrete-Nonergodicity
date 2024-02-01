import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

L = 6
l = 4
string = '../therm/N_flippable_'
string += 'L' + str(L) + 'l' + str(l) + '.dat'
rhos = pd.read_csv(string, header=None, skiprows=1).to_numpy() / (L*L)
N_runs = np.shape(rhos)[0]
N_steps = np.shape(rhos)[1]
data = np.average(rhos, axis=0)
data_max = np.max(data[int(N_steps/2):])
data_min = np.min(data[int(N_steps/2):])
data_rge = data_max-data_min

plt.title('L='+ str(L) + ', l=' + str(l) + ', ' + str(N_runs) + " runs, each of " + str(N_steps) + " steps")
plt.plot(data)
plt.ylim([data_min-data_rge, data_max+data_rge])
plt.savefig('../img/N_flippable_L' + str(L) + 'l' + str(l))