#ifndef DEF_SIFTSEARCH
#define DEF_SIFTSEARCH

#include <CImg.h>
#include <sift.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include "../tools/mathTools.h"
#include "../tools/regminmax.h"

using namespace std;
using namespace cimg_library;

typedef vector< pair<VL::Sift::Keypoint,VL::float_t> > Keypoints;

struct tempKeypoint 
{
	tempKeypoint(float x_,float y_,float sig_,float th_):x(x_),y(y_),sig(sig_),th(th_){}
	float x,y,sig,th;
};

typedef std::vector<tempKeypoint> keyVector;
bool cmpKeypoints(Keypoints::value_type const&a, Keypoints::value_type const&b);
void insertDescriptor(cimg_library::CImgList<float> &outList, VL::float_t x,VL::float_t y,VL::float_t sig,VL::float_t ang, VL::float_t const * descr_pt);
void imageToBuffer(cimg_library::CImg<float> &input, VL::PgmBuffer *buffer);
void siftPointsSearch(CImg<float> &input, CImgList<float> &outList);
void siftPairsSearch(CImgList<float> &siftPoints0, CImgList<float> &siftPoints1, CImgList<float> &outList, float maxSimilarity, int *nOper = NULL, int *status = NULL);

#endif
