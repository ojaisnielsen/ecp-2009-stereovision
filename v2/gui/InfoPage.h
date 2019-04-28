#ifndef DEF_INFOPAGE
#define DEF_INFOPAGE

#include <QtGui>
#include "../gui/RectifyPage.h"
#include "../core/Stereo.h"
#include "../tools/PhotoInfo.h"
#include "../tools/regminmax.h"

class InfoPage : public QWizardPage
{
	Q_OBJECT

public:
	InfoPage(int nView);
	~InfoPage();
	bool validatePage();

private slots:
	void followUrl(QString url);

private:
	int m_nView;
	View *m_currentView;
	QLabel *m_thumbLabel;
	QLabel *m_focalLabel;
	QLabel *m_ccdWidthLabel;
	QDoubleSpinBox *m_focalBox;
	QDoubleSpinBox *m_ccdWidthBox;
	QGridLayout *m_layout;
	QLabel *m_searchLinkLabel;
};


#endif
