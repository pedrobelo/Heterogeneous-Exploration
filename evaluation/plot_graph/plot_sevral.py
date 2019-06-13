import matplotlib.pyplot as plt
import csv
import numpy as np
import math
import sys

class graphs(object):
	count = []
	uncertainty = []
	average = []
	histogram_f = []
	histogram_o = []
	tSize = 0
	pSize = 0

	def __init__(self, tSize_, pSize_):
		self.tSize = tSize_
		self.pSize = pSize_
		self.count = [0] * self.tSize

	def add(self, values, time):
		self.histogram_f[int(math.floor(time))]

		self.histogram_f[int(math.floor(time))]= [x+float(y) for x, y in zip(self.histogram_f[int(math.floor(time))], values[0:len(values)/2])]
		self.histogram_o[int(math.floor(time))]= [x+float(y) for x, y in zip(self.histogram_o[int(math.floor(time))], values[len(values)/2:len(values)+1])]

	def readFile(self, str):
		self.count = [0] * self.tSize
		self.uncertainty = [0] * self.tSize
		self.average = [0] * self.tSize
		self.count = [0] * self.tSize

		self.histogram_f = []
		self.histogram_o = []
		for i in range(0, self.tSize):
			self.histogram_f.append([0] * self.pSize)
			self.histogram_o.append([0] * self.pSize)

		with open(str,'r') as csvfile:
			plots_ = csv.reader(csvfile, delimiter=' ')
			plots = []

			for row in plots_:
				plots.append(row)
			min_ = min(float(row[0]) for row in plots)

			for row in plots:
				if(int(math.floor(float(row[0])-min_)) < self.tSize):
					self.uncertainty[int(math.floor(float(row[0])-min_))] += float(row[1])
					self.average[int(math.floor(float(row[0])-min_))] += float(row[2])

					values = row[3:]
					self.add(values, float(row[0])-min_)

					self.count[int(math.floor(float(row[0])-min_))] += 1

	def get_uncertainty(self):
		ret = []
		for x, y in zip(self.uncertainty, self.count):
			if y == 0:
				ret.append(-1)
			else:
				ret.append(x/y)
		
		return ret

	def get_average(self):
		ret = []
		for x, y in zip(self.average, self.count):
			if y == 0:
				ret.append(-1)
			else:
				ret.append(x/y)
		
		return ret

	def get_histogram_f(self):
		ret2 = []
		for x in range(0,self.tSize):
			ret = []
			for y in self.histogram_f[x]:
				if self.count[x] == 0:
					ret.append(-1)
				else:
					ret.append(y/self.count[x])
			ret2.append(ret)
		return ret2

	def get_histogram_o(self):
		ret2 = []
		for x in range(0,self.tSize):
			ret = []
			for y in self.histogram_o[x]:
				if self.count[x] == 0:
					ret.append(-1)
				else:
					ret.append(y/self.count[x])
			ret2.append(ret)

		return ret2

class mergeGraphs(graphs):
	init = True

	def add(self, graph):
		uncertainty_aux = graph.get_uncertainty()
		average_aux = graph.get_average()
		histogram_f_aux = graph.get_histogram_f()
		histogram_o_aux = graph.get_histogram_o()
		for i in range(0,self.tSize):
			self.uncertainty[i] += uncertainty_aux[i]
			self.average[i] += average_aux[i]
			self.count[i] += 1
			for j in range(0,self.pSize):
				self.histogram_f[i][j] += histogram_f_aux[i][j]
				self.histogram_o[i][j] += histogram_o_aux[i][j]

	def addGraph(self, graph):
		if self.init == True:
			self.uncertainty = graph.get_uncertainty()
			self.average = graph.get_average()
			self.histogram_f = graph.get_histogram_f()
			self.histogram_o = graph.get_histogram_o()
			self.init = False
			for i in range(0,self.tSize):
				self.count[i] = 1
		else:
			self.add(graph)
						

def plotGraphs(time, heGraph, aepGraph):
	heUncertainty = heGraph.get_uncertainty()
	aepUncertainty = aepGraph.get_uncertainty()
	heAverage = heGraph.get_average()
	aepAverage = aepGraph.get_average()
	heHistogram_o = heGraph.get_histogram_o()
	aepHistogram_o = aepGraph.get_histogram_o()
	heHistogram_f = heGraph.get_histogram_f()
	aepHistogram_f = aepGraph.get_histogram_f()

	plt.subplot(3,2,1)
	[a,b] = plt.plot(time, heUncertainty, 'r', time, aepUncertainty, 'b')
	plt.xlabel('Time')
	plt.ylabel('Total uncertainty')
	plt.title('Total uncertainty over time')
	plt.legend([a,b], ["improved exploration", "normal exploration"])

	plt.subplot(3,2,2)
	[a,b] = plt.plot(time, heAverage, 'r', time, aepAverage, 'b')
	plt.xlabel('Time')
	plt.ylabel('Average uncertainty')
	plt.title('Average uncertainty over time')
	plt.legend([a,b], ["improved exploration", "normal exploration"])

	num_plots = 5
	colors = [plt.cm.jet(i) for i in np.linspace(0, 1, num_plots)]

	plt.rc('axes', prop_cycle=plt.cycler('color', colors))

	plt.subplot(3,2,3)
	labels = []
	data = np.array(list(heHistogram_f))

	for indx in range(0, num_plots):
	    plt.plot(time, data.transpose()[indx])
	    labels.append(r'$%.2f - %.2f$' % (indx*0.5/num_plots, (indx+1)*0.5/num_plots))
	plt.xlabel('Time')
	plt.ylabel('Number of cells')
	plt.title('Number of free cells in uncertainty interval for improved exploration')
	plt.legend(labels)

	plt.subplot(3,2,4)
	labels = []
	data = np.array(list(heHistogram_o))
	for indx in range(0, num_plots):
	    plt.plot(time, data.transpose()[indx])
	    labels.append(r'$%.2f - %.2f$' % (indx*0.5/num_plots+0.5, (indx+1)*0.5/num_plots+0.5))
	plt.xlabel('Time')
	plt.ylabel('Number of cells')
	plt.title('Number of occupied cells in uncertainty interval for improved exploration')
	plt.legend(labels)

	plt.subplot(3,2,5)
	labels = []
	data = np.array(list(aepHistogram_f))
	for indx in range(0, num_plots):
	    plt.plot(time, data.transpose()[indx])
	    labels.append(r'$%.2f - %.2f$' % (indx*0.5/num_plots, (indx+1)*0.5/num_plots))
	plt.xlabel('Time')
	plt.ylabel('Number of cells')
	plt.title('Number of free cells in uncertainty interval for normal exploration')
	plt.legend(labels)

	plt.subplot(3,2,6)
	labels = []
	data = np.array(list(aepHistogram_o))
	for indx in range(0, num_plots):
	    plt.plot(time, data.transpose()[indx])
	    labels.append(r'$%.2f - %.2f$' % (indx*0.5/num_plots+0.5, (indx+1)*0.5/num_plots+0.5))
	plt.xlabel('Time')
	plt.ylabel('Number of cells')
	plt.title('Number of occupied cells in uncertainty interval for normal exploration')
	plt.legend(labels)

	plt.show()


def printGraphs(startFile, endFile):
	graph = graphs(250, 5)
	heMergeGraphs = mergeGraphs(250, 5)
	aepMergeGraphs = mergeGraphs(250, 5)

	for x in range(startFile,endFile+1):
		print x
		graph.readFile('he_'+str(x)+'.txt')
		heMergeGraphs.addGraph(graph)
		graph.readFile('aep_'+str(x)+'.txt')
		aepMergeGraphs.addGraph(graph)

	plotGraphs([i for i in range(0, 250)], heMergeGraphs, aepMergeGraphs)



printGraphs(int(sys.argv[1]),int(sys.argv[2]))
