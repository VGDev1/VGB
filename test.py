import tkinter as tk

data = {
    "times": [20, 10, 10, 10, 30, 40, 30],
    "temps": [95, 95, 95, 90, 60, 85, 80],
    "speeds": [0, 200, 300, 400, 400, 510, 510]
}

class Timeline(tk.Canvas):
    def __init__(self, master, data, *args, **kwargs):
        super().__init__(master, *args, **kwargs)
        self.data = data
        self.width = self.winfo_reqwidth()
        self.height = self.winfo_reqheight()
        self.draw_timeline()

    def draw_timeline(self):
        num_points = len(self.data["times"])
        x_spacing = self.width / num_points

        for i, time in enumerate(self.data["times"]):
            x = i * x_spacing

            # Draw timestamp below the point
            self.create_text(x, self.height + 10, text=str(time), anchor="n")

            # Draw speed and temp above the point
            temp = self.data["temps"][i]
            speed = self.data["speeds"][i]
            self.create_text(x, self.height - 20, text=f"Temp: {temp}\nSpeed: {speed}", anchor="s")

root = tk.Tk()
root.geometry("500x300")

timeline = Timeline(root, data)
timeline.pack(fill="both", expand=True)

root.mainloop()
