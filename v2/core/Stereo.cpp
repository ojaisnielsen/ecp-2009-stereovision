#include "Stereo.h"

Stereo::Stereo()
{
}

void Stereo::loadFiles(QStringList fileNames)
{
	int n = fileNames.size();

	for (int i = 0; i < n; i++)
	{
		m_views << new View(fileNames[i]);
	}

	for (int i = 0; i < n - 1; i++)
	{
		m_pairs << new ViewPair(getView(i), getView(i + 1));
	}
}

View *Stereo::getView(int nView)
{
	return m_views[nView];
}

ViewPair *Stereo::getPair(int nPair)
{
	return m_pairs[nPair];
}

int Stereo::getNViews()
{
	return m_views.size();
}

int Stereo::getNPairs()
{
	return m_pairs.size();
}

CImgList<float> &Stereo::getTempPointPairs()
{
	return m_tempPointPairs;
}

void Stereo::addTempPointPairs(CImgList<float> &pairs)
{
	m_tempPointPairs << pairs;
}

void Stereo::dropTempPointPairs()
{
	m_tempPointPairs.remove(0, m_tempPointPairs.size - 1);
}

void Stereo::setMatrices(int nPair)
{
	m_pairs[nPair]->computeE(m_tempPointPairs);
	m_pairs[nPair]->computeR_t(m_tempPointPairs);
	m_pairs[nPair]->computeT0_T1();
	m_pairs[nPair]->setRectRegion();
}


Stereo::~Stereo()
{
	for (int i = 0; i < m_views.size(); i++)
	{
		delete m_views[i];
	}
	for (int i = 0; i < m_pairs.size(); i++)
	{
		delete m_pairs[i];
	}
}

