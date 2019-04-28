#ifndef DEF_MATHTOOLS
#define DEF_MATHTOOLS

#include <CImg.h>
//#include <jama_qr.h>
#include "../tools/tools.h"
#include "../tools/regminmax.h"

using namespace cimg_library;

float crossCorrel(CImg<float> &vect0, CImg<float> &vect1);
CImg<float> essentialMat(CImg<float> &K0, CImg<float> &K1, CImgList<float> &pairsList);
CImgList<float> alternateCameraMat(CImg<float> &E, CImg<float> &K0, CImg<float> &K1, CImgList<float> &testPts, int width0, int height0, int width1, int height1);
void triangulate(CImg<float> &dispMap, CImg<float> &output, CImg<float> &Kl, CImg<float> &Kr, CImg<float> &t, float f);
CImg<float> triangulate(CImg<float> &pR, CImg<float> &pL, CImg<float> &Kl, CImg<float> &Kr, float f, float b);
CImgList<float> rectifMats(CImg<float> &R1, CImg<float> &t1, CImg<float> &K0, CImg<float> &K1, int width0, int height0, int width1, int height1);
CImg<int> transRegion(CImg<float> &T, int x0, int y0, int x1, int y1);
void transImage(CImg<float> &T, CImg<float> *input, CImg<float> *output, CImg<int> &region, int *nOper = NULL, int *status = NULL);
CImg<float> normalize(CImg<float> &P);
CImg<float> eightPoints(CImg<float> &P0, CImg<float> &P1);
CImg<int> mutualRegion(int width0, int height0, int width1, int height1, CImg<float> &T0, CImg<float> &T1);
CImg<float> crossProdMat(CImg<float> v);
CImg<float> crossProdVect(CImg<float> M);
CImg<float> newCoords(CImg<float> R, CImg<float> t);
CImg<float> fundamentalMat(CImgList<float> &pairsList);
CImg<float> reverseMat(CImg<float> M);
bool isOutOfFrame(CImgList<float> &C, CImg<float> &P);
CImgList<float> transFilledRegion(int x0, int y0, int x1, int y1, CImg<float> &T, int Ox, int Oy);
CImg<float> toHomog(CImg<float> &M);
CImg<float> toNotHomog(CImg<float> &M);
CImg<float> interpLinear2d(CImg<float> knownPoints);



#endif
