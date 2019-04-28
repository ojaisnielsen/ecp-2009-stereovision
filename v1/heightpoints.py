#==============================================================================
# Author : Olivier Teboul
# Date : 11/08
# contact : olivier.teboul@ecp.fr
#
# Content : Implement the eight points algorithm that solves the equation:
#                       x'Ex = 0
# It first normalized the couples, then solves the linear equation by finding
# the smallest eigen vector or the correlation matrix, and finally use a SVD to
# ensure that the given matrix satisfies the requested eigen constraints
#
# Example of code : see __main__ script
#==============================================================================

import numpy
from numpy import matrix,zeros,diag
from numpy.linalg import svd

class HeightPoints:
    """
    Compute the 3x3 matrix E, that satisfies the equation x2Ex1 = 0
    """

    def __init__(self,x1,x2):
        """
        x1 is a list of normalized 3D points that corresponds to points in the 1st image
        x2 is a list of normalized 3D points that corresponds to points in the 2nd image
        """

        #pass the data to the class : make them matrices 
        self.set_points(x1,x2)

        #normalize the data
        self.T1 = self.normalization(self.x1)
        self.T2 = self.normalization(self.x2)

        #solve the linear system
        E0 = self.solve()
        
        #denormalize
        E1 = (self.T2.transpose())**(-1)*E0*self.T1**(-1)

        #find the best matrix that satisfies the internal constraints
        u,s,v = svd(E1)
        self.E = u* diag([1,1,0])*v 

    def set_points(self,x1,x2):
        """ From the sets of x and y, create two sets of homogeneous points in a numpy matrix"""
        l1 = []
        l2 = []
        
        for elem in x1:
            a = [u for u in list(elem)]
            l1.append(a)
        self.x1 = numpy.matrix(l1,dtype=float)

        for elem in x2:
            a = [u for u in list(elem)]
            l2.append(a)
        self.x2 = numpy.matrix(l2,dtype=float)

        self.nb_points = min(len(self.x1),len(self.x2))
        self.lines = self.x2[0].size
        self.columns = self.x1[0].size


    def solve(self):
        #Build the matrix from the sets of points
        self.build_matrix_from_points()

        #compute the eigen vector of MtM associated to the min eigen value
        M = self.A.transpose() * self.A
        val,vec = numpy.linalg.eig(M)
        index = val.argmin()
        p = vec[:,index]
        P = p.reshape(self.lines,self.columns)

        return P
        

    def build_matrix_from_points(self):
        """Build the matrix of the system from the sets of points"""
        self.A = matrix(zeros((self.nb_points,9)))
        
        for j in range(self.nb_points):
            x1 = self.x1[j]
            x2 = self.x2[j]
            self.A[j,:] = (x2.transpose()*x1).reshape((1,9)) 
       

    def normalization(self,points):
        """
        modify points so that the point cloud is normalized
        points - numpy.array of size nx3
        return the numpy.matrix of the transformation applied
        """

        #compute the centroid
        c = self.centroid(points)
        n = c.size
        
        #compute the average distance to the centroid
        d = self.average_distance(points,c)
        s = numpy.sqrt(2)/d

        #compute the transformation matrix
        Trslt = numpy.matrix(numpy.eye(n))
        for i in range(n-1):
            Trslt[i,n-1] = -c[0,i]

        Scale = s*numpy.matrix(numpy.eye(n))
        Scale[-1,-1] = 1.0
        
        T = Scale*Trslt

        #apply the matrix on the points
        self.apply_transformation(T,points)
            
        #return the corresponding transformation matrix
        return T

    def centroid(self,points):
        """ compute the centroid (homogeneous coordinates) of a points cloud"""
        centroid = [points[:,i].mean() for i in range(points[0,:].size)]
        c = numpy.matrix(centroid)
        return c


    def average_distance(self,points,center):
        """ compute the average distance to a center in a points cloud"""
        d = 0
        for x in points:
            a = x-center
            d += numpy.linalg.norm(a)
        d/= float(self.nb_points)
        return d.item()
        

    def apply_transformation(self,T,points):
        """ Apply the transformation T to the points cloud. It modify the points cloud"""
        for i in range(len(points)):
            x = points[i,:].transpose()
            y = T*x
            points[i] = y.transpose()

if __name__ == '__main__':
    
    p1 = [(322,2258,1),(530,803,1),(1215,2331,1),(1290,373,1),
          (1626,2326,1),(1578,376,1),(2354,788,1),(2526,2240,1)]

    p2 = [(831,1867,1),(725,788,1),(1358,2379,1),(1277,929,1),
            (1612,2404,1),(1556,931,1),(2480,801,1),(2400,2056,1)]
    
    a = HeightPoints(p1,p2)

    print a.E
