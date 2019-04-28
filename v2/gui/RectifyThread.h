#ifndef DEF_RECTIFYTHREAD
#define DEF_RECTIFYTHREAD

#include "../core/Stereo.h"
#include "../tools/regminmax.h"


class RectifyThread : public QThread
{
public:
	RectifyThread(int nPair, int idThread);
	void run();
	static int getStatus();

private:
	static int m_status;
	static int m_nOper;
	int m_nPair;
	int m_idThread;
};

#endif
