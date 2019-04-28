#ifndef DEF_PAIRSPAGE
#define DEF_PAIRSPAGE

#include <QtGui>
#include <dos.h>
#include "../core/Stereo.h"
#include "../gui/SiftSearchThread.h"
#include "../gui/Point.h"
#include "../tools/CsvReader.h"
#include "../tools/CsvWriter.h"
#include "../tools/regminmax.h"

class ClickablePixmap;


class PairsPage : public QWizardPage
{
	Q_OBJECT

public:
	PairsPage(View *view0, View *view1);
	~PairsPage();
	void addPairs(CImgList<float> &pairs);
	void addPoint(int nView, float x, float y);
	void showPoint(int nView, float x, float y);

private slots:
	void siftSearch();
	void loadCsv();
	void saveCsv();

protected:
	View *m_view0;
	View *m_view1;
	QFrame *m_topFrame;
	QGraphicsScene *m_leftScene;
	QGraphicsScene *m_rightScene;
	ClickablePixmap *m_leftPixmap;
	ClickablePixmap *m_rightPixmap;
	QGraphicsView *m_leftDisplay;
	QGraphicsView *m_rightDisplay;
	QGridLayout *m_topFrameLayout;
	QTabWidget *m_tabs;
	QFrame *m_siftTab;
	QLabel *m_maxSimilarityLabel;
	QDoubleSpinBox *m_maxSimilarityBox;
	QPushButton *m_siftSearchButton;
	QGridLayout *m_siftTabLayout;
	QFrame *m_pointsTab;
	QTableWidget *m_pointsTable;
	QPushButton *m_loadCsvButton;
	QPushButton *m_saveCsvButton;
	QGridLayout *m_pointsTabLayout;
	QGridLayout *m_layout;
	QProgressDialog *m_siftProgressBar;
	QString m_openFilePath;
	QList<Point*> m_leftPoints;
	QList<Point*> m_rightPoints;
	QGraphicsItem *m_tempCircle;
	bool m_incompletePair;
	CImg<float> m_tempPoint;
	int m_tempPointView;
	QLabel *m_leftCoords;
	QLabel *m_rightCoords;
};


#endif