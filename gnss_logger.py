import serial
import csv
import time
import datetime
gnss=serial.Serial("COM24",9600)
HP=2    ##if HP mode, place 2

##Stop recording, Ctrl+C
data=[]
i=0
try:
    while True:
        while True:
            temp=gnss.read()       ##uart read
            if temp==b'$':
                temp=gnss.read(5)       ##read sentence start
                if temp==b'GNGGA':
                    temp=gnss.read(100)
                    msg=temp.split(b'\r')
                    print(i,msg[0])
                    data.append([msg[0].decode('utf8')])
                    i=i+1
                    break
except KeyboardInterrupt:
    pass

#generate timestamped file name
dt=datetime.datetime.now()
filename="{:02}".format(dt.year)+"{:02}".format(dt.month)+"{:02}".format(dt.day)+'_'+"{:02}".format(dt.hour)+"{:02}".format(dt.minute)
print(filename+'.csv')

#data output
with open(filename+'.csv', 'w',newline="") as f:
    writer = csv.writer(f)
    writer.writerows(data)