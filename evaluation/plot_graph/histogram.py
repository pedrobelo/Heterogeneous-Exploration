import matplotlib.pyplot as plt
import csv
import numpy as np

values = [];
with open('histogram.txt','r') as csvfile:
    plots = csv.reader(csvfile, delimiter=' ')
    for row in plots:
        values.append(float(row[0]))

plt.hist(values, bins=20)
plt.show()