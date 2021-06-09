import tkinter as tk
import json
from serial.tools import list_ports
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import (FigureCanvasTkAgg, 
NavigationToolbar2Tk)


class GUI:

    def __init__(self) -> None:
        master = tk.Tk()
    

        def plot():
    
            # the figure that will contain the plot
            fig = Figure(figsize = (5, 5),
                        dpi = 100)
        
            # list of squares
            y = [i**2 for i in range(101)]
        
            # adding the subplot
            plot1 = fig.add_subplot(111)
        
            # plotting the graph
            plot1.plot(y)
        
            # creating the Tkinter canvas
            # containing the Matplotlib figure
            canvas = FigureCanvasTkAgg(fig,
                                    master = master)  
            canvas.draw()
        
            # placing the canvas on the Tkinter window
            canvas.get_tk_widget().grid(row=4, columnspan=10 ,sticky="nsew")
        
            #canvas.get_tk_widget().grid_columnconfigure(3, weight=1)
            canvas.get_tk_widget().grid_rowconfigure(4, weight=1)


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

        tk.Button(master, text="Analyze", command=plot, width=8).grid(row =3, column=6)

        tk.Button(master, text="Upload", command=clearFields, width=8).grid(row =3, column=7)

        #plot()
        
        tk.mainloop()
app = GUI()
