#!/usr/bin/python
import sys
import re
import time
from os import environ
from colorama import Fore, Back, Style
from os import environ
environ['PYGAME_HIDE_SUPPORT_PROMPT'] = '1'
import pygame

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

# screen init
matrix_scale=32
pygame.init()
screen = pygame.display.set_mode((8*(matrix_scale+1), 8*(matrix_scale+1)))

#set up the colors
white = (255, 255, 255)
black = (  0,   0,   0)
green = (0, 255, 0)
blue = (0, 0, 180)
red   = (255,   0,   0)
red0   = (255,   0,   0)
red1   = (255,   0,   0)
or0 = (150, 80, 0)
or1 = (200, 108, 0)
or2 = (255, 138, 0)

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

    # update the lamp
    bit=(1<<((col*8)+row))
    if (lamps & bit):
        lamps &= ~bit
    else:
        lamps |= bit

    # output when all events of an epoch are processed
    if (delay != 0):
        dt=(delay*TTAG_SCALE)
        t+=dt

        # draw the matrix
        screen.fill((0,0,0))
        for x in range(0,7):
            for y in range(0,7):
                if (lamps & (1<<((x*8)+y))):
                    pygame.draw.rect(screen, red, (x*(matrix_scale+1), y*(matrix_scale+1), matrix_scale, matrix_scale))
        pygame.display.update()

        time.sleep(dt/1000)
