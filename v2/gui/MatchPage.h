#ifndef DEF_MATCHPAGE
#define DEF_MATCHPAGE

#include <QtGui>
#include "../core/Stereo.h"
#include "../tools/MrfMatching.h"
#include "../gui/MatchThread.h"
#include "../gui/PairsPage.h"
#include "../gui/TriangulatePage.h"
#include "../tools/regminmax.h"

class MatchPage : public PairsPage
{
public:
	MatchPage(int nPair);
	~MatchPage();
	bool validatePage();

private:
	int m_nPair;
	QLabel *m_radiusLabel;
	QSpinBox *m_radiusBox;
	QLabel *m_maxDispLabel;
	QSpinBox *m_maxDispBox;
	QLabel *m_nThreadsLabel;
	QSpinBox *m_nThreadsBox;
	QLabel *m_stepLabel;
	QSpinBox *m_stepBox;
	QProgressDialog *m_matchProgressBar;
};

#endif
