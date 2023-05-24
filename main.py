from datetime import timedelta
import tkinter as tk
import json
from serial.tools import list_ports
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import (FigureCanvasTkAgg, 
NavigationToolbar2Tk)
import numpy as np
from tkinter.ttk import *
from tkinter import messagebox
from arduino import *
import time




class GUI:

    def __init__(self) -> None:
        master = tk.Tk()

        # set title of window to VG Break In System
        master.title("VG Break In System")

        tk.Label(master, text="Times").grid(row=0, columnspan=1)

        tk.Label(master, text="Temps").grid(row=1, columnspan=1)

        tk.Label(master, text="Speeds").grid(row=2, columnspan=1)

        times = []
        temps = []
        speeds = []

        def createGridEntrys(nbrOfColumns):
            for i in range(0, nbrOfColumns):
                tie = tk.Entry(master, width=10)
                tie.grid(row=0, column=i+1)
                times.append(tie)

                tee = tk.Entry(master, width=10)
                tee.grid(row=1, column=i+1)
                temps.append(tee)

                spe = tk.Entry(master, width=10)
                spe.grid(row=2, column=i+1)
                speeds.append(spe)

        # remove last column of times
        def removeGridEntrys():
            # grid forget all times speeds and temps
            for i in range(0, len(times)):
                times[i].grid_forget()
                temps[i].grid_forget()
                speeds[i].grid_forget()

            # clear lists
            times.clear()
            temps.clear()
            speeds.clear()

        with open("settings.json", "r") as read_file:
            data = json.load(read_file)

        engines = data["engines"]
        Kp = data["Kp"]
        Ki = data["Ki"]
        Kd = data["Kd"]
        # TODO add these in arduino code.

        engineNames = list(map(lambda x: x["Name"] , engines))

        selectedEngine = tk.StringVar(master)

        tk.Label(master, text="Select Profile").grid(row=3, column=0)

        tk.OptionMenu(master, selectedEngine, *engineNames).grid(row =3, column=1)

        def setTextInput(text, entry):
            entry.delete(0,"end")
            entry.insert(0, text)

        def loadProfile():
            engineModel = selectedEngine.get()
            for e in engines:
                if e["Name"] == engineModel:
                    timesList = e["times"] 
                    tempsList = e["temps"] 
                    speedsList = e["speeds"] 

            removeGridEntrys()
            createGridEntrys(len(timesList))
                    
            for i in range(0, len(timesList)):
                setTextInput(timesList[i], times[i])
                setTextInput(tempsList[i], temps[i])
                setTextInput(speedsList[i], speeds[i])

        def clearFields():
            for i in range(0, len(times)):
                setTextInput("", times[i])
                setTextInput("", temps[i])
                setTextInput("", speeds[i]) 
            
            removeGridEntrys()

        tk.Button(master, text="Load Profile", command=loadProfile).grid(row =3, column=2)

        tk.Button(master, text="Clear fields", command=clearFields).grid(row =3, column=3)


        selectedComPort = tk.StringVar(master)

        ports = list_ports.comports()

        tk.Label(master, text="Select Com Port").grid(row=3, column=4)
        om = tk.OptionMenu(master, selectedComPort, *ports)
        om.grid(row =3, column=5)
        om.config(width=7)

        def getTimes():
            return list(map(lambda e: int(e.get()), times))
        def getTemps(): 
            return list(map(lambda e: int(e.get()) , temps))
        def getSpeeds(): 
            return list(map(lambda e: int(e.get()), speeds))

        def plot():
    
            # the figure that will contain the plot
            fig1 = Figure(figsize = (5, 5),
                        dpi = 100)
            fig2 = Figure(figsize = (5, 5),
                        dpi = 100)                        
         
            # adding the subplot
            plt1 = fig1.add_subplot(111)
            plt2 = fig2.add_subplot(111)
            plt1.title.set_text('Temperatures')
            plt2.title.set_text('Speeds')

            # get total time as label

            x = getTimes()

            # create new list where each element is the sum of the previous elements
            for i in range(1, len(x)):
                x[i] = x[i-1] + x[i]


            print("printar x")
            print(x)

            plt1.plot(np.array(x), np.array(getTemps()))
            plt1.set_xlabel('Time (min)')
            plt1.set_ylabel('Temperature (C)')

            plt2.plot(np.array(x), np.array(getSpeeds()), color="green")
            plt2.set_xlabel('Time (min)')
            plt2.set_ylabel('Speed (rpm)')


            canvas1 = FigureCanvasTkAgg(fig1,
                                    master = master)  
            canvas1.draw()

            canvas2 = FigureCanvasTkAgg(fig2,
                                    master = master)  
            canvas2.draw()

            canvas1.get_tk_widget().grid(row=4, columnspan=5 ,sticky="nsew")
            canvas1.get_tk_widget().grid_rowconfigure(4, weight=1)

            canvas2.get_tk_widget().grid(row=4, column=5, columnspan=5 ,sticky="nsew")
            canvas2.get_tk_widget().grid_rowconfigure(4, weight=1)

            # sum of all times total time in hours and minutes
            total_time_minutes = x[-1]
            # string format ex: 3 hours and 30 minutes
            hours = int(total_time_minutes / 60)
            minutes = int(total_time_minutes % 60)
            # show total time in a label below charts, text in the middle bold and big
            tk.Label(master, text="Total time: " + str(hours) + " hours & " + str(minutes) + " minutes", font=("Arial Bold", 20)).grid(row=5, columnspan=10)


        tk.Button(master, text="Analyze", command=plot, width=8).grid(row =3, column=6)


        def upload():


            allEntrys = times + speeds + temps
            if not list(filter(lambda e: len(e.get()) != 0, allEntrys)):
                messagebox.showerror("showerror", "Error: Check values!")
                return 
            # check that comport is selected
            if selectedComPort.get() == "":
                messagebox.showerror("showerror", "Error: Select comport!")
                return
            
            placeValuesInIno(times, temps, speeds)
            compileCode(selectedComPort.get())
            time.sleep(10)
            # update progressbar to 100 over 20 seconds using timer
            messagebox.showinfo(title="Complete", message="Upload complete")
            return

        tk.Button(master, text="Upload", command=upload, width=8).grid(row =3, column=7)

        tk.mainloop()
app = GUI()
