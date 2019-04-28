# coding=UTF-8

import win32com.client
import pywintypes
import pg
import sys
import os
import time
from codecs import open
import chardet
import re


global dataBase, dataDir, log

def frenchDateToSql(date):
    if not isblank(date):
        date = str(date).split()[0];
        numbers = date.split("/")
        return "%s-%s-%s" % (numbers[2], numbers[1], numbers[0])
    else:
        return ""
    
def coordsBoxToSql(coordBox):
    if not isblank(coordBox):
        regexp = re.compile('\d+')
        coords = regexp.findall(coordBox)
        if len(coords) == 2:
            return "((%s, %s), (%s, %s))" % (coords[0], coords[1], coords[0], coords[1])
        elif len(coords) == 4:
            return "((%s, %s), (%s, %s))" % (coords[0], coords[1], coords[2], coords[3])
        else:
            return ""
    else:
        return ""
        
def isblank(string):
    return string.strip() == ""
    
def printTime():
    year, month, day, hour, min, sec, _, _, _ = time.localtime()
    return "%i-%i-%i_%i-%i-%i" % (year, month, day, hour, min, sec)
    
def toUnicode(text):
    if isinstance(text, unicode):
        return text
    else:
        try:
            encoding = chardet.detect(text)["encoding"]
        except:
            return str(text)
        else:
            return unicode(text, encoding)
    
class Log:
    def __init__(self, quiet = False):
        self.quiet = quiet
        if not os.path.isdir("logs"):
            os.mkdir("logs")
        self.filePath = os.path.join("logs", "%s.log" % printTime())
        self.logFile = open(self.filePath, "w", "UTF-8")
        
    def __call__(self, message):
        self.logFile = open(self.filePath, "a", "UTF-8")
        ligne = u"%s : %s\r\r" % (printTime(), message)
        self.logFile.write(ligne)
        self.logFile.close()
        if not self.quiet:
            print ligne.encode(sys.getdefaultencoding(), "replace")
        
    def Close(self):
        ligne = u"%s : Terminé" % printTime()
        self.logFile.write(ligne)
        if not self.quiet:
            print ligne.encode(sys.getdefaultencoding(), "replace")
        self.logFile.close()

class DataBase:
    def __init__(self, host, base, user , password):
        try:
            self.connection = pg.connect (dbname = base, host = host,  user = user, passwd = password)
            
        except pg.InternalError, err:
            message = u"Impossible de se connecter à la base PostGreSQL %s/%s\rErreur : %s" % (host, base, err)
            log(message)
            sys.exit(1)
        log(u"Connecté à la base %s/%s" % (host, base))
        
        
            
    def Execute(self, query):
        
        try:
            sql = query.encode("utf-8")
            search = self.connection.query(sql)
            if hasattr(search, "dictresult"):
                return search.dictresult()
            else:
                return search
        except pg.ProgrammingError, err:
            message = u"Erreur lors de la requête :\r  %s  \nErreur : %s" % (query, toUnicode(err.message))
            log(message)
            sys.exit (1)
        except pg.InternalError, err:
            message = u"Erreur lors de la requête :\r  %s  \nErreur : %s" % (query, toUnicode(err.message))
            log(message)
            sys.exit (1)
        
        
    def Close(self):
        self.connection.close()
        

class SpreadSheet:
    def __init__(self, path):
        try:
            self.xlApp = win32com.client.Dispatch("Excel.Application")
            self.xlApp.Visible = False
            self.xlWb = self.xlApp.Workbooks.Open(os.path.abspath(path))
        
        except pywintypes.com_error, err:
            message = u"Impossible d'ouvrir la feuille :\r  %s  \nErreur %d: %s" % (toUnicode(os.path.abspath(path)), err[0], err[1]);
            log(message)
            sys.exit(1)
        log(u"Feuille Excel '%s' ouverte" % toUnicode(os.path.abspath(path)))
            
    def CountRows(self):
        try:
            return self.xlWb.ActiveSheet.UsedRange.Rows.Count
        except pywintypes.com_error, err:
            message = u"Impossible de compter les lignes\nErreur %d: %s" % (err[0], err[1]);
            log(message)
            sys.exit(1)

    def CountCols(self):
        try :
            return self.xlWb.ActiveSheet.UsedRange.Columns.Count
        except pywintypes.com_error, err:
            message = u"Impossible de compter les colonnes\nErreur %d: %s" % (err[0], err[1]);
            log(message)
            sys.exit(1)
    
    def GetCell(self, row, col):
        try:
            cell = toUnicode(self.xlWb.ActiveSheet.Cells(row + 1, col + 1).Value)
            if cell.lower() == "none" or isblank(cell):
                return ""
            else:
                return toUnicode(cell)
        except pywintypes.com_error, err:
            message = u"Impossible de lire la cellule (%i, %i)\rErreur %d: %s" % (row, col, err[0], err[1]);
            log(message)
            sys.exit(1)
        
    def Close(self):
        self.xlWb.Close(SaveChanges = 0)
        self.xlApp.Quit()
     
        
class DbEntry:
    def __init__(self, tableName, values, idKey = None):
        
        self.name = tableName
        self.idKey = idKey
        
        query = u"SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name = '%s';" % self.name
        result = dataBase.Execute(query)
        self.values = {}.fromkeys(map(lambda x: x["column_name"], result))
        
        if (values.has_key(self.idKey)):
            query = u"SELECT * FROM %s WHERE %s = %i;" % (self.name, self.idKey, values[self.idKey])
            result = dataBase.Execute(query)
            for key in result[0].keys():
                self.values[key] = toUnicode(result[0][key])
        
        else:
            criteria = u" AND ".join(map(lambda (x, y): u"%s = '%s'" % (x, y), zip(values.keys(), values.values())))
            query = u"SELECT * FROM %s WHERE %s;" % (self.name, criteria)
            result = dataBase.Execute(query)
            if len(result) > 0:
                log(u"Ligne déja ajoutée à %s : %s" % (self.name, query))
                for key in result[0].keys():
                    self.values[key] = toUnicode(result[0][key])

            else:
                keys = u"(%s)" % ", ".join(values.keys())
                vals = u"(%s)" % ", ".join(map(lambda val: "'%s'" % val, values.values()))
                if self.values.has_key(self.idKey):
                    query = u"INSERT INTO %s %s VALUES %s RETURNING %s;" % (self.name, keys, vals, self.idKey)
                    result = dataBase.Execute(query)
                    self.values[self.idKey] = result[0][self.idKey]
                else:
                    query = u"INSERT INTO %s %s VALUES %s;" % (self.name, keys, vals)
                    dataBase.Execute(query) 
                log(u"Ligne ajoutée à %s : %s" % (self.name, query))
                for key in values.keys():
                    self.values[key] = toUnicode(values[key])
            
    def GetValues(self):
        if self.values.has_key(self.idKey):
            query = u"SELECT * FROM %s WHERE %s = %s;" % (self.name, self.idKey, self.values[self.idKey])
            result = dataBase.Execute(query)
            for key in result[0].keys():
                    self.values[key] = toUnicode(result[0][key])
            return self.values
        
    def SetValues(self, values):
        if (self.values.has_key(self.idKey)):
            setlist = ", ".join([u"%s = '%s'" % (key, value) for key, value in values.items()])
            query = u"UPDATE %s SET %s WHERE '%s' = %i;" % (self.name, setlist, self.idKey, self.values[self.idKey])
            dataBase.Execute(query)
            self.GetValues()
            
            
class Building(DbEntry):
    def __init__(self, values):
        DbEntry.__init__(self, "building", values, "b_id")


class Architect(DbEntry):
    def __init__(self, values):
        DbEntry.__init__(self, "architect", values)
    
    
class Address(DbEntry):
    def __init__(self, values):
        DbEntry.__init__(self, "address", values, "ad_id")
        
        
class Image(DbEntry):
    def __init__(self, values):
        DbEntry.__init__(self, "image", values, "i_id")
        
        
class ArchitecturalElement(DbEntry):
    def __init__(self, values):
        DbEntry.__init__(self, "architectural_element", values, "e_id")
        
        
class ImageOfBuilding(DbEntry):
    def __init__(self, values):
        DbEntry.__init__(self, "image_of_building", values)
        
        
class ImageOfElement(DbEntry):
    def __init__(self, values):
        DbEntry.__init__(self, "image_of_element", values)
        
class ElementOfBuilding(DbEntry):
    def __init__(self, values):
        DbEntry.__init__(self, "element_of_building", values)
        

log = Log()
dataDir = os.path.dirname(os.path.abspath(sys.argv[0]))
dataBase = DataBase("localhost", "Archi", "postgres", "azerty")

for dir in os.listdir(dataDir):
    if os.path.isdir(dir) and not dir == "logs":
        
        message =  u"Ouverture du dossier : '%s'" % toUnicode(os.path.abspath(dir))
        log(message)
        
        for file in os.listdir(dir):
            if os.path.isfile(os.path.join(dir, file)) and os.path.splitext(file)[1].lower() in[".xls", ".xlsx"] and not file[0] == "~":
                
                spreadSheet = SpreadSheet(os.path.join(dir, file))
                
                date = frenchDateToSql(spreadSheet.GetCell(0, 1).strip())
                style = spreadSheet.GetCell(1, 1).lower().strip()
                architect = spreadSheet.GetCell(2, 1).lower().strip()
                address1 = spreadSheet.GetCell(4, 1).lower().strip()
                district = spreadSheet.GetCell(5, 1).lower().strip()
                address2 = spreadSheet.GetCell(6, 1).lower().strip()
                
                
                buildingValues = {}
                if not isblank(style):
                    buildingValues["b_style"] = style
                buildingEntry = None
                buildingEntry = Building(buildingValues)
                
                print buildingEntry.values
                
                if not isblank(address1):
                    address1Values = {}
                    address1Values["b_id"] = buildingEntry.GetValues()["b_id"]
                    address1Parts = address1.split(" ");
                    address1Values["ad_street_num"] = address1Parts[0]
                    address1Values["ad_street"] = " ".join(address1Parts[1:])
                    address1Values["ad_city"] = "Paris"
                    if not isblank(district):
                        address1Values["ad_postal_code"] = int(75000 + eval(district))
                    address1Values["ad_country"] = "France"
                    address1Entry = None
                    address1Entry = Address(address1Values)
                    
                if not isblank(address2):
                    address2Values = {}
                    address2Values["b_id"] = building.GetValues()["b_id"]
                    address2Parts = address2.split(" ");
                    address2Values["ad_street_num"] = address2Parts[0]
                    address2Values["ad_street"] = " ".join(address2Parts[1:])
                    address2Values["ad_city"] = "Paris"
                    if not isblank(district):
                        address2Values["ad_postal_code"] = int(75000 + eval(district))
                    address2Values["ad_country"] = "France"
                    address2Entry = None
                    address2Entry = Address(address2Values)
                    
                if not isblank(architect):
                    architectValues = {}
                    architectValues["a_name"] = architect
                    architectValues["b_id"] = buildingEntry.GetValues()["b_id"]
                    architectEntry = None
                    architectEntry = Architect(architectValues)
                
                for i in range(8, spreadSheet.CountRows()):
                    
                    image = spreadSheet.GetCell(i, 0).strip()
                    elementName = spreadSheet.GetCell(i, 1).lower().strip()
                    elementDetails = spreadSheet.GetCell(i, 3).lower().strip()
                    elementCoords = coordsBoxToSql(spreadSheet.GetCell(i, 2).strip())
                    
                    if not isblank(image):
                        imageValues = {}
                        imageValues["i_path"] = "%s.jpg" % image
                        if not isblank(date):
                            imageValues["i_shooting_date"] = date
                        imageEntry = None
                        imageEntry = Image(imageValues)
                        
                        imageOfBuildingValues = {}
                        imageOfBuildingValues["b_id"] = buildingEntry.GetValues()["b_id"]
                        imageOfBuildingValues["i_id"] = imageEntry.GetValues()["i_id"]
                        imageOfBuildingEntry = ImageOfBuilding(imageOfBuildingValues)
                        
                        if not isblank(elementName):
                            elementValues = {}
                            elementValues["e_name"] = elementName
                            if not isblank(style):
                                elementValues["e_style"] = style
                            if not isblank(elementDetails):
                                elementValues["e_details"] = elementDetails
                            elementEntry = None
                            elementEntry = ArchitecturalElement(elementValues)
                            
                            imageOfElementValues = {}
                            imageOfElementValues["i_id"] = imageEntry.GetValues()["i_id"]
                            imageOfElementValues["e_id"] = elementEntry.GetValues()["e_id"]
                            if not isblank(elementCoords):
                                imageOfElementValues["ioe_roi"] = elementCoords
                            imageOfElementEntry = None
                            imageOfElementEntry = ImageOfElement(imageOfElementValues)
                            
                            elementOfBuildingValues = {}
                            elementOfBuildingValues["e_id"] = elementEntry.GetValues()["e_id"]
                            elementOfBuildingValues["b_id"] = buildingEntry.GetValues()["b_id"]
                            elementOfBuildingEntry = None
                            elementOfBuildingEntry = ElementOfBuilding(elementOfBuildingValues)
                                
                spreadSheet.Close()

dataBase.Close()
log.close()

