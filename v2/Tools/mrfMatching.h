#ifndef DEF_MRFMATCHING
#define DEF_MRFMATCHING

#include <CImg.h>

#define _MANY_LABELS_

#include <Fast_PD.h>
#include <LIMITS.H>
#include <FLOAT.H>
#include "../tools/imageTools.h"
#include "../tools/mathTools.h"
#include "../tools/regminmax.h"

class ViewPair;

using namespace cimg_library;

typedef CV_Fast_PD::Real Real;

class MrfMatching
{
public:
	MrfMatching(ViewPair *viewPair, int radius, int maxDisparity, int step, int nParts);
	void includeKnownMatches(CImgList<float> &pairs);
	bool initialOrder();
	static float probaEnergy(float proba);
	void startPart(int nPart = 1, int *nOper = NULL, int *status = NULL);
	CImg<float> disparityMap();

private:
	ViewPair *m_viewPair;
	CImgList<float> m_leftParts;
	CImgList<float> m_rightParts;
	CImgList<float> m_outputParts;
	CImg<int> m_knownPointsMap;
	CImgList<float> m_knownPointsProbas;
	CImg<float> m_errorMap;
	int m_nParts;
	int m_radius;
	int m_nDisparities;
	bool m_initialOrder;
	int m_maxIters;
	int m_step;
	int m_leftId;
	int m_rightId;

};

#endif