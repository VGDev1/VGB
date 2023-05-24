import matplotlib.pyplot as plt
from datetime import timedelta
import tkinter as tk
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

# Example data
data = {
    "times": [20, 10, 10, 10, 30, 40, 30],
    "temps": [95, 95, 95, 90, 60, 85, 80],
    "speeds": [0, 200, 300, 400, 400, 510, 510]
}

# Calculate cumulative sum of times and convert to hour timestamps
timestamps = [sum(data["times"][:i]) / 60 for i in range(1, len(data["times"]) + 1)]  # Convert to hours

# Create the plot
fig, ax1 = plt.subplots()
ax2 = ax1.twinx()
ax1.plot(timestamps, data["temps"], 'r-', label='Temperature')
ax2.plot(timestamps, data["speeds"], 'b-', label='Speed')

# Set labels and title
ax1.set_xlabel('Time (hours)')
ax1.set_ylabel('Temperature')
ax2.set_ylabel('Speed')
plt.title('Temperature and Speed')

# Add legends
lines, labels = ax1.get_legend_handles_labels()
lines2, labels2 = ax2.get_legend_handles_labels()
ax2.legend(lines + lines2, labels + labels2, loc='best')

# Create a Tkinter window
root = tk.Tk()
root.title("Data Plot")

# Create a Tkinter canvas
canvas = FigureCanvasTkAgg(fig, master=root)
canvas.draw()
canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=1)

# Run the Tkinter event loop
tk.mainloop()
