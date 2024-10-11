import pyRAPL
import numpy as np
import cProfile
from codecarbon import track_emissions

@track_emissions(measure_power_secs=0.00000000005)
def test():
    for i in range(100):
        a = np.random.rand(100, 100)
        b = np.random.rand(100,100)
        c = np.dot(b,a.T)
        sorted = [np.argsort(j)[:10] for j in c.T]
        print(sorted)

test()
