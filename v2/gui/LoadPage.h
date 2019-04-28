#ifndef DEF_LOADPAGE
#define DEF_LOADPAGE

#include <QtGui>
#include "../gui/InfoPage.h"
#include "../core/Stereo.h"
#include "../tools/regminmax.h"

class LoadPage : public QWizardPage
{
	Q_OBJECT

public:
	LoadPage();
	~LoadPage();
	bool validatePage();

private slots:
	void loadImages();

private:
	int m_nFiles;
	QLabel *m_loadLabel;
	QPushButton *m_browseButton;
	QGridLayout *m_layout;
	QList<QLabel*> m_thumbsLabels;
	QString m_openFilesPath;
};


#endif

