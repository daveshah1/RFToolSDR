
import matplotlib.pyplot as plt
import numpy as np
import sys
filename = sys.argv[1]

times = []
titles = []
values = []
spacing = 0

with open(filename) as f:
    lines = f.readlines()
    header = lines[0]
    splitHeader = header.split(",")
    titles = splitHeader[1:-1]
    for line in lines[1:]:
        splitLine = line.split(",")
        times.append(float(splitLine[0]))
        values.append(float(splitLine[1]) + 1.j * float(splitLine[2]))

    A = np.abs(np.fft.fft(values))
    plt.plot(A)
    plt.show()
