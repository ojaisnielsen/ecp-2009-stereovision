from PIL import Image
from numpy import array, matrix, zeros, round, concatenate, mgrid
from numpy.linalg import eig
from visual import *

def drawSurface(points):
    
    N = len(points)
    
    X = array(map(lambda (x, y, z): x, points), dtype = float)
    Y = array(map(lambda (x, y, z): y, points), dtype = float)
    Z = array(map(lambda (x, y, z): z, points), dtype = float)
    
    X = X - X.min()
    Y = Y - Y.min()
    Z = Z - Z.min()
    
    M = max([X.max(), Y.max(), Z.max()])
    
    X = 10 * X / M
    Y = 10 * Y / M
    Z = 10 * Z / M
    
    points = zip(X.tolist(), Y.tolist(), Z.tolist())
    surface = convex(pos = points)
    #dots = map(lambda X: sphere(pos = X, radius = 0.1), points)
    
    #i, j = mgrid[0:N,0:N]
    #couples = zip(list(i.flat), list(j.flat))
    #edges = map(lambda (x, y): not x == y and curve(pos = [points[x], points[y]]), couples)


def reconst3D(p1, p2, P1, P2, K1 = None, K2 = None):
    
    if K1 == None or K2 == None:
        K1, _, _ = geom.projDecomp(P1)
        K2, _, _ = geom.projDecomp(P2)
    
    x1, y1 = p1
    x2, y2 = p2
    w1 = 2 * K1[0,2]
    h1 = 2 * K1[1,2]
    w2 = 2 * K2[0,2]
    h2 = 2 * K2[1,2]
    
    p1 = matrix([x1, y1, 1], dtype = float).T
    p2 = matrix([x2, y2, 1], dtype = float).T
    
    H1 = matrix([[2./w1, 0, -1],
                [0, 2./h1, -1],
                [0, 0, 1]], dtype = float)
    
    H2 = matrix([[2./w2, 0, -1],
                [0, 2./h2, -1],
                [0, 0, 1]], dtype = float)
    
    P1 = H1*P1
    P2 = H2*P2
    
    p1 = H1*p1
    p2 = H2*p2
    
    x1, y1, z1 = list(p1.flat)
    x2, y2, z2 = list(p2.flat)
    
    M1 = matrix([[0, -z1, y1],
                 [z1, 0, -x1],
                 [-y1, x1, 0]], dtype = float)
    M2 = matrix([[0, -z2, y2],
                 [z2, 0, -x2],
                 [-y2, x2, 0]], dtype = float)
    
    A = concatenate((M1*P1, M2*P2))
    val, vec = eig(A.T*A)
    iX = val.argmin()

    X = matrix(vec[:, iX], dtype = float)
    
    s1 = P1[2,:]*X
    s2 = P2[2,:]*X

    
    if s1[0,0] < 0 or s2[0,0] <0:
        X = -X
    
    X = X / X[3,0]
    return (X[0, 0], X[1, 0], X[2, 0])
    

def reconst3dPoints(l1, l2, P1, P2, K1 = None, K2 = None):
    X = map(lambda (x, y): reconst3D(x, y, P1, P2, K1, K2), zip(l1, l2))
    return X
    
def rectDepthMap(l1, l2):#, K):
        #f = K[0,0]
        w = 2 * K[0,2]
        h = 2 * K[1,2]
        depthMap = Image.new("L", (w, h))
        pixels = depthMap.load()
        l1x = array(map(lambda (x, y): x, l1), dtype = float)
        l2x = array(map(lambda (x, y): x, l2), dtype = float)
        depth = 1. / (l1x - l2x) #1 / (l1x - l2x)
        depth = 255 * depth / depth.max()
        for i in range(0, len(l1)):
            pixels[l1[i]] = depth[i]
        depthMap.show()

if __name__ == "__main__":
    l = [(1,0,0), (0,1,0), (0,0,1), (1, 1, 1)]
    drawSurface(l)
    

