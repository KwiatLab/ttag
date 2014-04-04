#If you want to dump by time, set to True, if dump by datapoints, set to False
dumpByTime=False

#Amount to dump - 
#if dumpyByTime is True, the number of seconds to dump, if False, the number of datapoints\
dumpamt=-1

buffernumber=2

filename="C:/Users/kwiat/Desktop/dumpedDATA.csv"

from pylab import *
import ttag

print "Writing File: \""+filename+"\""
print "Amount to dump:",dumpamt
try:
	x=ttag.TTBuffer(buffernumber)
	#print 
	if (dumpByTime):
		(c,t)=x(dumpamt)
	else:
		if (dumpamt==-1):
			dumpamt = len(x)
			print "Set Dump Amount:",dumpamt
		(c,t) = x[-int(dumpamt):]
	savetxt(filename,(c,t))
except Exception, e:
	print e
	raw_input()
