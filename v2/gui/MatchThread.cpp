#include "MatchThread.h"

int MatchThread::m_status = 0;
int MatchThread::m_nOper = 0;

MatchThread::MatchThread(MrfMatching *mrfMatching, int nPart)
{
	m_mrfMatching = mrfMatching;
	m_nPart = nPart;
}

void MatchThread::run()
{
	m_mrfMatching->startPart(m_nPart, &m_nOper, &m_status);
}

int MatchThread::getStatus()
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
