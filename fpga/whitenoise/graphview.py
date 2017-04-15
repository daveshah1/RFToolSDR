
import matplotlib.pyplot as plt
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
    for i in range(0, len(titles)):
        values.append([])
    for line in lines[1:]:
        splitLine = line.split(",")
        for i in range(1, len(titles)+1):
            values[i-1].append(float(splitLine[i]) + ((i - 1) * spacing))
        times.append(float(splitLine[0]))
    for i in range(0, len(titles)):
        plt.plot(times, values[i], label=titles[i])

    plt.gca().axes.get_yaxis().set_ticks([])
    plt.legend(bbox_to_anchor=(1.02, 1), loc=2, borderaxespad=0.)
    plt.show()
