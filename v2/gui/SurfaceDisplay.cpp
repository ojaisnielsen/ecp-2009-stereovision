#include "SurfaceDisplay.h"


#include <CImg.h>

using namespace cimg_library;

SurfaceDisplay::SurfaceDisplay(CImg<float> &coordsMap) : SurfacePlot()
{
	int W = coordsMap.dimx();
	int H = coordsMap.dimy();
	for (int y = 0; y < H; y++)
	{
		for (int x = 0; x < W; x++)
		{
			Triple p(coordsMap(x, y, 0), coordsMap(x, y, 1), coordsMap(x, y, 2));
			m_vertices.push_back(p);

		}
	}

	for (int x = 0; x < W - 1; x++)
	{
		for (int y = 0; y < H - 1; y++)
		{
			Cell triangle0;
			triangle0.push_back(x + y * W);
			triangle0.push_back(x + (y + 1) * W);
			triangle0.push_back((x + 1) + (y + 1) * W);

			Cell triangle1;
			triangle1.push_back(x + y * W);
			triangle1.push_back((x + 1) + y * W);
			triangle1.push_back((x + 1) + (y + 1) * W);

			m_polygons.push_back(triangle0);
			m_polygons.push_back(triangle1);

		}
	}

	loadFromData(m_vertices, m_polygons);
    setCoordinateStyle(NOCOORD);
    updateData();
    updateGL();
}