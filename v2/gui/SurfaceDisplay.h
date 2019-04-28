#ifndef DEF_SURFACEDISPLAY
#define DEF_SURFACEDISPLAY

#include <CImg.h>
#include <qwt3d_surfaceplot.h>

using namespace Qwt3D;
using namespace cimg_library;

class SurfaceDisplay : public SurfacePlot
{

public:
	SurfaceDisplay(CImg<float> &coordsMap);

private:
	TripleField m_vertices;
	CellField m_polygons;
};

#endif