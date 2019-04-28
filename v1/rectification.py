from numpy import array, matrix, ones, mgrid, concatenate
from scipy.ndimage import map_coordinates
from PIL import Image

class Rectification:
    def __init__(self, image, T, c):
        self.status = 0.
        self.T = T
        self.c = c
        self.image = image
        self.nbOp = 1.
        self.imageRect = None

    def PercentStat(self):
        return self.status / self.nbOp
    
    def rectify(self):

        bands = self.image.split()
        nBands = len(bands)
        self.nbOp = nBands + 2.
        cols, rows = self.image.size
        cx, cy = self.c
        ncols, nrows, unknownPoints = setupRect(self.T, rows, cols, max(2 * cx, 2 * cy))
        
        
        self.status += 1.
        
        newBands = []
        for i in range(0, nBands):
            knownPoints = array(bands[i].getdata(), dtype = float).reshape((rows, cols)).T
            interpVals = map_coordinates(knownPoints, unknownPoints)
            newBands.append(Image.new("L", (ncols, nrows)))
            newBands[i].putdata(interpVals)
            
            self.status += 1.
        
        self.imageRect = Image.merge(self.image.mode, tuple(newBands))

        
        if 2 * cx < ncols:
            ncols, nrows = self.imageRect.size
            left = int(round((ncols / 2.) - cx))
            right = int(round((ncols / 2.) + cx))
            self.imageRect = self.imageRect.crop((left, 0, right, nrows))

        if 2 * cy < nrows:
            ncols, nrows = self.imageRect.size
            upper = int(round((nrows / 2.) - cy))
            lower = int(round((nrows / 2.) + cy))
            self.imageRect = self.imageRect.crop((0, upper, ncols, lower))
            
        if 2 * cx > ncols:
            ncols, nrows = self.imageRect.size
            black = Image.new("RGB", (2 * cx, nrows))
            left = int(round(cx - (ncols / 2.)))
            black.paste(self.imageRect, (left, 0))
            self.imageRect = black
            
        if 2 * cy > nrows:
            ncols, nrows = self.imageRect.size
            black = Image.new("RGB", (ncols, 2 * cy))
            top = int(round(cy - (nrows / 2.)))
            black.paste(self.imageRect, (0, top))
            self.imageRect = black
        
        self.status += 1.
        
        
def setupRect(T, rows, cols, size):

    region = (0, rows - 1, 0, cols - 1)
    bounds = transBounds(T, region)
    nrows = bounds[1] - bounds[0]
    ncols = bounds[3] - bounds[2]

    #size = max(rows, cols)
    scale = size / float(max(nrows, ncols))

    S = matrix([[scale, 0, 0],
                      [0, scale, 0],
                      [0, 0, 1]], dtype = float)

    T = S*T

    bounds = transBounds(T, region)
    nrows = bounds[1] - bounds[0]
    ncols = bounds[3] - bounds[2]

    invT = T.I

    (yi, xi) = mgrid[0:nrows,0:ncols]
    row1 = matrix(xi.ravel() + bounds[2], dtype = float)
    row2 = matrix(yi.ravel() + bounds[0], dtype = float)
    row3 = matrix(ones((1, ncols*nrows)), dtype = float)
    P = concatenate((row1, row2, row3))
    sxy = trans(invT, P)
    
    return ncols, nrows, sxy[0:2,:]


def trans(T, P):
    (m, n) = P.shape

    transP = T*P
    transP = transP.astype(float)

    transP = transP / transP[m-1,:]
    
    return transP


def transBounds(T, region):
    r = region
    P = matrix([[r[2], r[3], r[3], r[2]],
                [r[0], r[0], r[1], r[1]],
                [1,    1,    1,    1]], dtype = float)

    transP = trans(T, P)
    transP = transP.round()
    transP = transP.astype(int)
    
    return (transP[1,:].min(), transP[1,:].max(), transP[0,:].min(), transP[0,:].max())