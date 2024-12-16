import numpy as np

values = np.loadtxt("input", dtype=np.int32).T
values[0].sort()
values[1].sort()
print(np.abs(values[0] - values[1]).sum())
