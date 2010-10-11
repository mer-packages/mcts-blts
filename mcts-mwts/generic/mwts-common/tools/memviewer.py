#!/usr/bin/python
import sys
import pylab


if len(sys.argv)<2:
	print "Usage: memoryviewer.py filename.monitor"
	quit()

filename = sys.argv[1]


CpuX=[]
CpuY=[]
CpuPlots=[]

MemX=[]
MemY=[]
MemPlots=[]

TestModule = "unspecified";
TestCase = "unspecified";

pylab.ion() #  Interactive on

def showplot(x, y):
	pylab.plot(x, y, linewidth=1.0)
	pylab.xlabel('time (s)')
	pylab.ylabel('Used Memory (MB)')
	pylab.ylim(-5, 500)
	pylab.title(TestModule + " / " + TestCase)
	pylab.grid(True)
	pylab.show()


f=open(filename)
lines=f.readlines()
for line in lines:
	values=line.split(":")
	if len(values)<3:
		continue
	name=values[1]
	if name=="TestModule":
		TestModule=values[2]
		continue
	if name=="TestCase":
		TestCase=values[2]
		continue

	value=float(values[2].strip())
	time=float(values[0])

	if name=="CpuUsage":
		CpuPlots.append((time, value))
		CpuX.append(time)
		CpuY.append(value)

	if name=="UsedMem":
		MemPlots.append((time, value))
		MemX.append(time)
		MemY.append(value)


showplot(MemX, MemY)
#showplot(CpuX, CpuY)

