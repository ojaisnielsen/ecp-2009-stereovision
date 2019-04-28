#include "MatchPage.h"

MatchPage::MatchPage(int nPair) : PairsPage(Stereo::GetInstance()->getPair(nPair)->getRectView(0), Stereo::GetInstance()->getPair(nPair)->getRectView(1))
{
	m_nPair = nPair;
	Stereo::GetInstance()->getPair(nPair)->getRectView(0)->save("rect0.bmp");
	Stereo::GetInstance()->getPair(nPair)->getRectView(1)->save("rect1.bmp");

	setTitle(QString("Recherche des correspondances pour la paire d'images n°%1").arg(nPair + 1));

	m_radiusLabel = new QLabel("Rayon du patch : ");
	m_radiusBox = new QSpinBox;
	m_radiusBox->setMinimum(1);
	m_radiusBox->setValue(3);

	m_maxDispLabel = new QLabel("Disparité maximale (en pixels) : ");
	m_maxDispBox	= new QSpinBox;
	m_maxDispBox->setMinimum(0);
	m_maxDispBox->setMaximum(150);
	m_maxDispBox->setValue(80);

	m_stepLabel = new QLabel("Largeur du maillage : ");
	m_stepBox = new QSpinBox;
	m_stepBox->setMinimum(1);
	m_stepBox->setValue(2);


	m_nThreadsLabel = new QLabel("Nombre de processus simultanés : ");
	m_nThreadsBox	= new QSpinBox;
	m_nThreadsBox->setMinimum(1);
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	int nCpu = sysinfo.dwNumberOfProcessors;
	m_nThreadsBox->setValue(nCpu);

	m_layout->addWidget(m_radiusLabel, 3, 0);
	m_layout->addWidget(m_radiusBox, 4, 0);
	m_layout->addWidget(m_maxDispLabel, 5, 0);
	m_layout->addWidget(m_maxDispBox, 6, 0);
	m_layout->addWidget(m_stepLabel, 7, 0);
	m_layout->addWidget(m_stepBox, 8, 0);
	m_layout->addWidget(m_nThreadsLabel, 9, 0);
	m_layout->addWidget(m_nThreadsBox, 10, 0);

}

MatchPage::~MatchPage()
{
	delete m_radiusLabel;
	delete m_radiusBox;
	delete m_maxDispLabel;
	delete m_maxDispBox;
	delete m_stepLabel;
	delete m_stepBox;
	delete m_matchProgressBar;
}

bool MatchPage::validatePage()
{
	m_matchProgressBar = new QProgressDialog("Recherche des correspondances...", "Annuler", 0, 1000);
	m_matchProgressBar->setWindowModality(Qt::WindowModal);
	m_matchProgressBar->setMinimumDuration(0);
	m_matchProgressBar->setValue(1);

	Stereo *stereoEnv = Stereo::GetInstance();

	if (stereoEnv->getPair(m_nPair)->isRectified())
	{
		stereoEnv->setMatrices(m_nPair);
	}
	int radius = m_radiusBox->value();
	int maxDisp = m_maxDispBox->value();
	int step = m_stepBox->value();
	int nThreads = m_nThreadsBox->value();

	MrfMatching mrfMatching(stereoEnv->getPair(m_nPair), radius, maxDisp, step, nThreads);
	mrfMatching.includeKnownMatches(Stereo::GetInstance()->getTempPointPairs());

	QList<MatchThread*> threads;
	for (int i = 0; i < nThreads; i++)
	{
		threads << new MatchThread(&mrfMatching, i);
		threads[i]->start();
	}
	
	bool threadRunning = true;
	while (threadRunning)
	{
		m_matchProgressBar->setValue(MatchThread::getStatus());
		Sleep(100);
		threadRunning = false;
		for (int i = 0; i < nThreads; i++)
		{
			threadRunning = threadRunning || threads[i]->isRunning();
		}
	}

	m_matchProgressBar->setValue(1000);

	stereoEnv->getPair(m_nPair)->setDispMap(mrfMatching.disparityMap(), mrfMatching.initialOrder());
	
	for (int i = 0; i < nThreads; i++)
	{
		delete threads[i];
	}


	if (m_nPair < stereoEnv->getNPairs() - 1)
	{
		wizard()->setPage(wizard()->currentId () + 1, new MatchPage(m_nPair + 1));
	}
	else
	{
		wizard()->setPage(wizard()->currentId () + 1, new TriangulatePage(0));
	}

	return true;
}

