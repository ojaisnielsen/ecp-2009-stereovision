# coding=UTF-8

from Tkinter import *
from PIL import Image, ImageTk
from PIL.ExifTags import TAGS
from numpy import concatenate, ones
import tkFileDialog
import os
import csv
import threading
import urllib

import geom
import corresppoints
import rectification
import reconst3d


class point:
    def __init__(self, parent, x, y, n):
        self.x = int(round(x))
        self.y = int(round(y))
        self.parent = parent
        self.imId = self.parent.imId
        self.n = n
        self.circle = None
        self.number = None
        self.isVisible = False
        self.isChecked = IntVar(mainWindow.pointListFrame[self.imId])
        self.isChecked.set(0)
        self.checkBut = Checkbutton(mainWindow.pointListFrame[self.imId], text = "%i : (%i; %i)" % (self.n + 1, self.x, self.y), \
                                    variable = self.isChecked, command = (lambda:self.Toggle()))
        self.checkBut.pack(side = TOP)
        
    def ChangeN(self, n):
        self.n = n
        self.checkBut.config(text = "%i : (%i; %i)" % (self.n + 1, self.x, self.y))
        self.Hide()
        self.Show()
        
    def Toggle(self):
        if self.isChecked.get() == 0:
            self.Hide()
        else:
            self.Show()
    
    def Show(self):
        if not self.isVisible:
            self.isChecked.set(1)
            self.isVisible = True
            self.circle = mainWindow.canvas[self.imId].create_oval(self.x - 2, self.y - 2, self.x + 2, self.y + 2, outline = "red", width = 1)
            self.number = mainWindow.canvas[self.imId].create_text(self.x + 2, self.y + 2, anchor = "nw", fill = "red", text = self.n + 1)  
    
    def Hide(self):
        if self.isVisible:
            self.isChecked.set(0)
            self.isVisible = False
            mainWindow.canvas[self.imId].delete(self.circle)
            mainWindow.canvas[self.imId].delete(self.number)

        
    def Move(self, x, y):
        self.Hide()
        self.x = int(round(x))
        self.y = int(round(y))
        self.Show()
        self.checkBut.config(text = "%i : (%i; %i)" % (self.n + 1, self.x, self.y))
        
class pointList:
    def __init__(self, parent):
        self.parent = parent
        self.imId = self.parent.id
        self.l = []
        
    def CoordList(self):
        coordList = []
        for point in self.l:
            coordList.append((point.x, point.y))
        return coordList
    
    def Size(self):
        return len(self.l)

        
    def AddPoint(self, x, y, visible = False):
        self.l.append(point(self, x, y, self.Size()))
        newPoint = self.l[-1]
        if visible:
            newPoint.Show()
        if len(self.parent.parent.tuplesCheckButList) < self.Size():
            self.parent.parent.addTupleCheckBut()
            
        self.RefreshBut()

        
    def RemovePoints(self, indices):
        correc = 0
        for i in range(min(indices), len(self.l)):
            if i - correc < len(self.l):
                self.l[i - correc].ChangeN(i - correc)
                if i in indices:
                    self.l[i - correc].Hide()
                    self.l[i - correc].checkBut.destroy()
                    del self.l[i - correc]
                    correc += 1
        
        self.RefreshBut()
        
        
    def ShowAll(self):
        for point in self.l:
            point.Show()
            
    def HideAll(self):
        for point in self.l:
            point.Hide()
            
    def ToggleAll(self):
        if mainWindow.listCheckButVal[self.imId].get() == 0:
            self.HideAll()
        else:
            self.ShowAll()
            
    def RefreshBut(self):
        if self.Size() == 0:
            mainWindow.listCheckBut[self.imId].config(state = DISABLED)
        else:
            mainWindow.listCheckBut[self.imId].config(state = NORMAL)
            
        self.parent.parent.RefreshBut()
        
        
class image:
    def __init__(self, parent, imId):
        self.parent = parent
        self.id = imId
        self.list = pointList(self)
        self.focalLengthMm = 0
        self.cameraCcdWidth = 0
        self.cameraModel = ""
        self.cameraMake = ""
        self.image = None
        self.size = (0, 0)
        self.P = None
        self.K = None
        self.currentSelec = None
        
        
    def ResetSelection(self, event = None):
        if not self.currentSelec == None:
            mainWindow.canvas[self.id].delete(self.currentSelec[4])
            self.currentSelec[5].destroy()
            self.currentSelec = None

        
    def DrawSelection(self, event):
        x = int(mainWindow.canvas[self.id].canvasx(event.x))
        y = int(mainWindow.canvas[self.id].canvasy(event.y))
        if self.currentSelec == None:
            self.currentSelec = [x, y, x, y]
            self.currentSelec.append(mainWindow.canvas[self.id].create_rectangle(x, y, x, y, outline = "blue", width = 1))
            self.currentSelec.append(Button(mainWindow.canvas[self.id], text = "Valider", cursor = "arrow", command = (lambda:self.LoadSelection())))
            mainWindow.canvas[self.id].create_window(x, y, anchor = S + W, window = self.currentSelec[5])
        else:
            self.currentSelec[2] = x
            self.currentSelec[3] = y
            mainWindow.canvas[self.id].delete(self.currentSelec[4])
            self.currentSelec[4] = mainWindow.canvas[self.id].create_rectangle(tuple(self.currentSelec[0:4]), outline = "blue", width = 1)
        
    def LoadSelection(self):
        xMin = min(self.currentSelec[0], self.currentSelec[2])
        xMax = max(self.currentSelec[0], self.currentSelec[2])
        yMin = min(self.currentSelec[1], self.currentSelec[3])
        yMax = max(self.currentSelec[1], self.currentSelec[3])

        self.LoadImage(self.image.crop((xMin, yMin, xMax, yMax)), 0, 0, "", self.focalLengthPx)
        
        excluded = []
        for point in self.list.l:
            if point.x in range(xMin, xMax + 1) and point.y in range(yMin, yMax + 1):
                point.Move(point.x - xMin, point.y - yMin)
            else:
                excluded.append(point.n)
        if not excluded == []:
            self.parent.RemoveTuples(excluded)
        self.ResetSelection()
        
    def SetMatrix(self, P, K):
        self.P = P
        self.K = K
        mainWindow.matrixPrint[self.id].set(mainWindow.PrintMat(P))
        
    def ImprovePoint(self, n, imRef, pRef):
        if not self.image == None and not imRef == None:
            pi = (self.list.l[n].x, self.list.l[n].y)
            pRef = (pRef.x, pRef.y)
            xn, yn = corresppoints.improvePair(imRef, self.image, pRef, pi, 3)
            self.list.l[n].Move(xn, yn)
        
    def GetList(self):
        return self.list.CoordList()
        
    def LoadImage(self, image, focalLengthMm = 0, cameraCcdWidth = 0, cameraModel = "", focalLengthPx = 0, fileName = ""):
        self.image = image
        self.size = self.image.size
        self.focalLengthPx = focalLengthPx
        self.cameraModel = cameraModel
        mainWindow.imPath[self.id].set(fileName)
        self.canvasImage = ImageTk.PhotoImage(self.image)
        mainWindow.canvas[self.id].config(scrollregion = (0, 0, self.size[0], self.size[1]))
        mainWindow.canvas[self.id].create_image(self.size[0]/2., self.size[1]/2., image = self.canvasImage)
        if not self.cameraModel == "":
            mainWindow.ccdText[self.id].set("%s\nLargeur CCD %i (mm)" % (self.cameraModel, self.id + 1))
            mainWindow.ccdLab[self.id].config(cursor = "hand2", foreground = "blue")
        mainWindow.focalMmEnt[self.id].delete(0, END)
        mainWindow.focalMmEnt[self.id].insert(0, focalLengthMm)
        mainWindow.ccdEnt[self.id].delete(0, END)
        mainWindow.ccdEnt[self.id].insert(0, cameraCcdWidth)
        mainWindow.focalPxEnt[self.id].config(state = NORMAL)
        mainWindow.focalPxEnt[self.id].delete(0, END)
        mainWindow.focalPxEnt[self.id].insert(0, self.focalLengthPx)
        mainWindow.focalPxEnt[self.id].config(state = DISABLED)
        
    def UnloadImage(self):
        blank = Image.new("L", (1,1))
        self.LoadImage(blank)
        self.setMatrix(None)
        
        
    def AddCurrentPoint(self, event):
        self.list.AddPoint(mainWindow.canvas[self.id].canvasx(event.x), mainWindow.canvas[self.id].canvasy(event.y), True)

    def ShowCurrentPoint(self, event):
        pointSide = ["gauche", "droite"]
        mainWindow.currentPoint[self.id].set("Point de %s %i (%i; %i)" % (pointSide[self.id], self.list.Size() + 1, mainWindow.canvas[self.id].canvasx(event.x), mainWindow.canvas[self.id].canvasy(event.y)))

    def HideCurrentPoint(self, event):
        mainWindow.currentPoint[self.id].set("")
        
    def CheckCamera(self, event):
        if not self.cameraModel == "":
            keywords = "site:www.dpreview.com/reviews/specs/ %s" % self.cameraModel
            method = "I'm Feeling Lucky"
            url = "http://www.google.com/search?%s" % urllib.urlencode({"btnI" : method, "q" : keywords})
            os.startfile(url)
      
    def LoadFile(self):
        imPath = tkFileDialog.askopenfilename(filetypes = [("Image", "*.*")])

        if not imPath == "":
            
            image = Image.open(imPath)
            tags = {}
            focalLengthMm = 0
            cameraCcdWidth = 0
            cameraModel = ""
            
            try:
                image._getexif()
            except AttributeError:
                encTags = None
            else:
                encTags = image._getexif()
            if encTags != None:
                for tag, value in encTags.items():
                    tags[TAGS.get(tag, tag)] = value
        
            if "FocalLength" in tags.keys():
                focalLengthMm = float(tags["FocalLength"][0])/float(tags["FocalLength"][1])
    
            if "Model" in tags.keys():
                cameraModel = tags["Model"]
    
            if "CCD width" in tags.keys():
                cameraCcdWidth = float(tags["CCD width"])
            
            self.LoadImage(image, focalLengthMm, cameraCcdWidth, cameraModel, 0, os.path.basename(imPath))
            
            
    def GetFocalPx(self):
        if float(eval(mainWindow.ccdEnt[self.id].get())) > 0:
            self.focalLengthPx = mainWindow.image[self.id].size[0] * float(eval(mainWindow.focalMmEnt[self.id].get())) / float(eval(mainWindow.ccdEnt[self.id].get()))
        else:
            self.focalLengthPx = 0
        mainWindow.focalPxEnt[self.id].config(state = NORMAL)
        mainWindow.focalPxEnt[self.id].delete(0, END)
        mainWindow.focalPxEnt[self.id].insert(0, self.focalLengthPx)
        mainWindow.focalPxEnt[self.id].config(state = DISABLED)
        mainWindow.CheckFields()
        
class ImageList:
    def __init__(self, l):
        self.l = l
        self.tuplesCheckButList = []
    
    def TupleListsSize(self):
        return len(self.tuplesCheckButList)
        
    def RemoveTuples(self, indices):
        correc = 0
        for i in range(min(indices), max(indices) + 1):
            if i in indices:
                self.tuplesCheckButList[i - correc].destroy()
                del self.tuplesCheckButList[i - correc]
                correc += 1
        for im in self.l:
            im.list.RemovePoints(indices)

    def addTupleCheckBut(self):
        self.tuplesCheckButList.append(Checkbutton(mainWindow.removePtsFrame))
        self.tuplesCheckButList[-1].val = IntVar(mainWindow.removePtsFrame)
        self.tuplesCheckButList[-1].config(variable = self.tuplesCheckButList[-1].val)
        self.tuplesCheckButList[-1].val.set(0)
        self.tuplesCheckButList[-1].pack()
            
    def RemoveSelectedTuples(self):
        indices = []
        for i in range(0, len(self.tuplesCheckButList)):
            if self.tuplesCheckButList[i].val.get() == 1:
                indices.append(i)
        self.RemoveTuples(indices)
        
    def RemoveLastTuple(self):
        self.RemoveTuples([self.TupleListsSize() - 1])
        
    def ClearTuples(self):
        if not self.tuplesCheckButList == []:
            self.RemoveLastTuple()
            self.ClearTuples()
            
    def RefreshBut(self):
        if self.TupleListsSize() == 0:
            mainWindow.removeLastBut.config(state = DISABLED)
            mainWindow.removePtsCheckBut.config(state = DISABLED)
            mainWindow.removeSelectBut.config(state = DISABLED)
        else:
            mainWindow.removeLastBut.config(state = NORMAL)
            mainWindow.removePtsCheckBut.config(state = NORMAL)
            mainWindow.removeSelectBut.config(state = NORMAL)
        
        mainWindow.listsCanvasFrame.update_idletasks()
        mainWindow.listsCanvas.config(width = mainWindow.listsCanvas.bbox("all")[2])
        mainWindow.listsCanvas.config(scrollregion = mainWindow.listsCanvas.bbox("all"))
        mainWindow.CheckFields()
        
    def ToggleAllTuples(self):
        if mainWindow.removePtsCheckButVal.get() == 0:
            for checkBut in self.tuplesCheckButList:
                checkBut.val.set(0)
        else:
            for checkBut in self.tuplesCheckButList:
                checkBut.val.set(1)



class mainWindow(Tk):
    def __init__(self, parent):
        Tk.__init__(self, parent)
        self.tk.call("package", "require", "tile")
        self.tk.call("namespace", "import", "-force", "ttk::*")
        self.tk.call("ttk::setTheme", "xpnative")
        
        self.matrices = None
        self.image = []
        self.imageList = ImageList(self.image)
        self.image.append(image(self.imageList, 0))
        self.image.append(image(self.imageList, 1))
        

        
        ### Haut
        
        topFrame = Frame(self)
        topFrame.pack(side = TOP, fill = BOTH, expand = 1)
        
        ## Cadre 1 : images
        
        imagesFrame = Frame(topFrame)
        imagesFrame.pack(side = LEFT, fill = BOTH, expand = 1)
        

        # Ligne 0
  

        browseBut = []
        
        browseBut.append(Button(imagesFrame, text=u"Choisir image 1", command = self.image[0].LoadFile))
        browseBut[0].grid(column = 0, row = 0)

        browseBut.append(Button(imagesFrame, text=u"Choisir image 2", command = self.image[1].LoadFile))
        browseBut[1].grid(column = 2, row = 0)

        
        # Ligne 1


        self.imPath = []
        imPathLab = []
        
        self.imPath.append(StringVar(self))
        imPathLab.append(Label(imagesFrame, textvariable = self.imPath[0]))
        imPathLab[0].grid(column = 0, row = 1)

        self.imPath.append(StringVar(self))
        imPathLab.append(Label(imagesFrame, textvariable = self.imPath[1]))
        imPathLab[1].grid(column = 2, row = 1)


        # Ligne 2


        self.canvas = []
        
        self.canvas.append(Canvas(imagesFrame, width = 400, height = 400, bg = "black", highlightthickness = 0, cursor = "crosshair"))
        self.canvas[0].bind("<Button-1>", self.image[0].AddCurrentPoint)
        self.canvas[0].bind("<B3-Motion>", self.image[0].DrawSelection)
        self.canvas[0].bind("<Button-3>", self.image[0].ResetSelection)
        self.canvas[0].bind("<Motion>", self.image[0].ShowCurrentPoint)
        self.canvas[0].bind("<Leave>", self.image[0].HideCurrentPoint)
        self.canvas[0].grid(column = 0, row = 2, sticky = N + S + E + W)

        self.canvas[0].scrollY = Scrollbar(imagesFrame, orient = VERTICAL)
        self.canvas[0].config(yscrollcommand = self.canvas[0].scrollY.set)
        self.canvas[0].scrollY.config(command = self.canvas[0].yview)
        self.canvas[0].scrollY.grid(column = 1, row = 2, sticky = N + S)

        self.canvas.append(Canvas(imagesFrame, width = 400, height = 400, bg = "black", highlightthickness = 0, cursor = "crosshair"))
        self.canvas[1].bind("<Button-1>", self.image[1].AddCurrentPoint)
        self.canvas[1].bind("<B3-Motion>", self.image[1].DrawSelection)
        self.canvas[1].bind("<Button-3>", self.image[1].ResetSelection)
        self.canvas[1].bind("<Motion>", self.image[1].ShowCurrentPoint)
        self.canvas[1].bind("<Leave>", self.image[1].HideCurrentPoint)
        self.canvas[1].grid(column = 2, row = 2, sticky = N + S + E + W)

        self.canvas[1].scrollY = Scrollbar(imagesFrame, orient = VERTICAL)
        self.canvas[1].config(yscrollcommand = self.canvas[1].scrollY.set)
        self.canvas[1].scrollY.config(command = self.canvas[1].yview)
        self.canvas[1].scrollY.grid(column = 3, row = 2, sticky = N + S)
        
        # Ligne 3

        self.canvas[0].scrollX = Scrollbar(imagesFrame, orient = HORIZONTAL)
        self.canvas[0].config(xscrollcommand = self.canvas[0].scrollX.set)
        self.canvas[0].scrollX.config(command = self.canvas[0].xview)
        self.canvas[0].scrollX.grid(column = 0, row = 3, sticky = E + W)

        self.canvas[1].scrollX = Scrollbar(imagesFrame, orient = HORIZONTAL)
        self.canvas[1].config(xscrollcommand = self.canvas[1].scrollX.set)
        self.canvas[1].scrollX.config(command = self.canvas[1].xview)
        self.canvas[1].scrollX.grid(column = 2, row = 3, sticky = E + W)
        
        
        # Ligne 4

        
        self.currentPoint = []
        currentPointLab = []
         
        self.currentPoint.append(StringVar(imagesFrame))
        currentPointLab.append(Label(imagesFrame, textvariable = self.currentPoint[0]))
        currentPointLab[0].grid(column = 0, row = 4)
        
        self.currentPoint.append(StringVar(imagesFrame))
        currentPointLab.append(Label(imagesFrame, textvariable = self.currentPoint[1]))
        currentPointLab[1].grid(column = 2, row = 4)
        
        
        ## Cadre 2 : Listes
        
        listsFrame = Frame(topFrame)
        listsFrame.pack(side = LEFT, fill = BOTH, expand = 1)

        
        # Ligne 0
        
        self.pointListFrame = []
        self.listCheckButVal = []
        self.listCheckBut = []

        self.listsCanvas = Canvas(listsFrame, height = 400)
        self.listsCanvas.grid(column = 0, row = 0, sticky = N + S + E + W)
        self.listsCanvasFrame = Frame(self.listsCanvas)
        self.listsCanvas.create_window(0, 0, anchor = N + W, window = self.listsCanvasFrame)
        
        self.pointListFrame.append(Frame(self.listsCanvasFrame, borderwidth = 2, relief = SUNKEN))
        self.pointListFrame[0].grid(column = 0, row = 0, sticky = N + S)
        
        self.listCheckButVal.append(IntVar(self.pointListFrame[0]))
        self.listCheckButVal[0].set(1)
        self.listCheckBut.append(Checkbutton(self.pointListFrame[0], text = "Points de l'image 1 :", \
                                             variable = self.listCheckButVal[0], command = self.image[0].list.ToggleAll))
        self.listCheckBut[0].config(state = DISABLED)
        self.listCheckBut[0].pack(side = TOP)


        self.pointListFrame.append(Frame(self.listsCanvasFrame, borderwidth = 2, relief = SUNKEN))
        self.pointListFrame[1].grid(column = 1, row = 0, sticky = N + S)
        
        self.listCheckButVal.append(IntVar(self.pointListFrame[1]))
        self.listCheckButVal[1].set(1)
        self.listCheckBut.append(Checkbutton(self.pointListFrame[1], text = "Points de l'image 2 :", \
                                             variable = self.listCheckButVal[1], command = self.image[1].list.ToggleAll))
        self.listCheckBut[1].config(state = DISABLED)
        self.listCheckBut[1].pack(side = TOP)
        
        
        self.removePtsFrame = Frame(self.listsCanvasFrame)
        self.removePtsFrame.grid(column = 2, row = 0, sticky = N + S)
        self.removePtsCheckButVal = IntVar(self.removePtsFrame)
        self.removePtsCheckButVal.set(0)
        self.removePtsCheckBut = Checkbutton(self.removePtsFrame, \
                                             variable = self.removePtsCheckButVal, command = self.imageList.ToggleAllTuples)
        self.removePtsCheckBut.config(state = DISABLED)
        self.removePtsCheckBut.pack(side = TOP)
        
        
        listsCanvasScroll = Scrollbar(listsFrame, orient = VERTICAL)
        self.listsCanvas.config(yscrollcommand = listsCanvasScroll.set)
        listsCanvasScroll.config(command =  self.listsCanvas.yview)
        listsCanvasScroll.grid(column = 1, row = 0, sticky = N + S)
        
        self.listsCanvasFrame.update_idletasks()
        self.listsCanvas.config(width = self.listsCanvasFrame.winfo_width())


        # Ligne 1

        removeButFrame = Frame(listsFrame)
        removeButFrame.grid(column = 0, row = 1)
        
        self.removeLastBut = Button(removeButFrame, text = "Supprimer le dernier", command = self.imageList.RemoveLastTuple)
        self.removeLastBut.pack(side = LEFT)
        self.removeLastBut.config(state = DISABLED)
        
        self.removeSelectBut = Button(removeButFrame, text = u"Supprimer la sélection", command = self.imageList.RemoveSelectedTuples)
        self.removeSelectBut.pack(side = LEFT)
        self.removeSelectBut.config(state = DISABLED)

        ### Bas
        
        bottomFrame = Frame(self)
        bottomFrame.pack(side = TOP, fill = BOTH, expand = 1)
        
        ## Cadre 3 : Champs
        
        formsFrame = Frame(bottomFrame)
        formsFrame.pack(side = LEFT, fill = BOTH, expand = 1)
        
        # Ligne 0
        
        
        focalMmLab = []

        focalMmLab.append(Label(formsFrame, text = "Focale 1 (mm)"))
        focalMmLab[0].grid(column = 0, row = 0)

        focalMmLab.append(Label(formsFrame, text = "Focale 2 (mm)"))
        focalMmLab[1].grid(column = 1, row = 0)


        # Ligne 1
    
    
        self.focalMmEnt = []
        
        self.focalMmEnt.append(Entry(formsFrame))
        self.focalMmEnt[0].insert(0, "0")
        self.focalMmEnt[0].grid(column = 0, row = 1, sticky = E + W)
        
        self.focalMmEnt.append(Entry(formsFrame))
        self.focalMmEnt[1].insert(0, "0")
        self.focalMmEnt[1].grid(column = 1, row = 1, sticky = E + W)


        # Ligne 2
        
        
        self.ccdLab = []
        self.ccdText = []
        
        self.ccdText.append(StringVar(formsFrame))
        self.ccdText[0].set("Largeur CCD 1 (mm)")
        self.ccdLab.append(Label(formsFrame, textvariable = self.ccdText[0]))
        self.ccdLab[0].bind("<Button-1>", self.image[0].CheckCamera)
        self.ccdLab[0].grid(column = 0, row = 2)
        
        self.ccdText.append(StringVar(formsFrame))
        self.ccdText[1].set("Largeur CCD 2 (mm)")
        self.ccdLab.append(Label(formsFrame, textvariable = self.ccdText[1]))
        self.ccdLab[1].bind("<Button-1>", self.image[1].CheckCamera)
        self.ccdLab[1].grid(column = 1, row = 2)


        # Ligne 3
        
        
        self.ccdEnt = []
        
        self.ccdEnt.append(Entry(formsFrame))
        self.ccdEnt[0].insert(0, "0")
        self.ccdEnt[0].grid(column = 0, row = 3, sticky = E + W)
        
        self.ccdEnt.append(Entry(formsFrame))
        self.ccdEnt[1].insert(0, "0")
        self.ccdEnt[1].grid(column = 1, row = 3, sticky = E + W)
        
        
        # Ligne 4
        
        
        focalPxBut = []
        
        focalPxBut.append(Button(formsFrame, text = "Focale 1 en pixel", command = self.image[0].GetFocalPx))
        focalPxBut[0].grid(column = 0, row = 4, sticky = E + W)

        focalPxBut.append(Button(formsFrame, text = "Focale 2 en pixel", command = self.image[1].GetFocalPx))
        focalPxBut[1].grid(column = 1, row = 4, sticky = E + W)


        # Ligne 5
        
        
        self.focalPxEnt = []
        
        self.focalPxEnt.append(Entry(formsFrame))
        self.focalPxEnt[0].insert(0, "0")
        self.focalPxEnt[0].config(state = DISABLED)
        self.focalPxEnt[0].grid(column = 0, row = 5)
        
        self.focalPxEnt.append(Entry(formsFrame))
        self.focalPxEnt[1].insert(0, "0")
        self.focalPxEnt[1].config(state = DISABLED)
        self.focalPxEnt[1].grid(column = 1, row = 5)
        
        # Ligne 6
        
        self.computeMatBut = Button(formsFrame, text = "Calculer les matrices", command = self.ComputeMat)
        self.computeMatBut.config(state = DISABLED)
        self.computeMatBut.grid(column = 0, row = 6, sticky = E + W)
        
        ## Cadre 4 : Matrices
        
        matrixFrame = Frame(bottomFrame)
        matrixFrame.pack(side = LEFT, fill = BOTH, expand = 1)
        
        # Ligne 0
        
        matrixLab = []

        matrixLab.append(Label(matrixFrame, text = "Matrice de projection 1 :"))
        matrixLab[0].grid(column = 0, row = 0)
        
        matrixLab.append(Label(matrixFrame, text = "Matrice de projection 2 :"))
        matrixLab[1].grid(column = 1, row = 0)
        
        # Ligne 1
        
        self.matrixPrint = []
        matrixPrintLab = []
        
        self.matrixPrint.append(StringVar(matrixFrame))
        self.matrixPrint[0].set("Inconnue")
        matrixPrintLab.append(Label(matrixFrame, textvariable = self.matrixPrint[0]))
        matrixPrintLab[0].config(borderwidth = 2, relief = SUNKEN, font = ("courier",))
        matrixPrintLab[0].grid(column = 0, row = 1)
        
        self.matrixPrint.append(StringVar(matrixFrame))
        self.matrixPrint[1].set("Inconnue")
        matrixPrintLab.append(Label(matrixFrame, textvariable = self.matrixPrint[1]))
        matrixPrintLab[1].config(borderwidth = 2, relief = SUNKEN, font = ("courier",))
        matrixPrintLab[1].grid(column = 1, row = 1)
        
        
        ## Cadre 5 : Boutons
        
        butFrame = Frame(bottomFrame)
        butFrame.pack(side = LEFT, fill = BOTH, expand = 1)
        
        # Ligne 0
        
        self.siftBut = Button(butFrame, text = u"Détecter les paires\n de points SIFT", command = self.SiftPairs)
        self.siftBut.grid(column = 0, row = 0, sticky = N + S + W + E)
        
        plotBut = Button(butFrame, text = u"Afficher les couples\nde points d'un CSV", command = self.PlotCSV)
        plotBut.grid(column = 1, row = 0, sticky = N + S + W + E)

        
        # Ligne 1
        
        self.improveBut = Button(butFrame, text = u"Améliorer les paires", command = self.Improve)
        self.improveBut.grid(column = 0, row = 1, sticky = N + S + W + E)

        clearBut = Button(butFrame, text = u"Effacer les paires", command = self.imageList.ClearTuples)
        clearBut.grid(column = 1, row = 1, sticky = N + S + W + E)


        
        # Ligne 2

        self.rectBut = Button(butFrame, text = u"Rectification", command = self.Rect)
        self.rectBut.config(state = DISABLED)
        self.rectBut.grid(column = 0, row = 2, sticky = N + S + W + E)
        
        self.correspBut = Button(butFrame, text = u"Chercher les correspondances\n(pour images réctifiées)", command = self.Corresp)
        self.correspBut.grid(column = 1, row = 2, sticky = N + S + W + E)
        
        # Ligne 3
        
    
        savePointsBut = Button(butFrame, text = u"Sauvegarder les couples\nde points dans un CSV", command = self.SavePoints)
        savePointsBut.grid(column = 0, row = 3, sticky = N + S + W + E)
        
        saveBut = Button(butFrame, text = u"Sauvegarder les images\navec les points", command = self.SaveCanvas)
        saveBut.grid(column = 1, row = 3, sticky = N + S + W + E)
        
        # Ligne 4
        
        self.plot3dBut = Button(butFrame, text = u"Afficher/Sauvegarder\nles points 3D", command = self.Plot3d)
        self.plot3dBut.config(state = DISABLED)
        self.plot3dBut.grid(column = 0, row = 4, sticky = N + S + W + E)
        
        self.plotDepthBut = Button(butFrame, text = u"Afficher la profondeur\n(pour images rectifiées)", command = self.PlotDepth)
        self.plotDepthBut.grid(column = 1, row = 4, sticky = N + S + W + E)
        
        
    def PrintMat(self, mat):
        if mat == None:
            return "Inconnue"
        else:
            m, n = mat.shape
            colWidths = [max(map(lambda x: len(("%.f" % x[0,0])), mat[:,j])) for j in range(0, n)]
            return "\n".join([" ".join([("%.f" % mat[i,j]).ljust(colWidths[j]) for j in range(0, n)]) for i in range(0, m)])
    
    def ComputeMat(self):
        image1 = self.image[0]
        image2 = self.image[1]
        f1 = eval(self.focalPxEnt[0].get())
        f2 = eval(self.focalPxEnt[1].get())
        c1 = (image1.size[0]/2., image1.size[1]/2.)
        c2 = (image2.size[0]/2., image2.size[1]/2.)
        p1 = image1.GetList()
        p2 = image2.GetList()
        mainWindow.matrices = geom.matrices(f1, f2, c1, c2, p1, p2)
        self.image[0].SetMatrix(mainWindow.matrices.P1, mainWindow.matrices.K1)
        self.image[1].SetMatrix(mainWindow.matrices.P2, mainWindow.matrices.K2)
        self.rectBut.config(state = NORMAL)
        self.plot3dBut.config(state = NORMAL)
        
        
    def SiftPairs(self):
        pySiftWindow = PySiftWindow(None)
        pySiftWindow.wm_iconbitmap("logoECP.ico")
        pySiftWindow.title(u"Points SIFT")
        pySiftWindow.mainloop()

        
    def Plot3d(self):
        p1 = self.image[0].GetList()
        p2 = self.image[1].GetList()
            
        points3d = reconst3d.reconst3dPoints(p1, p2, self.image[0].P,  self.image[1].P, self.image[0].K, self.image[1].K)
        
        csvPath = tkFileDialog.asksaveasfilename(filetypes = [("Feuille CSV", "*.csv")])
        if not csvPath == "":
            csvFile = open(csvPath, "w")
            csvSheet = csv.writer(csvFile)
            for i in range(0, len(points3d)):
                csvSheet.writerow([int(points3d[i][0]),int(points3d[i][1]),int(points3d[i][2])])
        plotWindow = threadProc(lambda:reconst3d.drawSurface(points3d))
        plotWindow.start()
        
        
    def PlotDepth(self):
        p1 = self.image[0].GetList()
        p2 = self.image[1].GetList()
            
        reconst3d.rectDepthMap(p1, p2)#, self.image[0].K)
        

    def CheckFields(self, event = None):
        l0 = self.image[0].list.Size()
        l1 = self.image[1].list.Size()
        if l0 == l1 and l0 >= 8 \
        and float(eval(self.focalPxEnt[0].get())) > 0 and float(eval(self.focalPxEnt[1].get())) > 0 \
        and self.image[0].size[0] > 0 and self.image[1].size[0] > 0:
            self.computeMatBut.config(state = NORMAL)
        else:
            self.computeMatBut.config(state = DISABLED)
            
    def Improve(self):
        improveWindow = ImproveWindow(None)
        improveWindow.wm_iconbitmap("logoECP.ico")
        improveWindow.title(u"Amélioration des paires")
        improveWindow.mainloop()
        
            
    def Rect(self):
        rectWindow = RectWindow(None)
        rectWindow.wm_iconbitmap("logoECP.ico")
        rectWindow.title(u"Rectification")
        rectWindow.mainloop()

    def Corresp(self):
        correspWindow = CorrespWindow(None)
        correspWindow.wm_iconbitmap("logoECP.ico")
        correspWindow.title(u"Recherche des correspondances")
        correspWindow.mainloop()
        
    def SavePoints(self):
        p1 = self.image[0].GetList()
        p2 = self.image[1].GetList()
        csvPath = tkFileDialog.asksaveasfilename(filetypes = [("Feuille CSV", "*.csv")])
        if not csvPath == "":
            csvFile = open(csvPath, "w")
            csvSheet = csv.writer(csvFile)
            for i in range(0, min(len(p1),len(p2))):
                csvSheet.writerow([int(p1[i][0]),int(p1[i][1]),int(p2[i][0]),int(p2[i][1])])
     
        
    def PlotPairs(self, pairs):
        for pair in pairs:
            x1, y1, x2, y2 = pair
            self.image[0].list.AddPoint(x1, y1)
            self.image[1].list.AddPoint(x2, y2)


    def PlotCSV(self):
        csvPath = tkFileDialog.askopenfilename(filetypes = [("Feuille CSV", "*.csv")])
        if not csvPath == "":
            csvFile = open(csvPath, "rb")
            csvSheet = csv.reader(csvFile)
            for row in csvSheet:

                x1 = int(row[0])
                y1 = int(row[1])
                x2 = int(row[2])
                y2 = int(row[3])
            
                self.image[0].list.AddPoint(x1, y1, False)          
                self.image[1].list.AddPoint(x2, y2, False)
            

    def SaveCanvas(self):
        psPath = []
        psPath.append(tkFileDialog.asksaveasfilename(filetypes = [("Fichier PostScript", "*.ps")]))
        psPath.append(tkFileDialog.asksaveasfilename(filetypes = [("Fichier PostScript", "*.ps")]))
        if not "" in psPath:
            self.canvas[0].postscript(file = psPath[0], x = 0, y = 0, height = self.image[0].size[1], width = self.image[0].size[0])
            self.canvas[1].postscript(file = psPath[1], x = 0, y = 0, height = self.image[1].size[1], width = self.image[1].size[0])

class threadProc(threading.Thread):
    def __init__(self, command):
        threading.Thread.__init__(self)
        self.command = command
        
    def run(self):
        self.command()

class ImproveWindow(Tk):
    def __init__(self, parent):
        Tk.__init__(self, parent)
        self.tk.call("package", "require", "tile")
        self.tk.call("namespace", "import", "-force", "ttk::*")
        self.tk.call("ttk::setTheme", "xpnative")
        
        self.grid()
        
        # Ligne 0
        
        titleLab = Label(self, text = u"Amélioration des paires")
        titleLab.grid(column = 0, row = 0)
        
        # Ligne 1
        
        self.progressLab = Label(self)
        self.progressLab.grid(column = 0, row = 1)
        
        # Ligne 2
        
        self.cancelBut = Button(self, text = "Annuler", command = self.destroy)
        self.cancelBut.grid(column = 0, row = 2)
        
        self.Submit()
        
    class Prog:
        def __init__(self):
            self.status = 0.
            
        def PercentStat(self):
            return self.status / mainWindow.image[1].list.Size()
            
        def ImprovePairs(self):
            for n in range(0, mainWindow.image[1].list.Size()):
                self.status += 1
                mainWindow.image[1].ImprovePoint(n, mainWindow.image[0].image, mainWindow.image[0].list.l[n])
    
    def Submit(self):
        
        self.prog = self.Prog()
        self.progThread = threadProc(self.prog.ImprovePairs)
        self.progThread.start()
        
        self.monitThread = threadProc(self.CheckStatus)
        self.monitThread.start()
        
        
    def CheckStatus(self):
        self.progressLab.config(text = u"%.2f%% effectués" % (100*self.prog.PercentStat()))
        if self.progThread.isAlive():
            self.progressLab.after(200, self.CheckStatus)
        else:
            self.destroy()
        
class RectWindow(Tk):
    def __init__(self, parent):
        Tk.__init__(self, parent)
        self.tk.call("package", "require", "tile")
        self.tk.call("namespace", "import", "-force", "ttk::*")
        self.tk.call("ttk::setTheme", "xpnative")

        self.grid()
        
        # Ligne 0
        
        titleLab = Label(self, text = u"Rectification des photos")
        titleLab.grid(column = 0, row = 0)
        
        # Ligne 1
        
        self.saveImages = IntVar(self)
        self.saveImages.set(0)
        saveImagesCheck = Checkbutton(self, text = u"Sauvegarder les images réctifiées", variable = self.saveImages)
        saveImagesCheck.grid(column = 0, row = 1)
        
        # Ligne 2
        
        self.progressLab = Label(self)
        self.progressLab.grid(column = 0, row = 2)
        
        # Ligne 3
        
        self.cancelBut = Button(self, text = "Annuler", command = self.destroy)
        self.cancelBut.grid(column = 0, row = 3)
        
        self.Submit()
        
    def Submit(self):
        
        image1 = mainWindow.image[0]
        image2 = mainWindow.image[1]
        c1 = (image1.K[0,2], image1.K[1,2])
        c2 = (image2.K[0,2], image2.K[1,2])

        self.prog1 = rectification.Rectification(image1.image, mainWindow.matrices.T1, c1)
        self.prog2 = rectification.Rectification(image2.image, mainWindow.matrices.T2, c2)
        
        self.progThread1 = threadProc(self.prog1.rectify)
        self.progThread2 = threadProc(self.prog2.rectify)
        self.progThread1.start()
        self.progThread2.start()
        
        self.monitThread = threadProc(self.CheckStatus)
        self.monitThread.start()
        
        
    def CheckStatus(self):
        totalProg = (self.prog1.PercentStat() + self.prog2.PercentStat())/2.
        self.progressLab.config(text = u"%.2f%% effectués" % (100*totalProg))
        if self.progThread1.isAlive() or self.progThread2.isAlive():
            self.progressLab.after(200, self.CheckStatus)
        else:
            self.cancelBut.config(state = DISABLED)
            image1 = mainWindow.image[0]
            image2 = mainWindow.image[1]
            image1.LoadImage(self.prog1.imageRect)
            image2.LoadImage(self.prog2.imageRect)
            image1.SetMatrix(mainWindow.matrices.Pn1, mainWindow.matrices.Kn)
            image2.SetMatrix(mainWindow.matrices.Pn2, mainWindow.matrices.Kn)
            if self.saveImages.get() == 1:
                path = []
                path.append(tkFileDialog.asksaveasfilename(filetypes = [("Image", "*.*")]))
                path.append(tkFileDialog.asksaveasfilename(filetypes = [("Image", "*.*")]))
                if not "" in path:
                    mainWindow.image[0].image.save(path[0])
                    mainWindow.image[1].image.save(path[1])
            mainWindow.imageList.ClearTuples()
            self.destroy()

class CorrespWindow(Tk):
    def __init__(self, parent):
        Tk.__init__(self, parent)
        self.tk.call("package", "require", "tile")
        self.tk.call("namespace", "import", "-force", "ttk::*")
        self.tk.call("ttk::setTheme", "xpnative")

        self.grid()
        
        # Ligne 0
        
        titleLab = Label(self, text = u"Recherche des correspondances sur les images réctifiées")
        titleLab.grid(column = 0, row = 0)
        
        self.submitBut = Button(self, text = u"Démarrer", command = self.Submit)
        self.submitBut.grid(column = 1, row = 0)
        
        # Ligne 1
        
        self.searchTypeVar = IntVar(self)
        self.searchTypeVar.set(0)
        
        searchTypeRadio0 = Radiobutton(self, text = "Recherche sur tous les points", variable = self.searchTypeVar, value = 0, command = self.SetSearchType)
        searchTypeRadio0.grid(column = 0, row = 1)
        
        # Ligne 2
        
        searchTypeRadio1 = Radiobutton(self, text = "Recherche sur les contours", variable = self.searchTypeVar, value = 1, command = self.SetSearchType)
        searchTypeRadio1.grid(column = 0, row = 2)
        
        # Ligne 3
        
        thresholdLab = Label(self, text = u"Seuil des contours (de 0 à 255)")
        thresholdLab.grid(column = 0, row = 3)
        
        self.thresholdEnt = Entry(self)
        self.thresholdEnt.insert(0, "150")
        self.thresholdEnt.grid(column = 1, row = 3)
        
        # Ligne 4
        
        radiusLab = Label(self, text = u"Rayon du patch unitaire (en pixels)")
        radiusLab.grid(column = 0, row = 4)
        
        self.radiusEnt = Entry(self)
        self.radiusEnt.insert(0, "2")
        self.radiusEnt.grid(column = 1, row = 4)
        
        # Ligne 5
        
        minCorrelLab = Label(self, text = u"Correlation minimale acceptable (en %)")
        minCorrelLab.grid(column = 0, row = 5)
        
        self.minCorrelEnt = Entry(self)
        self.minCorrelEnt.insert(0, "95")
        self.minCorrelEnt.grid(column = 1, row = 5)
        
        # Ligne 6
        
        minSharpnessLab = Label(self, text = u"Acuité minimale acceptable pour le maximum de correlation (en %)")
        minSharpnessLab.grid(column = 0, row = 6)
        
        self.minSharpnessEnt = Entry(self)
        self.minSharpnessEnt.insert(0, "10")
        self.minSharpnessEnt.grid(column = 1, row = 6)
        
        # Ligne 7
        
        maxHorShiftLab = Label(self, text = u"Décalage relatif maximal entre les images réctifiées")
        maxHorShiftLab.grid(column = 0, row = 7)
        
        self.maxHorShiftEnt = Entry(self)
        self.maxHorShiftEnt.insert(0, "0.5")
        self.maxHorShiftEnt.grid(column = 1, row = 7)
        
        # Ligne 8
        
        threadNbLab = Label(self, text = u"Nombre de threads simultanés")
        threadNbLab.grid(column = 0, row = 8)
        
        self.threadNbEnt = Entry(self)
        self.threadNbEnt.insert(0, "1")
        self.threadNbEnt.grid(column = 1, row = 8)
        
        # Ligne 9
        
        gridLenLab = Label(self, text = u"Largeur du maillage (en pixels)")
        gridLenLab.grid(column = 0, row = 9)
        
        self.gridLenEnt = Entry(self)
        self.gridLenEnt.insert(0, "1")
        self.gridLenEnt.grid(column = 1, row = 9)
        
        # Ligne 10
        
        self.progressLab = Label(self)
        self.progressLab.grid(column = 0, row = 10)
        self.progressLab.grid_remove()
        
        # Ligne 11
        
        self.cancelBut = Button(self, text = "Annuler", command = self.destroy)
        self.cancelBut.config(state = DISABLED)
        self.cancelBut.grid(column = 0, row = 11)
        
        self.plotBut = Button(self, text = "Afficher les couples déja trouvés", command = self.PlotCurrent)
        self.plotBut.config(state = DISABLED)
        self.plotBut.grid(column = 1, row = 11)
        
        self.SetSearchType()
        
        
    def SetSearchType(self):
        if self.searchTypeVar.get() == 1:
            self.gridLenEnt.config(state = DISABLED)
            self.maxHorShiftEnt.config(state = DISABLED)
            self.thresholdEnt.config(state = NORMAL)
            
        else:
            self.gridLenEnt.config(state = NORMAL)
            self.maxHorShiftEnt.config(state = NORMAL)
            self.thresholdEnt.config(state = DISABLED)
        

    def Submit(self):
        
        self.submitBut.config(state = DISABLED)
        self.cancelBut.config(state = NORMAL)
        
        radius = int(eval(self.radiusEnt.get()))
        minCorrel = float(eval(self.minCorrelEnt.get()))/100.
        minSharpness = float(eval(self.minSharpnessEnt.get()))/100.
        maxHorShift = float(eval(self.maxHorShiftEnt.get()))
        edgeThreshold = int(eval(self.thresholdEnt.get()))
        self.threadNb = int(eval(self.threadNbEnt.get()))
        gridLen = int(eval(self.gridLenEnt.get()))

        image0 = mainWindow.image[0].image
        image1 = mainWindow.image[1].image
        
        self.progressLab.grid()
        
        self.progs = []
        self.progThreads = []
        images0 = []
        images1 = []
        
        for i in range(0, self.threadNb):
            images0.append(image0.crop((0, int(round(float(i)*image0.size[1]/self.threadNb)), image0.size[0], int(round(float(i+1)*image0.size[1]/self.threadNb)))))
            images1.append(image1.crop((0, int(round(float(i)*image1.size[1]/self.threadNb)), image1.size[0], int(round(float(i+1)*image1.size[1]/self.threadNb)))))
            self.progs.append(corresppoints.CorrespPoints(images0[i], images1[i], radius, minCorrel, minSharpness)) #, maxHorShift, gridLen, 100))
            if self.searchTypeVar.get() == 1:
                self.progThreads.append(threadProc((lambda:self.progs[i].FastFindPairs(edgeThreshold))))
            else:
                self.progThreads.append(threadProc((lambda:self.progs[i].FindPairs(maxHorShift, gridLen))))                
            self.progThreads[i].start()

        self.plotBut.config(state = NORMAL)
        
        self.monitThread = threadProc(self.CheckStatus)
        self.monitThread.start()

        
    def CheckStatus(self):
        totalProg = 0.
        for i in range(0, self.threadNb):
            totalProg += self.progs[i].PercentStat()
        totalProg = totalProg/self.threadNb
        self.progressLab.config(text = u"%.2f%% effectués" % (100*totalProg))
        
        alive = False
        for i in range(0, self.threadNb):
            alive= alive or self.progThreads[i].isAlive()
            
        if alive:
            self.progressLab.after(200, self.CheckStatus)

        else:
            self.PlotCurrent()
            self.destroy()

            
    def PlotCurrent(self):
            #pairs = []
            disparityMaps = [prog.disparities for prog in self.progs]
            disparities = concatenate(disparityMaps)
            disparities[disparities < 0] = 0
            disparities *= 255 / disparities.max()
            h, w = disparities.shape
            image = Image.new("L", (w, h))
            image.putdata(disparities.ravel())
            mainWindow.image[0].LoadImage(image)
            mainWindow.image[1].UnloadImage()
            #    pairs.append(prog.LastFoundPairs())
            #mainWindow.PlotPairs(self.MergePairs(pairs))        
            
            
    def MergePairs(self, pairsList):
        pairs = []
        h = mainWindow.image[0].size[1]
        n = len(pairsList)
        for i in range(0, n):
            for pair in pairsList[i]:
                x1, y1, x2, y2 = pair
                pairs.append((x1, y1 + int(round(float(i)*h/n)), x2, y2 + int(round(float(i)*h/n))))
        return pairs
    
class PySiftWindow(Tk):
    def __init__(self, parent):
        Tk.__init__(self, parent)
        self.tk.call("package", "require", "tile")
        self.tk.call("namespace", "import", "-force", "ttk::*")
        self.tk.call("ttk::setTheme", "xpnative")

        self.grid()
        
        # Ligne 0
        
        titleLab = Label(self, text = u"Recherche des paires de points SIFT")
        titleLab.grid(column = 0, row = 0)
        
        self.submitBut = Button(self, text = u"Démarrer", command = self.Submit)
        self.submitBut.grid(column = 1, row = 0)
    
        
        # Ligne 1
        
        minCorrelLab = Label(self, text = u"Correlation minimale acceptable (en %)")
        minCorrelLab.grid(column = 0, row = 1)
        
        self.minCorrelEnt = Entry(self)
        self.minCorrelEnt.insert(0, "95")
        self.minCorrelEnt.grid(column = 1, row = 1)
        
        # Ligne 2
        
        minSharpnessLab = Label(self, text = u"Acuité minimale acceptable pour le maximum de correlation (en %)")
        minSharpnessLab.grid(column = 0, row =2)
        
        self.minSharpnessEnt = Entry(self)
        self.minSharpnessEnt.insert(0, "10")
        self.minSharpnessEnt.grid(column = 1, row = 2)
        
        # Ligne 3
        
        self.progressLab = Label(self)
        self.progressLab.grid(column = 0, row = 3)
        
        # Ligne 4
        
        self.cancelBut = Button(self, text = "Annuler", command = self.destroy)
        self.cancelBut.grid(column = 0, row = 4)
        
        self.plotBut = Button(self, text = "Afficher les couples déja trouvés", command = self.PlotCurrent)
        self.plotBut.grid(column = 1, row = 4)
        
    def Submit(self):
        
        minCorrel = float(eval(self.minCorrelEnt.get()))/100.
        minSharpness = float(eval(self.minSharpnessEnt.get()))/100.
        
        self.prog = corresppoints.pySiftPairs(mainWindow.image[0].image, mainWindow.image[1].image, minCorrel, minSharpness)
        self.progThread = threadProc(self.prog.FindPairs)
        self.progThread .start()
        
        self.monitThread = threadProc(self.CheckStatus)
        self.monitThread.start()
        
        
    def CheckStatus(self):
        self.progressLab.config(text = u"%.2f%% effectués" % (100 * self.prog.PercentStat()))
        if self.progThread.isAlive():
            self.progressLab.after(200, self.CheckStatus)
        else:
            self.PlotCurrent()
            self.destroy()
        
    def PlotCurrent(self):
        mainWindow.PlotPairs(self.prog.LastFoundPairs())
        
        
if __name__ == "__main__":
    mainWindow = mainWindow(None)
    mainWindow.wm_iconbitmap("logoECP.ico")
    mainWindow.title(u"Stéréovision")
    mainWindow.mainloop()


