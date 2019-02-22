#!/usr/bin/python3
import matplotlib.pyplot as plt
import numpy

bufferFileIn = ""
bufferFileOut = ""

with open("schedulerInput.txt",'r') as file:
    bufferFileIn = file.readlines()

with open("schedulerOutput.txt",'r') as file:
    bufferFileOut = file.readlines()

plt.ioff()

#Discard first lines
bufferFileIn = bufferFileIn[1:]
bufferFileOut = bufferFileOut[1:]

bufferFileIn = bufferFileIn[:-3]
bufferFileOut = bufferFileOut[:-3]


inputAndOutputBitmapPerInterval = {}

for intervalBitmap in bufferFileIn:
    y = intervalBitmap.split(sep=':')

    interval = y[0] #"+350000000.0ns"
    interval = interval.split(sep=".")#["+350000000", "0ns"]
    interval = interval[0]#"+350000000"
    interval = int(interval)#350000000
    interval /= 1000000 #350

    inputAndOutputBitmapPerInterval[interval] = (int(y[1],2), 0)

for intervalBitmap in bufferFileOut:
    y = intervalBitmap.split(sep=':')

    interval = y[0] #"+350000000.0ns"
    interval = interval.split(sep=".")#["+350000000", "0ns"]
    interval = interval[0]#"+350000000"
    interval = int(interval)#350000000
    interval /= 1000000 #350

    if y[0] not in inputAndOutputBitmapPerInterval:
        inputAndOutputBitmapPerInterval[interval] = (0, int(y[1],2))
    else:
        inputAndOutputBitmapPerInterval[interval][1] = int(y[1],2)
    

fig,ax = plt.subplots()

x = list(sorted(inputAndOutputBitmapPerInterval.keys()))

for interval in sorted(inputAndOutputBitmapPerInterval.keys()):

    bitmaps = inputAndOutputBitmapPerInterval[interval]

    input_barh = []
    output_barh = []

    for i in range(25):
        if ( (bitmaps[0]>>i) & 0x01):
            ax.broken_barh([(interval,1)],(i*2,2), facecolors="red", alpha=0.7) #Blocks occupied before scheduling (unexpected access reported by UEs, may be a PU,SU,whatever)
        if ( (bitmaps[1]>>i) & 0x01):
            ax.broken_barh([(interval,1)],(i*2,2), facecolors="blue", alpha=0.5) #Blocks occupied by transmissions scheduled by the eNB

    ax.set_xlabel('time (ms)', )
    ax.set_ylabel('Input')
    ax.tick_params('y')
    plt.show(block=False)
    #plt.savefig("%s.jpg" % ue)
    pass

print("Done")
input()
pass