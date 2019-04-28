#include "ViewPair.h"


ViewPair::ViewPair(View *view0, View *view1)
{
	m_view << view0;
	m_view << view1;
	m_isRectified = false;
	m_rectView << new View;
	m_rectView << new View;
}

View *ViewPair::getView(int nView)
{
	return m_view[nView];
}

View *ViewPair::getRectView(int nView)
{
	if (m_isRectified)
	{
		return m_view[nView];
	}
	else
	{
		return m_rectView[nView];
	}
}

CImg<float> &ViewPair::getDispMap()
{
	return m_dispMap;
}

void ViewPair::setRectified()
{
	m_isRectified = true;
}

void ViewPair::computeE(CImgList<float> &pairs)
{
	m_E = essentialMat(m_view[0]->getK(), m_view[1]->getK(), pairs);
}

void ViewPair::computeR_t(CImgList<float> &testPts)
{
	CImgList<float> matrices = alternateCameraMat(m_E, m_view[0]->getK(), m_view[1]->getK(), testPts, m_view[0]->dimx(), m_view[0]->dimy(), m_view[1]->dimx(), m_view[1]->dimy());
	m_R = matrices[0];
	m_t = matrices[1];
}

void ViewPair::computeT0_T1()
{
	if (!m_isRectified)
	{
		CImgList<float> matrices = rectifMats(m_R, m_t, m_view[0]->getK(), m_view[1]->getK(), m_view[0]->dimx(), m_view[0]->dimy(), m_view[1]->dimx(), m_view[1]->dimy());
		m_T << matrices[0];
		m_T << matrices[1];
		float f = (m_view[0]->getFocal() + m_view[1]->getFocal()) / 2;
		m_rectView[0]->setK(matrices[2]);
		m_rectView[0]->setFocal(f);
		m_rectView[1]->setK(matrices[3]);
		m_rectView[1]->setFocal(f);
		m_rectR = matrices[4];
		m_rect_t = matrices[5];
	}
	else
	{
		m_rect_t = m_t;
	}

}

void ViewPair::setRectRegion()
{
	if (!m_isRectified)
	{
		m_rectRegion = mutualRegion(m_view[0]->dimx(), m_view[0]->dimy(), m_view[1]->dimx(), m_view[1]->dimy(), m_T[0], m_T[1]);
		m_rectFilledRegion << transFilledRegion(0, 0, m_view[0]->dimx() - 1, m_view[0]->dimy() - 1, m_T[0], m_rectRegion(0),  m_rectRegion(1));
		m_rectFilledRegion << transFilledRegion(0, 0, m_view[1]->dimx() - 1, m_view[1]->dimy() - 1, m_T[1], m_rectRegion(0),  m_rectRegion(1));
	}
	else
	{
		CImg<float> A(1, 3, 1, 1, "0, 0, 1", 0);
		CImg<float> B = A;
		B(0, 0) = m_view[0]->dimx() - 1;
		CImg<float> C = A;
		C(0, 0) = m_view[0]->dimx() - 1;
		C(0, 1) = m_view[0]->dimy() - 1;
		CImg<float> D = A;
		D(0, 1) = m_view[0]->dimy() - 1;
		CImgList<float> region0;
		region0 << A << B << C << D;
		m_rectFilledRegion << region0;

		B = A;
		B(0, 0) = m_view[1]->dimx() - 1;
		C = A;
		C(0, 0) = m_view[1]->dimx() - 1;
		C(0, 1) = m_view[1]->dimy() - 1;
		D = A;
		D(0, 1) = m_view[1]->dimy() - 1;
		CImgList<float> region1;
		region1 << A << B << C << D;
		m_rectFilledRegion << region1;
	}

}

CImgList<float> &ViewPair::getRectFilledRegion(int nView)
{
	return m_rectFilledRegion[nView];
}

void ViewPair::rectify(int nView, int *nOper, int *status)
{
	m_view[nView]->transform(m_T[nView], m_rectView[nView], m_rectRegion, nOper, status);
}

void ViewPair::setDispMap(CImg<float> &dispMap, bool initialOrder)
{
	m_dispMap = dispMap;
	m_initialOrder = initialOrder;
}

void ViewPair::compute3dCoords()
{
	int idLeftView = 1 - (int) (m_initialOrder);
	int idRightView = (int) (m_initialOrder);
	//CImg<float> Kl = getRectView(idLeftView)->getK();
	//CImg<float> Kr = getRectView(idRightView)->getK();
	//Kr(2, 0) -=181;
	//Kr(2, 1) -=113;
	triangulate(m_dispMap, m_3dCoordsMap, getRectView(idLeftView)->getK(), getRectView(idRightView)->getK(), m_rect_t, getRectView(idLeftView)->getFocal());
	//triangulate(m_dispMap, m_3dCoordsMap, Kl, Kr, m_rect_t);

}

CImg<float> &ViewPair::get3dCoordsMap()
{
	return m_3dCoordsMap;
}

bool ViewPair::isRectified()
{
	return m_isRectified;
}

ViewPair::~ViewPair()
{
	delete m_rectView[0];
	delete m_rectView[1];
}