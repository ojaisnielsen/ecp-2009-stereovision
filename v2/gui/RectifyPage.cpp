#include "RectifyPage.h"


RectifyPage::RectifyPage(int nPair) : PairsPage(Stereo::GetInstance()->getPair(nPair)->getView(0), Stereo::GetInstance()->getPair(nPair)->getView(1))
{

	m_isRectified = false;
	m_nPair = nPair;

	setTitle(QString("Réctification de la paire d'images n°%1").arg(nPair + 1));

	m_isRectCheckButton = new QPushButton("Ces images sont déja rectifiées");
	m_layout->addWidget(m_isRectCheckButton, 0, 0);

	connect(m_isRectCheckButton, SIGNAL(clicked()), this, SLOT(setRectified()));

}

void RectifyPage::setRectified()
{
	m_isRectified = true;
	wizard()->next();
}

RectifyPage::~RectifyPage()
{
	delete m_isRectCheckButton;
	delete m_rectProgressBar;
}


bool RectifyPage::validatePage()
{
	Stereo *stereoEnv = Stereo::GetInstance();
	if (m_isRectified)
	{
		stereoEnv->getPair(m_nPair)->setRectified();

	}
	else
	{
		m_rectProgressBar = new QProgressDialog("Rectification...", "Annuler", 0, 1000);
		m_rectProgressBar->setWindowModality(Qt::WindowModal);
		m_rectProgressBar->setMinimumDuration(0);
		m_rectProgressBar->setValue(1);

		stereoEnv->setMatrices(m_nPair);
		stereoEnv->dropTempPointPairs();
		RectifyThread thread0(m_nPair, 0);
		RectifyThread thread1(m_nPair, 1);


		thread0.start();
		thread1.start();
		while (thread0.isRunning() || thread1.isRunning())
		{
			m_rectProgressBar->setValue(RectifyThread::getStatus());
			Sleep(100);
		}
		m_rectProgressBar->setValue(1000);
	}

	Stereo::GetInstance()->getPair(m_nPair)->getRectView(0)->save("rect0.bmp");
	Stereo::GetInstance()->getPair(m_nPair)->getRectView(1)->save("rect1.bmp");
	if (m_nPair < stereoEnv->getNPairs() - 1)
	{
		wizard()->setPage(wizard()->currentId () + 1, new RectifyPage(m_nPair + 1));
	}
	else
	{
		wizard()->setPage(wizard()->currentId () + 1, new MatchPage(0));
	}

	return true;
}
