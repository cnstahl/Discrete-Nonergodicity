import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

def autocorrelation(L,l):
	string = '../autoc/N_flippable_'
	string += 'L' + str(L) + 'l' + str(l) + '.dat'
	rhos = pd.read_csv(string, header=None, skiprows=1).to_numpy() / (L*L)
	N_runs = np.shape(rhos)[0]
	N_steps = np.shape(rhos)[1]
	interval = np.shape(rhos)[1] - 1
	result = np.zeros(2*(interval)+1)

	for rho in rhos:
	    x = rho - np.mean(rho)
	    result += fft_correl(x, x)
	data = result[interval:]/len(rhos)/np.arange(1+interval,0,-1)
	data_max = np.max(data[int(N_steps/4):int(N_steps/2)])
	data_min = np.min(data[int(N_steps/4):int(N_steps/2)])
	data_rge = data_max-data_min

	plt.title('autoc: L='+ str(L) + ', l=' + str(l) + ', ' + str(N_runs) + " runs, each of " + str(N_steps) + " steps")
	plt.plot(data)
	plt.ylim([data_min-data_rge, data_max+data_rge])
	plt.savefig('../img/N_flippable_autoc_L' + str(L) + 'l' + str(l))
	plt.clf()

for L in (6, 12, 18, 24, 36):
	for l in (0,6):
		autocorrelation(L,l)