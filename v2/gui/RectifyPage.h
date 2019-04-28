#ifndef DEF_RECTIFYPAGE
#define DEF_RECTIFYPAGE

#include <QtGui>
#include <dos.h>
#include "../gui/PairsPage.h"
#include "../gui/MatchPage.h"
#include "../core/Stereo.h"
#include "../gui/RectifyThread.h"
#include "../tools/regminmax.h"


class RectifyPage : public PairsPage
{
	Q_OBJECT

public:
	RectifyPage(int nPair);
	~RectifyPage();
	bool validatePage();

private slots:
	void setRectified();

private:
	int m_nPair;
	bool m_isRectified;
	QPushButton *m_isRectCheckButton;
	QProgressDialog *m_rectProgressBar;
};



#endif
