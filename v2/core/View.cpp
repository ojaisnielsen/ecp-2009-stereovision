#include "View.h"

View::View() : CImg<float>()
{
	m_fileName = "";
}


View::View(QString fileName) : CImg<float>(qPrintable(fileName))
{
	m_fileName = fileName;
	resize(dimx(), dimy(), 1, 3);
	m_K.assign(3, 3);
	m_K.identity_matrix();
	m_K(2, 0) = dimx()/2.;
	m_K(2, 1) = dimy()/2.;
	updatePixmap();
}

QString &View::getFileName()
{
	return m_fileName;
}


void View::updatePixmap()
{
	m_pixmap = cimgToQpixmap(*this);
}

QPixmap &View::getPixmap()
{
	return m_pixmap;
}

void View::setFocal(float focal)
{
	m_f = focal;
}

void View::computeK(float focal, float ccdWidth)
{
	setFocal(focal);
	float pxWidth = ccdWidth / dimx();
	m_K(0, 0) = m_f / pxWidth;
	m_K(1, 1) = m_f / pxWidth;
}

float View::getFocal()
{
	return m_f;
}


CImg<float> &View::getK()
{
	return m_K;
}



void View::setK(CImg<float> &K)
{
	m_K = K;
}


void View::transform(CImg<float> &T, View *output, CImg<int> &n_region, int *nOper, int *status)
{
	unsigned int newWidth = n_region(2) - n_region(0) + 1;
	unsigned int newHeight = n_region(3) - n_region(1) + 1;	
	output->assign(newWidth, newHeight, 1, 3);
	transImage(T, this, output, n_region, nOper, status);
	output->updatePixmap();
}
