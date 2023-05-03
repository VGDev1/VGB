import os
import subprocess

import os

abspath = os.path.abspath(__file__)
dname = os.path.dirname(abspath)
os.chdir(dname)

def placeValuesInIno(times, temps, speeds):
    #input file
    fin = open("VBISS/VBISS.ino", "rt")
    #remove old output file and create new one
    fout = open("VBISU/VBISU.ino", "wt")

    # extract times, temps and speeds from the list of tk.Entry objects
    times = list(map(lambda x: int(x.get()), times))
    temps = list(map(lambda x: int(x.get()), temps))
    speeds = list(map(lambda x: int(x.get()), speeds))

    ts = times

    for i in range(1, len(ts)):
        ts[i] = ts[i-1] + ts[i]
    # replace tms with the times, on format {time1, time2} from the times list, and so on. do for temps and speeds as well
    for line in fin:
        fout.write(line.replace('tms', str(ts).replace('[', '{').replace(']', '}')).replace('tmps', str(temps).replace('[', '{').replace(']', '}')).replace('spds', str(speeds).replace('[', '{').replace(']', '}')))
    
    #close input and output files
    fin.close()
    fout.close()

def compileCode(port):
    print(port)
    # use arduino-cli to install #include <Adafruit_MAX31865.h> include <LiquidCrystal.h>  include <PID_v1.h> include <Wire.h>

    subprocess.call(["arduino-cli", "lib", "install", "Adafruit BusIO"])
    subprocess.call(["arduino-cli", "lib", "install", "LiquidCrystal"])
    subprocess.call(["arduino-cli", "lib", "install", "PID"])
    subprocess.call(["arduino-cli", "compile", "--fqbn", "arduino:avr:uno", "VBISU/VBISU.ino"])
    subprocess.call(["arduino-cli", "upload", "-p", port, "--fqbn", "arduino:avr:uno", "VBISU/VBISU.ino"])
