#"SAILOR" GUI - Systemy Mikroprocesorowe 2
#Wykonał: Sylwester Mącznik

import serial
import math
import time
import tkinter as tk
from tkinter import *
from PIL import ImageTk
from PIL import Image


global ser


def mapp(x,in_min, in_max, out_min, out_max):
    return int((x-in_min)*(out_max-out_min)/(in_max-in_min)+out_min)


def init():
    global ser
    ser = serial.Serial('COM6', 9600) #COM3 kabel, COM6 Bluetooth


def read():
    global ser
    fixedheading = 0
    i = 0
    wind = 0
    sum = [0, 0, 0, 0, 0]
    commas = []
    data = str(ser.readline(1000)).replace("b\'\\r", "").replace("\\n\'", "")

    for c in range(len(data)):
        if (data[c] == ","):
            commas.append(c)  #znajdz wszyjskie przecinki
    try:
        headingDegrees = float(data[0:commas[0]])
        tpm1Diff = int(data[commas[0] + 1:len(data)])
        if tpm1Diff != 0:
            herz = 1/tpm1Diff*0.0000026
        else:
            herz = 0
        sum[i] = herz
        i += 1
        if i == 5:
            i = 0
        for j in range(len(sum)):
            wind += sum[j]

        wind = wind / 5
        speedw = format(round(wind, 1))
        finalspeed = format(round(herz*1661270000, 1))
        beaufort = ((float(finalspeed)/2)+10)/6
        format(round(beaufort, 1))
        if 1 <= headingDegrees < 240:
            fixedheading = mapp(headingDegrees, 0, 239, 0, 179)
        elif headingDegrees >= 240:
            fixedheading = mapp(headingDegrees, 240, 360, 180, 360)
        print(finalspeed)
        #print(beaufort)
        return[finalspeed, fixedheading, beaufort]

    except Exception as e:
        print(e)
        commas = []
        return[0, 0, 0]






