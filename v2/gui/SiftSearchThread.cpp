#include "SiftSearchThread.h"

SiftSearchThread::SiftSearchThread(View *input0, View *input1, float maxSimilarity) : QThread()
{
	m_input0 = input0;
	m_input1 = input1;
	m_maxSimilarity = maxSimilarity;
	m_status = 0;
	m_nOper = 0;
}


void SiftSearchThread::run()
{
	SiftPointsThread pointsThread0(m_input0, &m_siftPoints0);
	SiftPointsThread pointsThread1(m_input1, &m_siftPoints1);
	pointsThread0.start();
	pointsThread1.start();
	while (pointsThread0.isRunning() || pointsThread1.isRunning())
	{
		;
	}
	siftPairsSearch(m_siftPoints0, m_siftPoints1, m_siftPairs, m_maxSimilarity, &m_nOper, &m_status);
}

int SiftSearchThread::getStatus()
{
	if (m_nOper == 0)
	{
		return 0;
	}
	else
	{
		return (int) (1000 * (float)(m_status) / (float)(m_nOper));
	}
}

CImgList<float> &SiftSearchThread::getSiftPairs()
{
	return m_siftPairs;
}

SiftSearchThread::SiftPointsThread::SiftPointsThread(View *input, CImgList<float> *output)
{
	m_input = input;
	m_output = output;
}

void SiftSearchThread::SiftPointsThread::run()
{
	siftPointsSearch(*m_input, *m_output);
}