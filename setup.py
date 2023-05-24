from cx_Freeze import setup, Executable

base = None    

executables = [Executable("main.py", base=base)]


# packages import tkinter as tk
# import json
# from serial.tools import list_ports
# from matplotlib.figure import Figure
# from matplotlib.backends.backend_tkagg import (FigureCanvasTkAgg, 
# NavigationToolbar2Tk)
# import numpy as np
# from tkinter.ttk import *
# from tkinter import messagebox
# from arduino import *
# import time
#import os
#import subprocess
packages = ["tkinter", "json", "serial.tools", "matplotlib.figure", "matplotlib.backends.backend_tkagg", "numpy", "tkinter.ttk", "tkinter.messagebox", "arduino", "time", "subprocess", "os"]
options = {
    'build_exe': {    
        'packages':packages,
    },    
}

setup(
    name = "VG_Break_In",
    options = options,
    version = "1.0",
    description = 'VG Break in system programmer',
    executables = executables
)