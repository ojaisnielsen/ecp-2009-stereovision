#include "PairsPage.h"

#include "ClickablePixmap.h"


PairsPage::PairsPage(View *view0, View *view1) : QWizardPage()
{

	m_view0 = view0;
	m_view1 = view1;
	m_incompletePair = false;

	m_topFrame = new QFrame;

	m_leftPixmap = new ClickablePixmap(m_view0->getPixmap(), this, 0);
	m_leftPixmap->setZValue(0);

	m_rightPixmap = new ClickablePixmap(m_view1->getPixmap(), this, 1);
	m_rightPixmap->setZValue(0);
	
	m_leftScene = new QGraphicsScene;
	m_leftScene->addItem(m_leftPixmap);

	m_rightScene = new QGraphicsScene;
	m_rightScene->addItem(m_rightPixmap);

	m_leftDisplay = new QGraphicsView(m_leftScene);
	m_rightDisplay = new QGraphicsView(m_rightScene);

	m_leftCoords = new QLabel;
	m_rightCoords = new QLabel;

	m_topFrameLayout = new QGridLayout;
	m_topFrameLayout->addWidget(m_leftDisplay, 0, 0);
	m_topFrameLayout->addWidget(m_rightDisplay, 0, 1);
	m_topFrameLayout->addWidget(m_leftCoords, 1, 0);
	m_topFrameLayout->addWidget(m_rightCoords, 1, 1);
	m_topFrame->setLayout(m_topFrameLayout);


	m_tabs = new QTabWidget;

	m_siftTab = new QFrame;

	m_maxSimilarityLabel = new QLabel("Ressemblance maximale entre le correspondant et les autres candidats (critère de Lowe) (%) : ");
	m_maxSimilarityBox = new QDoubleSpinBox;
	m_maxSimilarityBox->setValue(50);

	m_siftSearchButton = new QPushButton("Rechercher");

	m_siftTabLayout = new QGridLayout;
	m_siftTabLayout->addWidget(m_maxSimilarityLabel, 0, 0);
	m_siftTabLayout->addWidget(m_maxSimilarityBox, 0, 1);
	m_siftTabLayout->addWidget(m_siftSearchButton, 1, 1);
	m_siftTab->setLayout(m_siftTabLayout);

	m_pointsTab = new QFrame;

	m_pointsTable = new QTableWidget(0, 2);

	QStringList headers;
	headers << "Coordonnées à gauche" << "Coordonnées à droite";
	m_pointsTable->setHorizontalHeaderLabels(headers);

	m_loadCsvButton = new QPushButton("Charger un CSV...");
	m_saveCsvButton = new QPushButton("Sauvegarder dans un CSV...");

	m_pointsTabLayout = new QGridLayout;
	m_pointsTabLayout->addWidget(m_pointsTable, 0, 0);
	m_pointsTabLayout->addWidget(m_loadCsvButton, 1, 0);
	m_pointsTabLayout->addWidget(m_saveCsvButton, 1, 1);
	m_pointsTab->setLayout(m_pointsTabLayout);

	m_tabs->addTab(m_pointsTab, "Paires de points");
	m_tabs->addTab(m_siftTab, "Rechercher les paires de points SIFT");


	m_layout = new QGridLayout;
	m_layout->addWidget(m_topFrame, 1, 0);
	m_layout->addWidget(m_tabs, 2, 0);
	setLayout(m_layout);


	connect(m_siftSearchButton, SIGNAL(clicked()), this, SLOT(siftSearch()));
	connect(m_loadCsvButton, SIGNAL(clicked()), this, SLOT(loadCsv()));
	connect(m_saveCsvButton, SIGNAL(clicked()), this, SLOT(saveCsv()));
}

PairsPage::~PairsPage()
{
	delete m_topFrame;
	delete m_leftScene;
	delete m_rightScene;
	delete m_topFrameLayout;
	delete m_tabs;
	delete m_siftTab;
	delete m_maxSimilarityLabel;
	delete m_maxSimilarityBox;
	delete m_siftSearchButton;
	delete m_siftTabLayout;
	delete m_pointsTab;
	delete m_pointsTable;
	delete m_loadCsvButton;
	delete m_saveCsvButton;
	delete m_pointsTabLayout;
	delete m_layout;
	delete m_siftProgressBar;
	for (int i = 0; i < m_leftPoints.size(); i++)
	{
		delete m_leftPoints[i];
	}
	for (int i = 0; i < m_rightPoints.size(); i++)
	{
		delete m_rightPoints[i];
	}
	delete m_leftDisplay;
	delete m_rightDisplay;
	delete m_leftPixmap;
	delete m_rightPixmap;
	delete m_leftCoords;
	delete m_rightCoords;
}

void PairsPage::addPoint(int nView, float x, float y)
{
	CImg<float> p(1, 3, 1, 1, 1);
	p(0, 0) = x;
	p(0, 1) = y;
	if (!m_incompletePair)
	{
		if (nView == 0)
		{
			m_tempCircle = m_leftScene->addEllipse(x - 1, y - 1, 3, 3, QPen(QColor("blue")));
		}
		else
		{
			m_tempCircle = m_rightScene->addEllipse(x - 1, y - 1, 3, 3, QPen(QColor("blue")));
		}
		m_tempCircle->setZValue(1);
		m_incompletePair = true;
		m_tempPoint = p;
		m_tempPointView = nView;
	}
	else
	{
		CImgList<float> pair(2);
		pair[m_tempPointView] = m_tempPoint;
		pair[1 - m_tempPointView] = p;
		addPairs(pair);
		m_tempCircle->hide();
		m_incompletePair = false;
	}
}

void PairsPage::showPoint(int nView, float x, float y)
{
	if (nView == 0)
	{
		m_leftCoords->setText(QString("(%1, %2)").arg(x).arg(y));
	}
	else
	{
		m_rightCoords->setText(QString("(%1, %2)").arg(x).arg(y));
	}
}


void PairsPage::siftSearch()
{
	float maxSimilarity = m_maxSimilarityBox->value() / 100;


	m_siftProgressBar = new QProgressDialog("Recherche des paires...", "Annuler", 0, 1000);
	m_siftProgressBar->setWindowModality(Qt::WindowModal);
	m_siftProgressBar->setMinimumDuration(0);
	m_siftProgressBar->setValue(1);

	SiftSearchThread thread(m_view0, m_view1, maxSimilarity);

	thread.start();
	while (thread.isRunning())
	{
		m_siftProgressBar->setValue(thread.getStatus());
		Sleep(100);
	}
	m_siftProgressBar->setValue(1000);

	addPairs(thread.getSiftPairs());
}


void PairsPage::loadCsv()
{
	QString csvFileName = QFileDialog::getOpenFileName(this, "Sélectionner un fichier CSV", m_openFilePath, "Fichier CSV (*.csv)");
	if (csvFileName != "")
	{
		m_openFilePath = csvFileName;
		CsvReader reader(csvFileName);
		int m = reader.dimi();
		CImgList<float> pairs;
		CImg<float> point0(1, 3, 1, 1, 1);
		CImg<float> point1(1, 3, 1, 1, 1);
		for (int i = 0; i < m; i++)
		{
			point0(0, 0) = atof(qPrintable(reader.getCell(i, 0)));
			point0(0, 1) = atof(qPrintable(reader.getCell(i, 1)));
			point1(0, 0) = atof(qPrintable(reader.getCell(i, 2)));
			point1(0, 1) = atof(qPrintable(reader.getCell(i, 3)));

			pairs << point0 << point1;
		}
		addPairs(pairs);
	}
}


void PairsPage::saveCsv()
{
	QString csvFileName = QFileDialog::getSaveFileName(this, "Enregistrer un fichier CSV", m_openFilePath, "Fichier CSV (*.csv)");
	if (csvFileName != "")
	{
		m_openFilePath = csvFileName;
		CsvWriter writer(csvFileName);
		CImgList<float> &pairs = Stereo::GetInstance()->getTempPointPairs();
		float x0, y0, w0, x1, y1, w1;
		for (int i = 0; i < pairs.size; i += 2)
		{
			x0 = pairs[i](0, 0);
			y0 = pairs[i](0, 1);
			w0 = pairs[i](0, 2);
			x1 = pairs[i + 1](0, 0);
			y1 = pairs[i + 1](0, 1);
			w1 = pairs[i + 1](0, 2);
			writer.addCell(x0 / w0);
			writer.addCell(y0 / w0);
			writer.addCell(x1 / w1);
			writer.addCell(y1 / w1);
			writer.newLine();
		}
	}

}

void PairsPage::addPairs(CImgList<float> &pairs)
{
	int oldN = Stereo::GetInstance()->getTempPointPairs().size / 2;
	m_pointsTable->setRowCount(oldN + (pairs.size / 2));

	float x0, y0, w0, x1, y1, w1;
	for (int i = 0; i < pairs.size; i += 2)
	{
		x0 = pairs[i](0, 0);
		y0 = pairs[i](0, 1);
		w0 = pairs[i](0, 2);
		x1 = pairs[i + 1](0, 0);
		y1 = pairs[i + 1](0, 1);
		w1 = pairs[i + 1](0, 2);

		m_leftPoints << new Point(oldN + 1 + (i / 2), x0 / w0,  y0 / w0, m_leftScene);
		m_rightPoints << new Point(oldN + 1 + (i / 2), x1 / w1,  y1 / w1, m_rightScene);
		m_pointsTable->setItem(oldN + i / 2, 0, m_leftPoints[oldN + i / 2]->getCoordTableItem());
		m_pointsTable->setItem(oldN + i / 2, 1, m_rightPoints[oldN + i / 2]->getCoordTableItem());
	}
	Stereo::GetInstance()->addTempPointPairs(pairs);
}