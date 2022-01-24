#"SAILOR" GUI - Systemy Mikroprocesorowe 2
#Wykonał: Sylwester Mącznik

import tkinter as tk
from tkinter import *
from PIL import ImageTk
from PIL import Image
import data as dat
#x = 151.0 #north of image


dat.init()


class Gui(object):
    def __init__(self, master, filename, **kwargs):
        self.master = master

        self.canvas = tk.Canvas(master, width=1596, height=1500)
        self.canvas.pack(fill="both", expand=True)
        self.filename = filename
        self.canvas.pack()

        self.update = self.draw().__next__
        master.after(10, self.update)

    def draw(self):
        image = Image.open(self.filename)
        angle = 0
        bg = PhotoImage(file="C://Users//Sylwek//Desktop//MAIN.png")
        canvas_bg = self.canvas.create_image(487, 670, image=bg)
        while True:
            speedw = dat.read()[0]
            angl = dat.read()[1] + 45
            beauf = dat.read()[2]
            tkimage = ImageTk.PhotoImage(image.rotate(angle))
            dat.read()

            canvas_obj = self.canvas.create_image(500, 500, image=tkimage)
            self.master.after_idle(self.update)
            canvas_txt = self.canvas.create_text(450, 700, text= str(speedw) + " m/s || ", font=("Helvetica", 50), fill="gray")
            canvas_beauf = self.canvas.create_text(650,700, text = str(beauf) + " B", font = ("Helvetica",50), fill = "gray")

            yield
            self.canvas.delete(canvas_txt)
            self.canvas.delete(canvas_beauf)
            self.canvas.delete(canvas_obj)
            angle = 360 - angl
            angle %= 360


root = tk.Tk()
app = Gui(root, "C://Users//Sylwek//Desktop//ARR.png")
root.mainloop()
