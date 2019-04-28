import pylab
import matplotlib.axes3d
import numpy

def drawSurface(points):
    x = []
    y = []
    z = []
    for point in points:
        x.append(point[0])
        y.append(point[1])
        z.append(point[2])
    
        
    n = len(points)
    
    x = numpy.array(x)
    y = numpy.array(y)
    z = numpy.array(z)
    x = x - min(x)
    y = y - min(y)
    z = z - min(z)
    M = float(max(max(x), max(y), max(z)))
    x = numpy.round(99*x/M)
    y = numpy.round(99*y/M)
    z = numpy.array(99*z/M)
    
    X = numpy.outer(numpy.r_[0:100], numpy.ones(100))
    Y = numpy.outer(numpy.ones(100), numpy.r_[0:100])
    Z = numpy.zeros((100, 100))
    
    for i in range(0, n):
            Z[(x[i], y[i])] = z[i]
    
    figure = pylab.figure()
    axes = matplotlib.axes3d.Axes3D(figure)
    axes.plot_wireframe(X,Y,Z)
    axes.set_xlabel("X")
    axes.set_ylabel("Y")
    axes.set_zlabel("Z")
    pylab.show()
    
#points = [(1,2,3),(3,8,9),(5,2,8),(7,9,2)]
#drawSurface(points)