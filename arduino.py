import os
import subprocess

abspath = os.path.abspath(__file__)
dname = os.path.dirname(abspath)
os.chdir(dname)

def placeValuesInIno(times, temps, speeds, Kp, Ki, Kd):
    #input file
    fin = open("VBISS/VBISS.ino", "rt")
    #remove old output file and create new one
    fout = open("VBISU/VBISU.ino", "wt")

    times = list(map(lambda x: int(x.get()), times))
    temps = list(map(lambda x: int(x.get()), temps))
    speeds = list(map(lambda x: int(x.get()), speeds))

    ts = times

    for i in range(1, len(ts)):
        ts[i] = ts[i-1] + ts[i]

    for line in fin:
        fout.write(line.replace('ParTimes', str(ts).replace('[', '{').replace(']', '}')).replace('ParTemps', str(temps).replace('[', '{').replace(']', '}')).replace('ParSpeeds', str(speeds).replace('[', '{').replace(']', '}')).replace('ParNbrOfSequences', str(len(ts))).replace('ParKp', str(Kp)).replace('ParKi', str(Ki)).replace('ParKd', str(Kd)))
    
    fin.close()
    fout.close()

def compileCode(port):
    print(port)
    subprocess.call(["arduino-cli", "lib", "install", "Adafruit BusIO"])
    subprocess.call(["arduino-cli", "lib", "install", "PID"])
    subprocess.call(["arduino-cli", "lib", "install", "OneWire"])
    subprocess.call(["arduino-cli", "lib", "install", "DallasTemperature"])
    subprocess.call(["arduino-cli", "lib", "install", "U8g2"])
    subprocess.call(["arduino-cli", "compile", "--fqbn", "arduino:avr:uno", "VBISU/VBISU.ino"])
    subprocess.call(["arduino-cli", "upload", "-p", port, "--fqbn", "arduino:avr:uno", "VBISU/VBISU.ino"])
