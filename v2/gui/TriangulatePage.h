#ifndef DEF_TRIANGULATEPAGE
#define DEF_TRIANGULATEPAGE

#include <QWizard>

class QGridLayout;
class QLabel;
class QPushButton;
class QString;

class TriangulatePage : public QWizardPage
{
	Q_OBJECT

public:
	TriangulatePage(int nPair);
	~TriangulatePage();

private slots:
	void saveModel();

private:
	int m_nPair;
	QLabel *m_disparityLabel;
	QPushButton *m_displayButton;
	QPushButton *m_saveButton;
	QString m_fileName;
	QGridLayout *m_layout;
};

#endif