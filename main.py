import tkinter as tk
import json
from serial.tools import list_ports
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import (FigureCanvasTkAgg, 
NavigationToolbar2Tk)
import numpy as np
from tkinter.ttk import *



class GUI:

    def __init__(self) -> None:
        master = tk.Tk()


        tk.Label(master, text="Times").grid(row=0, columnspan=1)

        tk.Label(master, text="Temps").grid(row=1, columnspan=1)

        tk.Label(master, text="Speeds").grid(row=2, columnspan=1)

        times = []
        temps = []
        speeds = []

        for i in range(0, 8):
            tie = tk.Entry(master, width=10)
            tie.grid(row=0, column=i+1)
            times.append(tie)

            tee = tk.Entry(master, width=10)
            tee.grid(row=1, column=i+1)
            temps.append(tee)

            spe = tk.Entry(master, width=10)
            spe.grid(row=2, column=i+1)
            speeds.append(spe)

        with open("profiles.json", "r") as read_file:
            data = json.load(read_file)

        engines = data["engines"]

        engineNames = list(map(lambda x: x["Name"] , engines))

        selectedEngine = tk.StringVar(master)

        tk.Label(master, text="Select Profile").grid(row=3, column=0)

        tk.OptionMenu(master, selectedEngine, *engineNames).grid(row =3, column=1)

        def setTextInput(text, entry):
            entry.delete(0,"end")
            entry.insert(0, text)

        def loadProfile():
            timesList = []
            tempsList = []
            speedsList = []
            engineModel = selectedEngine.get()
            for e in engines:
                if e["Name"] == engineModel:
                    timesList = e["times"] 
                    tempsList = e["temps"] 
                    speedsList = e["speeds"] 

            for i in range(0, len(timesList)):
                setTextInput(timesList[i], times[i])
                setTextInput(tempsList[i], temps[i])
                setTextInput(speedsList[i], speeds[i])

        def clearFields():
            for i in range(0, len(times)):
                setTextInput("", times[i])
                setTextInput("", temps[i])
                setTextInput("", speeds[i]) 

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

            x = getTimes()

            lst = []
            for v in x:
                lst.append(v + sum(lst))
            x = lst

            print("printar x")
            print(x)

            plt1.plot(np.array(x), np.array(getTemps()))

            plt2.plot(np.array(x), np.array(getSpeeds()), color="green")


            canvas1 = FigureCanvasTkAgg(fig1,
                                    master = master)  
            canvas1.draw()

            canvas2 = FigureCanvasTkAgg(fig2,
                                    master = master)  
            canvas2.draw()
        
            # placing the canvas on the Tkinter window
            canvas1.get_tk_widget().grid(row=4, columnspan=5 ,sticky="nsew")
            canvas1.get_tk_widget().grid_rowconfigure(4, weight=1)

            canvas2.get_tk_widget().grid(row=4, column=5, columnspan=5 ,sticky="nsew")
            canvas2.get_tk_widget().grid_rowconfigure(4, weight=1)

        tk.Button(master, text="Analyze", command=plot, width=8).grid(row =3, column=6)

        progress = Progressbar(master, orient = "horizontal",
              length = 100, mode = 'determinate')
        progress.grid(row =3, column=8)

        def upload():
            #Lite checks här så inget är tomt
            #Ändra i .ino filen
            #Compile
            #Upload och en progressbar
            #Complete popup
            print("upload")

        tk.Button(master, text="Upload", command=upload, width=8).grid(row =3, column=7)


        #plot()
        
        tk.mainloop()
app = GUI()
