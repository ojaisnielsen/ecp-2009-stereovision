#include "tools.h"
#include "../tools/WavefrontWriter.h"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_3.h>

#include <QList>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;

typedef CGAL::Triangulation_3<K>      Triangulation;

typedef Triangulation::Cell_handle    Cell_handle;
typedef Triangulation::Vertex_handle  Vertex_handle;
typedef Triangulation::Locate_type    Locate_type;
typedef Triangulation::Point          Point;
typedef Triangulation::Triangle Triangle;


void coordsMapToWaveFront(CImg<float> &coordsMap, QString fileName)
{
	int W = coordsMap.dimx();
	int H = coordsMap.dimy();

	WavefrontWriter waveFront(fileName);
	//Triangulation T;
	//int nPoint = 1;

	//int size = (int) ceil(coordsMap.max() - coordsMap.min() + 1);
	//int m = (int) (coordsMap.min());
	//CImg<int> addedPoints(size, size, size, 1, 0);
	//QList<QList<int>> addedFaces;

	for (int y = 0; y < H; y++)
	{
		for (int x = 0; x < W; x++)
		{
			//T.insert(Point(coordsMap(x, y, 0, 0), coordsMap(x, y, 0, 1), coordsMap(x, y, 0, 2)));
			waveFront.addVertex(coordsMap(x, y, 0, 0), coordsMap(x, y, 0, 1), coordsMap(x, y, 0, 2));

		}
	}

	//for (Triangulation::Finite_facets_iterator f = T.finite_facets_begin(); f !=  T.finite_facets_end(); ++f)
	//{
	//	Cell_handle cell = f->first; 
	//	int vertexInd = f->second;
	//	
	//	int indices[3];
	//	for (int i = 0; i < 3; i++)
	//	{
	//		//Point p = tr.vertex(i);
	//		Point p = cell->vertex((vertexInd + i + 1) % 4)->point();
	//		int x = (int) p.x();
	//		int y = (int) p.y();
	//		int z = (int) p.z();
	//		if (addedPoints(x - m, y - m, z - m) == 0)
	//		{
	//			waveFront.addVertex(x, y, z);
	//			addedPoints(x - m, y - m, z - m) = nPoint;
	//			indices[i] = nPoint;
	//			//alreadyNeighbours.push_back(QList<int>());
	//			nPoint++;
	//		}
	//		else
	//		{
	//			indices[i] = addedPoints(p.x(), p.y(), p.z());
	//		}
	//	}

		//QList<int> face;
		//face << indices[0] << indices[1] << indices[2];
		//if (!addedFaces.contains(face))
		//{
		//	waveFront.addFace(indices[0], indices[1], indices[2]);
		//	addedFaces.push_back(face);
		//}
	//}
	for (int y = 0; y < H - 1; y++)
	{
		for (int x = 0; x < W - 1; x++)
		{
			waveFront.addFace(1 + x + (y * W), 1 + x + ((y + 1) * W), 1 + (x + 1) + ((y + 1) * W));
			waveFront.addFace(1 + x + (y * W), 1 + (x + 1) + (y * W), 1 + (x + 1) + ((y + 1) * W));
		}
	}
}

QPixmap cimgToQpixmap(CImg<float> &inputImage)
{
	float mn = inputImage.min();
	float mx = inputImage.max(); 
	int w = inputImage.dimx();
	int h = inputImage.dimy();
	int v = inputImage.dimv();

	QImage outputImage(w, h, QImage::Format_RGB32);
	cimg_forXY(inputImage, x, y)
	{
		int r = (int)(255 * (inputImage(x, y, 0, 0) - mn) / (mx - mn));
		int g = r;
		int b = r;
		if (v == 3)
		{
			g = (int)(255 * (inputImage(x, y, 0, 1) - mn) / (mx - mn));
			b = (int)(255 * (inputImage(x, y, 0, 2) - mn) / (mx - mn));
		}
		outputImage.setPixel(x, y, qRgb(r, g, b));
	}
	return QPixmap::fromImage(outputImage);
}


QString uid()
{
	static int id = 0;
	id++;
	QString uid = QString("%1").arg(id);
	return uid;
}

void saveMatrix(CImg<float> M, QString fileName)
{
	FILE *file;
	if (fileName == "")
	{
		file = fopen(qPrintable(uid() + ".txt"), "w+");
	}
	else
	{
		file = fopen(qPrintable(fileName), "w+");
	}
	for (int y = 0; y < M.dimy(); y++)
	{
		for (int x = 0; x < M.dimx(); x++)
		{
			QString coef = (QString("%1").arg(M(x, y)) + " ");
			fputs(qPrintable(coef), file);
		}
		fputs("\n", file);
	}
	fclose(file);
}
