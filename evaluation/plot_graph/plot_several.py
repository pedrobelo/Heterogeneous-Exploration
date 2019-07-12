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
		last = -1
		for x, y in zip(self.uncertainty, self.count):
			if y == 0:
				ret.append(last)
			else:
				last = x/y
				ret.append(x/y)
		
		return ret

	def get_average(self):
		ret = []
		last = -1
		for x, y in zip(self.average, self.count):
			if y == 0:
				ret.append(last)
			else:
				last = x/y
				ret.append(x/y)
		
		return ret

	def get_histogram_f(self):
		ret2 = []

		last = []
		for i in range(self.pSize):
			last.append(-1)

		for x in range(0,self.tSize):
			ret = []
			for idx, y in enumerate(self.histogram_f[x]):
				if self.count[x] == 0:
					ret.append(last[idx])
				else:
					last[idx] = y/self.count[x]
					ret.append(y/self.count[x])
			ret2.append(ret)
		return ret2

	def get_histogram_o(self):
		ret2 = []
		
		last = []
		for i in range(self.pSize):
			last.append(-1)
			
		for x in range(0,self.tSize):
			ret = []
			for idx, y in enumerate(self.histogram_o[x]):
				if self.count[x] == 0:
					ret.append(last[idx])
				else:
					last[idx] = y/self.count[x]
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
						

def plotGraphs(time, heGraph, ibGraph, aepGraph):
	plt.figure()

	heUncertainty = heGraph.get_uncertainty()
	ibUncertainty = ibGraph.get_uncertainty()
	aepUncertainty = aepGraph.get_uncertainty()

	heAverage = heGraph.get_average()
	ibAverage = ibGraph.get_average()
	aepAverage = aepGraph.get_average()

	heHistogram_o = heGraph.get_histogram_o()
	ibHistogram_o = ibGraph.get_histogram_o()
	aepHistogram_o = aepGraph.get_histogram_o()

	heHistogram_f = heGraph.get_histogram_f()
	ibHistogram_f = ibGraph.get_histogram_f()
	aepHistogram_f = aepGraph.get_histogram_f()

	plt.subplot(4,2,1)
	[a,b,c] = plt.plot(time, heUncertainty, 'r', time, ibUncertainty, 'b', time, aepUncertainty, 'g')
	plt.xlabel('Time')
	plt.ylabel('Total uncertainty')
	plt.title('Total uncertainty over time')
	plt.legend([a,b,c], ["HE", "UHE", "AEP"])

	plt.subplot(4,2,2)
	[a,b,c] = plt.plot(time, heAverage, 'r', time, ibAverage, 'b', time, aepAverage, 'g')
	plt.xlabel('Time')
	plt.ylabel('Average uncertainty')
	plt.title('Average uncertainty over time')
	plt.legend([a,b,c], ["HE", "UHE", "AEP"])

	num_plots = 5
	colors = [plt.cm.jet(i) for i in np.linspace(0, 1, num_plots)]

	plt.rc('axes', prop_cycle=plt.cycler('color', colors))



	plt.subplot(4,2,3)
	labels = []
	data = np.array(list(heHistogram_f))
	for indx in range(0, num_plots):
	    plt.plot(time, data.transpose()[indx])
	    labels.append(r'$%.2f - %.2f$' % (indx*0.5/num_plots, (indx+1)*0.5/num_plots))
	plt.xlabel('Time')
	plt.ylabel('Number of cells')
	plt.title('HE-Number of free cells in uncertainty interval for improved exploration')
	plt.legend(labels)

	plt.subplot(4,2,4)
	labels = []
	data = np.array(list(heHistogram_o))
	for indx in range(0, num_plots):
	    plt.plot(time, data.transpose()[indx])
	    labels.append(r'$%.2f - %.2f$' % (indx*0.5/num_plots+0.5, (indx+1)*0.5/num_plots+0.5))
	plt.xlabel('Time')
	plt.ylabel('Number of cells')
	plt.title('HE-Number of occupied cells in uncertainty interval for improved exploration')
	plt.legend(labels)



	plt.subplot(4,2,5)
	labels = []
	data = np.array(list(ibHistogram_f))
	for indx in range(0, num_plots):
	    plt.plot(time, data.transpose()[indx])
	    labels.append(r'$%.2f - %.2f$' % (indx*0.5/num_plots, (indx+1)*0.5/num_plots))
	plt.xlabel('Time')
	plt.ylabel('Number of cells')
	plt.title('UHE-Number of free cells in uncertainty interval for normal exploration')
	plt.legend(labels)

	plt.subplot(4,2,6)
	labels = []
	data = np.array(list(ibHistogram_o))
	for indx in range(0, num_plots):
	    plt.plot(time, data.transpose()[indx])
	    labels.append(r'$%.2f - %.2f$' % (indx*0.5/num_plots+0.5, (indx+1)*0.5/num_plots+0.5))
	plt.xlabel('Time')
	plt.ylabel('Number of cells')
	plt.title('UHE-Number of occupied cells in uncertainty interval for normal exploration')
	plt.legend(labels)



	plt.subplot(4,2,7)
	labels = []
	data = np.array(list(aepHistogram_f))
	for indx in range(0, num_plots):
	    plt.plot(time, data.transpose()[indx])
	    labels.append(r'$%.2f - %.2f$' % (indx*0.5/num_plots, (indx+1)*0.5/num_plots))
	plt.xlabel('Time')
	plt.ylabel('Number of cells')
	plt.title('AEP-Number of free cells in uncertainty interval for normal exploration')
	plt.legend(labels)

	plt.subplot(4,2,8)
	labels = []
	data = np.array(list(aepHistogram_o))
	for indx in range(0, num_plots):
	    plt.plot(time, data.transpose()[indx])
	    labels.append(r'$%.2f - %.2f$' % (indx*0.5/num_plots+0.5, (indx+1)*0.5/num_plots+0.5))
	plt.xlabel('Time')
	plt.ylabel('Number of cells')
	plt.title('AEP-Number of occupied cells in uncertainty interval for normal exploration')
	plt.legend(labels)


def saveGraphs(time, environment, heGraph, ibGraph, aepGraph):
	heUncertainty = heGraph.get_uncertainty()
	ibUncertainty = ibGraph.get_uncertainty()
	aepUncertainty = aepGraph.get_uncertainty()

	heAverage = heGraph.get_average()
	ibAverage = ibGraph.get_average()
	aepAverage = aepGraph.get_average()

	heHistogram_o = heGraph.get_histogram_o()
	ibHistogram_o = ibGraph.get_histogram_o()
	aepHistogram_o = aepGraph.get_histogram_o()

	heHistogram_f = heGraph.get_histogram_f()
	ibHistogram_f = ibGraph.get_histogram_f()
	aepHistogram_f = aepGraph.get_histogram_f()

	[a,b,c] = plt.plot(time, heUncertainty, 'r', time, ibUncertainty, 'b', time, aepUncertainty, 'g')
	plt.xlabel('Time')
	plt.ylabel('Total uncertainty')
	plt.legend([a,b,c], ["HE", "UHE", "AEP"])
	plt.savefig(environment+'_total.png', bbox_inches='tight')
	plt.close()

	[a,b,c] = plt.plot(time, heAverage, 'r', time, ibAverage, 'b', time, aepAverage, 'g')
	plt.xlabel('Time')
	plt.ylabel('Average uncertainty')
	plt.legend([a,b,c], ["HE", "UHE", "AEP"])
	plt.savefig(environment+'_average_occupied.png', bbox_inches='tight')
	plt.close()

	num_plots = 5
	colors = [plt.cm.jet(i) for i in np.linspace(0, 1, num_plots)]

	plt.rc('axes', prop_cycle=plt.cycler('color', colors))



	labels = []
	data = np.array(list(heHistogram_f))
	for indx in range(0, num_plots):
	    plt.plot(time, data.transpose()[indx])
	    labels.append(r'$%.2f - %.2f$' % (indx*0.5/num_plots, (indx+1)*0.5/num_plots))
	plt.xlabel('Time')
	plt.ylabel('Number of cells')
	plt.legend(labels)
	plt.savefig(environment+'_free_HE.png', bbox_inches='tight')
	plt.close()

	labels = []
	data = np.array(list(heHistogram_o))
	for indx in range(0, num_plots):
	    plt.plot(time, data.transpose()[indx])
	    labels.append(r'$%.2f - %.2f$' % (indx*0.5/num_plots+0.5, (indx+1)*0.5/num_plots+0.5))
	plt.xlabel('Time')
	plt.ylabel('Number of cells')
	plt.legend(labels)
	plt.savefig(environment+'_occupied_HE.png', bbox_inches='tight')
	plt.close()



	labels = []
	data = np.array(list(ibHistogram_f))
	for indx in range(0, num_plots):
	    plt.plot(time, data.transpose()[indx])
	    labels.append(r'$%.2f - %.2f$' % (indx*0.5/num_plots, (indx+1)*0.5/num_plots))
	plt.xlabel('Time')
	plt.ylabel('Number of cells')
	plt.legend(labels)
	plt.savefig(environment+'_free_UHE.png', bbox_inches='tight')
	plt.close()

	labels = []
	data = np.array(list(ibHistogram_o))
	for indx in range(0, num_plots):
	    plt.plot(time, data.transpose()[indx])
	    labels.append(r'$%.2f - %.2f$' % (indx*0.5/num_plots+0.5, (indx+1)*0.5/num_plots+0.5))
	plt.xlabel('Time')
	plt.ylabel('Number of cells')
	plt.legend(labels)
	plt.savefig(environment+'_occupied_UHE.png', bbox_inches='tight')
	plt.close()



	labels = []
	data = np.array(list(aepHistogram_f))
	for indx in range(0, num_plots):
	    plt.plot(time, data.transpose()[indx])
	    labels.append(r'$%.2f - %.2f$' % (indx*0.5/num_plots, (indx+1)*0.5/num_plots))
	plt.xlabel('Time')
	plt.ylabel('Number of cells')
	plt.legend(labels)
	plt.savefig(environment+'_free_AEP.png', bbox_inches='tight')
	plt.close()

	labels = []
	data = np.array(list(aepHistogram_o))
	for indx in range(0, num_plots):
	    plt.plot(time, data.transpose()[indx])
	    labels.append(r'$%.2f - %.2f$' % (indx*0.5/num_plots+0.5, (indx+1)*0.5/num_plots+0.5))
	plt.xlabel('Time')
	plt.ylabel('Number of cells')
	plt.legend(labels)
	plt.savefig(environment+'_occupied_AEP.png', bbox_inches='tight')
	plt.close()


def printGraphs(environment, size, mode):
	graph = graphs(size, 5)
	heMergeGraphs = mergeGraphs(size, 5)
	ibMergeGraphs = mergeGraphs(size, 5)
	aepMergeGraphs = mergeGraphs(size, 5)

	for x in range(1,10):
		try:
			graph.readFile(environment+'_he_'+str(x)+'.txt')
			heMergeGraphs.addGraph(graph)
		except Exception as e:
			print(environment+'_he_'+str(x)+'.txt not found')

		try:
			graph.readFile(environment+'_ib_'+str(x)+'.txt')
			ibMergeGraphs.addGraph(graph)
		except Exception as e:
			print(environment+'_ib_'+str(x)+'.txt not found')

		try:
			graph.readFile(environment+'_aep_'+str(x)+'.txt')
			aepMergeGraphs.addGraph(graph)
		except Exception as e:
			print(environment+'_aep_'+str(x)+'.txt not found')

	if mode == 'plot':
		plotGraphs([i for i in range(0, size)], heMergeGraphs, ibMergeGraphs, aepMergeGraphs)
	else:
		saveGraphs([i for i in range(0, size)], environment, heMergeGraphs, ibMergeGraphs, aepMergeGraphs)
 

def saveAll():
	printGraphs('simple',340,'save')
	printGraphs('rooms',300,'save')
	printGraphs('office',270,'save')

def plotAll():
	printGraphs('simple',340,'plot')
	printGraphs('rooms',300,'plot')
	printGraphs('office',270,'plot')
	plt.show()

plotAll()
