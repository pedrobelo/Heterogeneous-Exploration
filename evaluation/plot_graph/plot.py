import matplotlib.pyplot as plt
import csv
import numpy as np

uncertainty1 = []
average1 = []
x1 = []

uncertainty2 = []
average2 = []
x2 = []

histogram_f1 = []
histogram_o1 = []
histogram_f2 = []
histogram_o2 = []

with open('rooms_he_1.txt','r') as csvfile:
    plots = csv.reader(csvfile, delimiter=' ')
    for row in plots:
        x1.append(float(row[0]))
        uncertainty1.append(float(row[1]))
        average1.append(float(row[2]))

        values = row[3:];
        histogram_f1.append([float(i) for i in values[0:len(values)/2]])
        histogram_o1.append([float(i) for i in values[len(values)/2:len(values)+1]])

with open('rooms_aep_1.txt','r') as csvfile:
    plots = csv.reader(csvfile, delimiter=' ')
    for row in plots:
        x2.append(float(row[0]))
        uncertainty2.append(float(row[1]))
        average2.append(float(row[2]))

        values = row[3:];
        histogram_f2.append([float(i) for i in values[0:len(values)/2]])
        histogram_o2.append([float(i) for i in values[len(values)/2:len(values)+1]])

aux = x1[0]
x1[:] = [x - aux for x in x1]

aux = x2[0]
x2[:] = [x - aux for x in x2]

uncertainty1 = [x for _,x in sorted(zip(x1,uncertainty1))]
average1 = [x for _,x in sorted(zip(x1,average1))]
histogram_f1 = [x for _,x in sorted(zip(x1,histogram_f1))]
histogram_o1 = [x for _,x in sorted(zip(x1,histogram_o1))]
x1.sort()

uncertainty2 = [x for _,x in sorted(zip(x2,uncertainty2))]
average2 = [x for _,x in sorted(zip(x2,average2))]
histogram_f2 = [x for _,x in sorted(zip(x2,histogram_f2))]
histogram_o2 = [x for _,x in sorted(zip(x2,histogram_o2))]
x2.sort()

plt.subplot(3,2,1)
[a,b] = plt.plot(x1, uncertainty1, 'r', x2, uncertainty2, 'b')
plt.xlabel('Time')
plt.ylabel('Total uncertainty')
plt.title('Total uncertainty over time')
plt.legend([a,b], ["improved exploration", "normal exploration"])

plt.subplot(3,2,2)
[a,b] = plt.plot(x1, average1, 'r', x2, average2, 'b')
plt.xlabel('Time')
plt.ylabel('Average uncertainty')
plt.title('Average uncertainty over time')
plt.legend([a,b], ["improved exploration", "normal exploration"])

num_plots = len(values)/2
colors = [plt.cm.jet(i) for i in np.linspace(0, 1, num_plots)]

plt.rc('axes', prop_cycle=plt.cycler('color', colors))

plt.subplot(3,2,3)
labels = []
data = np.array(list(histogram_f1))
for indx in range(0, num_plots):
    plt.plot(x1, data.transpose()[indx])
    labels.append(r'$%.2f - %.2f$' % (indx*0.5/num_plots, (indx+1)*0.5/num_plots))
plt.xlabel('Time')
plt.ylabel('Number of cells')
plt.title('Number of free cells in uncertainty interval for improved exploration')
plt.legend(labels)

plt.subplot(3,2,4)
labels = []
data = np.array(list(histogram_o1))
for indx in range(0, num_plots):
    plt.plot(x1, data.transpose()[indx])
    labels.append(r'$%.2f - %.2f$' % (indx*0.5/num_plots+0.5, (indx+1)*0.5/num_plots+0.5))
plt.xlabel('Time')
plt.ylabel('Number of cells')
plt.title('Number of occupied cells in uncertainty interval for improved exploration')
plt.legend(labels)

plt.subplot(3,2,5)
labels = []
data = np.array(list(histogram_f2))
for indx in range(0, num_plots):
    plt.plot(x2, data.transpose()[indx])
    labels.append(r'$%.2f - %.2f$' % (indx*0.5/num_plots, (indx+1)*0.5/num_plots))
plt.xlabel('Time')
plt.ylabel('Number of cells')
plt.title('Number of free cells in uncertainty interval for normal exploration')
plt.legend(labels)

plt.subplot(3,2,6)
labels = []
data = np.array(list(histogram_o2))
for indx in range(0, num_plots):
    plt.plot(x2, data.transpose()[indx])
    labels.append(r'$%.2f - %.2f$' % (indx*0.5/num_plots+0.5, (indx+1)*0.5/num_plots+0.5))
plt.xlabel('Time')
plt.ylabel('Number of cells')
plt.title('Number of occupied cells in uncertainty interval for normal exploration')
plt.legend(labels)

plt.show()
