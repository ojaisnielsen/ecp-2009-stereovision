#ifndef DEF_MATCHTHREAD
#define DEF_MATCHTHREAD

#include "../core/Stereo.h"
#include "../tools/MrfMatching.h"
#include "../tools/regminmax.h"


class MatchThread : public QThread
{
public:
	MatchThread(MrfMatching *mrfMatching, int nPart);
	void run();
	static int getStatus();

private:
	MrfMatching *m_mrfMatching;
	int m_nPart;
	static int m_status;
	static int m_nOper;
};

#endif