#include "RectifyThread.h"

int RectifyThread::m_status = 0;
int RectifyThread::m_nOper = 0;

RectifyThread::RectifyThread(int nPair, int idThread)
{
	m_nPair = nPair;
	m_idThread = idThread;
}

void RectifyThread::run()
{
	Stereo::GetInstance()->getPair(m_nPair)->rectify(m_idThread, &m_nOper, &m_status);
}


int RectifyThread::getStatus()
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