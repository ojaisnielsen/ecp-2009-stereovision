#ifndef DEF_TOOLS
#define DEF_TOOLS

#include <QtGui>
#include <CImg.h>
#include "../tools/tools.h"
#include "../tools/regminmax.h"

using namespace cimg_library;

QPixmap cimgToQpixmap(CImg<float> &inputImage);
void saveMatrix(CImg<float> M, QString fileName = "");
void coordsMapToWaveFront(CImg<float> &coordsMap, QString fileName);
QString uid();

#endif
