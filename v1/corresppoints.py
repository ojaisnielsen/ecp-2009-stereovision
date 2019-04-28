from PIL import Image, ImageFilter, ImageChops, ImageOps
from sift import *
from numpy import argmax, argsort, array, matrix, zeros, unravel_index, dtype, nonzero, ones, histogram
from numpy.linalg import norm
from opencv import *
from scipy.interpolate import interp1d


class CorrespPoints:
        def __init__(self, image0, image1, radius, minCorrel, minSharpness):
                self.radius = radius
                self.minCorrel = minCorrel
                self.minSharpness = minSharpness
                self.status = 0.
                self.nbOp = 1.
                self.image0 = image0
                self.image1 = image1
                self.w, self.h = self.image0.size
                nBands = len(self.image0.getbands())
                tuple = dtype(", ".join(["f"] * nBands))
                self.pixels = (array(self.image0.getdata(), dtype = tuple).reshape((self.h, self.w)), \
                               array(self.image1.getdata(), dtype = tuple).reshape((self.h, self.w)))
                self.disparities = -1 * ones((self.h, self.w), dtype = float)
                self.lastIndex = 0
                        
                
        def FindPairs(self, maxHorShift, step):
                
                searRad = int(maxHorShift*self.w)
                self.nbOp= ((self.w - 2*self.radius)*(self.h - 2*self.radius)) / step
                
                for y0 in range (self.radius, self.h - self.radius, step):
                        for x0 in range (self.radius, self.w - self.radius, step):
                                
                                self.status += 1
                                
                                x1Min = max(x0 - searRad, self.radius)
                                x1Max = min(x0 + searRad, self.w - self.radius - 1)
                                neighbours0 = tupleArrayToList(neighbours(y0, x0, self.pixels[0], self.radius))
                                pointsCorrel = []
                                for x1 in range (x1Min, x1Max + 1):
                                        neighbours1 = tupleArrayToList(neighbours(y0, x1, self.pixels[1], self.radius))
                                        pointsCorrel.append(crossCorrel(neighbours0, neighbours1))
                                
                                indices = argsort(pointsCorrel)
                                argMax = indices[-1]
                                argSecondMax = indices[-2]
                                maxCorrel = pointsCorrel[argMax]
                                secondMaxCorrel = pointsCorrel[argSecondMax]
                                sharpness = (1 - (secondMaxCorrel / maxCorrel))
                                if maxCorrel > self.minCorrel and sharpness > self.minSharpness:
                                        self.disparities[y0, x0] = abs(argMax + x1Min - x0)
                                        
                knownIndices = nonzero(self.disparities[y,:] >= 0)[0]
                if len(knownIndices) > 1:
                        firstInd = knownIndices.min()
                        lastInd = knownIndices.max()
                        interpolation = interp1d(knownIndices, self.disparities[y,knownIndices], bounds_error = False, fill_value = 0, kind = "linear")
                        self.disparities[y,:] = interpolation(range(0, self.w))
                                        
                                        
                                        
                                        
        def FastFindPairs(self,  edgeThreshold):

                edges0, _, _ = canny(self.image0, edgeThreshold).split()
                edges1, _, _ = canny(self.image1, edgeThreshold).split()
                
                edges0 = array(edges0.getdata(), dtype = float).reshape((self.h, self.w))
                edges1 = array(edges1.getdata(), dtype = float).reshape((self.h, self.w))
                y0s, x0s = nonzero(edges0 > 0)
                self.nbOp = self.h
                hist, _ = histogram(y0s, bins = self.h, range = (0, self.h))
                
                k = 0
                for y in range(0, self.h):
                        nEdges = hist[y]
                        for iX0 in range(0, nEdges):
                                x0 = x0s[k+iX0]
                                x1s = nonzero((edges1[y,:]>0))[0]
                                if x0 in range(self.radius, self.w - self.radius) and y in range(self.radius, self.h - self.radius):
                                        neighbours0 = neighbours(y, x0, self.pixels[0], self.radius)
                                        pointsCorrel = []
                                        for x1 in x1s:
                                                if x1 in range(self.radius, self.w - self.radius):
                                                        pointsCorrel.append(crossCorrel(tupleArrayToList(neighbours0), \
                                                                                        tupleArrayToList(neighbours(y, x1, self.pixels[1], self.radius))))
                                        indices = argsort(pointsCorrel)
                                        if len(indices) > 0:
                                                argMax = indices[-1]
                                                maxCorrel = pointsCorrel[argMax]
                                                
                                        if len(indices) <= 1:
                                                secondMaxCorrel = 0
                                        else:
                                                argSecondMax = indices[-2]
                                                secondMaxCorrel = pointsCorrel[argSecondMax]
                                        if not pointsCorrel == [] and maxCorrel >= self.minCorrel and (1 - (secondMaxCorrel / maxCorrel)) > self.minSharpness:
                                                x1 = x1s[argMax]
                                                edges1[y,x1] = 0
                                                self.disparities[y,x0] = abs(x1 - x0)
                        self.status += 1
                        k += nEdges
                        knownIndices = nonzero(self.disparities[y,:] >= 0)[0]
                        if len(knownIndices) > 1:
                                firstInd = knownIndices.min()
                                lastInd = knownIndices.max()
                                interpolation = interp1d(knownIndices, self.disparities[y,knownIndices], bounds_error = False, fill_value = 0, kind = "linear")
                                self.disparities[y,:] = interpolation(range(0, self.w))
                                
                                        
                                        
                                        
        def PercentStat(self):
                return self.status / self.nbOp
        
class pySiftPairs:
        def __init__(self, image0, image1, minCorrel, minSharpness):
                self.image0 = image0
                self.image1 = image1
                self.minCorrel = minCorrel
                self.minSharpness = minSharpness
                self.pairs = []
                self.lastIndex = 0
                self.status = 0.
                self.nbOp = 1.
                
        def LastFoundPairs(self):
                if self.lastIndex in range(0, len(self.pairs)):
                        i = self.lastIndex
                        self.lastIndex = len(self.pairs)
                        return self.pairs[i:]
                        
                
        def PercentStat(self):
                return self.status / self.nbOp
        
        def FindPairs(self):
                self.image0 = ImageOps.grayscale(self.image0)
                self.image1 = ImageOps.grayscale(self.image1)
                
                data0 = list(self.image0.getdata())
                data0.insert(0, self.image0.size)
                data1 = list(self.image1.getdata())
                data1.insert(0, self.image1.size)
                
                desc0 = sift(data0, verbose=True)
                desc1 = sift(data1, verbose=True)
                
                m = len(desc0)
                n = len(desc1)
                
                self.nbOp = m * n

                for i in range(0, m):
                        d0 = array(desc0[i])
                        x0 = d0[0]
                        y0 = d0[1]
                        d0 = d0[4:d0.size]
                        descsCorrel = []
                        for j in range(0, len(desc1)):
                                self.status += 1
                                d1 = desc1[j]
                                d1 = d1[4:]
                                descsCorrel.append(crossCorrel(d0, d1))
                        indices = argsort(descsCorrel)
                        argMax = indices[-1]
                        argSecondMax = indices[-2]
                        maxCorrel = descsCorrel[argMax]
                        secondMaxCorrel = descsCorrel[argSecondMax]
                        d1 = desc1[argMax]
                        x1 = d1[0]
                        y1 = d1[1]
                        if maxCorrel > self.minCorrel and (1 - (secondMaxCorrel / maxCorrel)) > self.minSharpness:
                                del desc1[argMax]
                                self.nbOp -= 1 
                                self.pairs.append((x0, y0, x1, y1))

                
def neighbours(i, j, pixels, radius):
        return pixels[i - radius:i + radius + 1,j - radius:j + radius + 1]


def improvePair (image0, image1, p0, p1i, searchRad):
        w0, h0 = image0.size
        w1, h1 = image1.size
        nBands = len(image0.getbands())
        tuple = dtype(", ".join(["f"] * nBands))
        pixels = (array(image0.getdata(), dtype = tuple).reshape((h0, w0)), \
                  array(image1.getdata(), dtype = tuple).reshape((h1, w1)))
        x0, y0 = p0
        x1i, y1i = p1i
        xMin = max(1, x1i - searchRad)
        xMax = min(x1i + searchRad, w1 - 2)
        yMin = max(1, y1i - searchRad)
        yMax = min(y1i + searchRad, h1 - 2)
        pointsCorrel = zeros((xMax - xMin + 1, yMax - yMin + 1), dtype = float)
        neighbours0 = neighbours(y0, x0, pixels[0], 1)
        for x1 in range(xMin, xMax + 1):
                for y1 in range(yMin, yMax + 1):
                        pointsCorrel[x1 - xMin,y1 - yMin] = crossCorrel(tupleArrayToList(neighbours0), tupleArrayToList(neighbours(y1, x1, pixels[1], 1)))
        print pointsCorrel
        pointsCorrel = pointsCorrel.ravel().tolist()

        indices = argsort(pointsCorrel)
        argMax = indices[-1]
        argSecondMax = indices[-2]
        maxCorrel = pointsCorrel[argMax]
        secondMaxCorrel = pointsCorrel[argSecondMax]
        sharpness = (1 - (secondMaxCorrel / maxCorrel))
        if sharpness > 0.01:
                x, y = unravel_index(argMax, (xMax - xMin + 1, yMax - yMin + 1))
                return (x + xMin, y + yMin)

        else:
                return p1i


def maxSharpness(argMax, l):
        if len(l) == 1:
                return 1
        else:
                indices = argsort(l)
                argMax = indices[-1]
                argSecondMax = indices[-2]
                return (1 - (l[argSecondMax] / l[argMax]))

def tupleArrayToList(u):
        return map(list, list(u.flat))

def crossCorrel(u, v):
        u = matrix(u, dtype = float)
        u = u.ravel()
        v = matrix(v, dtype = float)
        v = v.ravel()
        u = u - u.mean()
        v = v - v.mean()
        p = (u*v.T)/(norm(u)*norm(v))
        return p[0,0]
   
   
#def substrAverage(image):
#        averageMat = (1,  1,  1,  1,  1,
#                      1,  1,  1,  1,  1,
#                      1,  1,  1,  1,  1,
#                      1,  1,  1,  1,  1,
#                      1,  1,  1,  1,  1)
#        
#        kernel = ImageFilter.Kernel((5, 5), averageMat, 25)
#        average = image.filter(kernel)
#        image =  ImageChops.difference(image, average)
#        return image

        
def canny(image, threshold):
        cvIm = pil_to_ipl(image)
        col_edge = cvCreateImage (cvSize (cvIm.width, cvIm.height), 8, 1)
        gray = cvCreateImage (cvSize (cvIm.width, cvIm.height), 8, 1)
        edge = cvCreateImage (cvSize (cvIm.width, cvIm.height), 8, 1)
        output = cvCreateImage (cvSize (cvIm.width, cvIm.height), 8, 3)
        cvCvtColor (cvIm, gray, CV_BGR2GRAY)
        cvCanny (gray, edge, threshold, 3 * threshold, 3)
        cvMerge( edge, edge, edge, None, output )
        return ipl_to_pil(output)
        
        
if __name__ == "__main__":
        image0 = Image.open("tsukuba_left.ppm")
        image1 = Image.open("tsukuba_right.ppm")
        w, h = image0.size
        disp = -1 * ones((h, w), dtype = float)
        prog = CorrespPoints(image0, image1, disp, 3, 0.9, 0.1, 0.5, 1, 100)
        prog.FastFindPairs()
        output = Image.new("L", (w, h))
        output.putdata(disp.ravel())
        output.show()

