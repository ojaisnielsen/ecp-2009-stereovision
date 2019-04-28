# coding=UTF-8

from numpy import zeros, matrix, diag, concatenate, cross
from numpy.linalg import norm, eig, svd, qr
from heightpoints import HeightPoints
from reconst3d import reconst3D

class matrices:
    def __init__(self, f1, f2, c1, c2, p1, p2):
        self.K1 = calibMat(f1, c1)
        self.K2 = calibMat(f2, c2)
        self.E = essMat(self.K1, self.K2, p1, p2)
        self.R, self.t = matrCam(self.E, self.K1, self.K2, p1[0], p2[0])
        self.T1, self.T2, self.P1, self.P2, self.Pn1, self.Pn2, self.Kn = rectifMat(self.R, self.t, self.K1, self.K2)

def normalize(x, K):
    x1, x2 = x
    x = K.I*matrix([x1, x2, 1], dtype = float).T 
    return (x[0,0], x[1,0], x[2,0])
    
def calibMat (f, c):
    cx, cy = c
    K = matrix([[f, 0, cx], 
                [0, f, cy],
                [0, 0, 1]], dtype = float)
    return K

def essMat(K1, K2, p1, p2):
    p1n = [normalize(x, K1) for x in p1]
    p2n = [normalize(x, K2) for x in p2]
    return HeightPoints(p1n, p2n).E
    
def essMat2(K1, K2, p1, p2):
    n = min(len(p1), len(p2))
    p1n = [matrix(normalize(p, K1), dtype = float) for p in p1]
    p2n = [matrix(normalize(p, K2), dtype = float) for p in p2]

    M = matrix(zeros((n, 9)), dtype = float)
    for i in range(0, n):
        M[i,:] = (p2n[i].T * p1n[i]).reshape((1, 9))
    
    val, vec = eig(M.T*M)
    iE = val.argmin()
    
    E = vec[:,iE].reshape((3, 3))
    #U, s, Vt = svd(E)
    #E = U * diag([1., 1., 0.]) * Vt

    return E
    
     
def matrCam (E, K1, K2, p1Test, p2Test):
    
    if isinstance (E, tuple):
        U, Vt = E
    else:
        U, _, Vt = svd(E)

    W = matrix([[0, -1, 0],
                [1,  0, 0],
                [0,  0, 1]], dtype = float)
    
    idMat = matrix([[1, 0, 0],
                    [0, 1, 0],
                    [0, 0, 1]], dtype = float)

    R = U*W*Vt
    Rp = U*(W.T)*Vt
    t = U*(matrix([0, 0, 1], dtype = float).T)
    tp = -t
    
    candidates = [(R, t), (R, tp), (Rp, t), (Rp, tp)]
    
    C1 = matrix([0, 0, 0], dtype = float).T
    F1 = matrix([0, 0, K1[0,0]], dtype = float).T
    P1 = K1*concatenate((idMat, matrix([0, 0, 0], dtype = float).T), 1)

    for i in range(0, 4):
        R, t = candidates[i]
        C2 = t
        F2 = R*matrix([0, 0, K2[0,0]], dtype = float).T + t
        P2 = K2*concatenate((R, t), 1)
        xtest, ytest, ztest = reconst3D(p1Test, p2Test, P1, P2, K1, K2)
        Xtest = matrix([xtest, ytest, ztest], dtype = float).T
        if ((F1 - C1).T*(Xtest - C1))[0,0] > 0 and ((F2 - C2).T*(Xtest - C2))[0,0] > 0:
            break
        
    return (R, t)
    
def rectifMat(R2, t2, K1, K2):
    idMat = matrix([[1, 0, 0],
                    [0, 1, 0],
                    [0, 0, 1]], dtype = float)
    
    nulVect = matrix([0, 0, 0], dtype = float).T
    
    R1 = idMat
    
    t1 = nulVect
    
    P1 = K1*concatenate((R1, t1), 1)
    
    P2 = K2*concatenate((R2, t2), 1)
    
    c1 =  - P1[:,0:3].I * P1[:,3]
    c2 =  - P2[:,0:3].I * P2[:,3]
    
    xAxis = c2 - c1
    yAxis = cross(R1[2,:], xAxis.T).T
    zAxis = cross(xAxis.T, yAxis.T).T
    
    
    K = (K1 + K2)/2
    K[0,1] = 0
    
    R = concatenate((xAxis.T / norm(xAxis), \
                     yAxis.T / norm(yAxis), \
                     zAxis.T / norm(zAxis)))
    
    Pn1 = K * concatenate((R, - R*c1), 1)
    Pn2 = K * concatenate((R, - R*c2), 1)
    
    T1 = Pn1[:,0:3] * P1[:,0:3].I
    T2 = Pn2[:,0:3] * P2[:,0:3].I

    return T1, T2, P1, P2, Pn1, Pn2, K

def projDecomp(P):
    Q = p[:,0:3].I
    U, B = qr(Q)
    R = U.I
    t = B * P[:,3]
    K = B.I
    K = K / K[2,2]
    return K, R, t
    