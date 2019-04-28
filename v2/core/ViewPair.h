#ifndef DEF_VIEWPAIR
#define DEF_VIEWPAIR


#include <CImg.h>
#include "../core/View.h"
#include "../tools/mathTools.h"
#include "../tools/regminmax.h"

using namespace cimg_library;
using namespace std;

class ViewPair
{
public:
	ViewPair(View *view0, View *view1);
	~ViewPair();
	View *getView(int nView);
	View *getRectView(int nView);
	CImg<float> &getDispMap();
	void setRectified();
	void computeE(CImgList<float> &pairs);
	void computeR_t(CImgList<float> &testPts);
	void computeT0_T1();
	void rectify(int nView, int *nOper = NULL, int *status = NULL);
	void setRectRegion();
	void setDispMap(CImg<float> &dispMap, bool initialOrder);
	void compute3dCoords();
	CImg<float> &get3dCoordsMap();
	bool isRectified();
	CImgList<float> &getRectFilledRegion(int nView);

private:
	bool m_isRectified;
	QList<View*> m_view;
	QList<View*> m_rectView;
	CImg<float> m_t;
	CImg<float> m_R;
	CImg<float> m_rect_t;
	CImg<float> m_rectR;
	CImg<float> m_E;
	CImgList<float> m_T;
	CImg<int> m_rectRegion;
	QList<CImgList<float>> m_rectFilledRegion;
	CImg<float> m_dispMap;
	CImg<float> m_3dCoordsMap;
	bool m_initialOrder;
};

#endif