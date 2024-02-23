#!/usr/bin/python
import sys
import re
from os import environ
from colorama import Fore, Back, Style

TTAG_SCALE=20

num_args=len(sys.argv)
if (num_args<=1):
    print(Fore.RED + "Usage: pattern_analysis.py <c struct file>")
    sys.exit()

print("Reading data...")
with open(sys.argv[1]) as f:
    cdata = f.read()
print("done.")

# parse the string
result = re.findall("\{(\d+),\s(\d+),\s(\d+)\}", cdata)
#for ev in result:
#    print(ev)
#sys.exit()

# lamp matrix
lamps=0

# statistics
bins = [0]*100

# process all events
t=0
sp=0
is_special=0
col_sp=0
row_sp=0
delay_sp=0
num_ev=0
for ev in result:
    col=int(ev[0])
    row=int(ev[1])
    delay=int(ev[2])
    # handle 3 byte events
    if ((col==7) and (row==7) and (delay==3) and (is_special==0)):
        #print("ha %d %d %d" % (col, row, delay))
        sp += 1
        is_special=1
        continue
    if (is_special==1):
        #print("he %d %d %d" % (col, row, delay))
        col_sp = col
        row_sp = row
        delay_sp = delay
        is_special=2
        continue
    if (is_special==2):
        #print("hui %d %d %d" % (col, row, delay))
        delay = (delay_sp | (col << 2) | (row << 5) | (delay << 8))
        #print("bla %d" % delay)
        col = col_sp
        row = row_sp
        is_special = 0
    num_ev += 1
    # output when all events of an epoch are processed
    if (delay != 0):
        t+=(delay*TTAG_SCALE)
        #print("%d, %s" % (t, hex(lamps)))
    # update statistics
    if (delay<100):
        bins[delay]+=1
    # update the lamp
    bit=(1<<((col*8)+row))
    if (lamps & bit):
        lamps &= ~bit
    else:
        lamps |= bit

# print statistics
print("Total events: %d" % num_ev)
print("Special events: %d" % sp)
for i in range(0,32):
    print("%d - %d" % (i, bins[i]))
