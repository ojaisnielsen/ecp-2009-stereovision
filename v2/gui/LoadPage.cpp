#include "LoadPage.h"
#include "../tools/mathTools.h"

LoadPage::LoadPage() : QWizardPage()
{

	setTitle("Chargement des images");

	m_loadLabel = new QLabel("Sélectionner les images correspondant aux différentes vues.");
	m_browseButton = new QPushButton("Parcourir...");

	m_layout = new QGridLayout;
	m_layout->addWidget(m_loadLabel, 0, 0);
	m_layout->addWidget(m_browseButton, 0, 1);
	setLayout(m_layout);

	connect(m_browseButton, SIGNAL(clicked()), this, SLOT(loadImages()));
}

LoadPage::~LoadPage()
{
	delete m_loadLabel;
	delete m_browseButton;
	delete m_layout;
	for (int i = 0; i < m_nFiles; i++)
	{
		delete m_thumbsLabels[i];
	}
}

void LoadPage::loadImages()
{
	Stereo *stereoEnv = Stereo::GetInstance();

	QStringList fileNamesList = QFileDialog::getOpenFileNames(this, "Sélectionner des images", m_openFilesPath, "Images (*)");
	m_nFiles = fileNamesList.count();

	if (m_nFiles > 1) 
	{
		m_openFilesPath = fileNamesList[0];

		stereoEnv->loadFiles(fileNamesList);

		QList<QPixmap> thumbs;
		for (int i = 0; i < m_nFiles; i++)
		{
			thumbs << stereoEnv->getView(i)->getPixmap().scaled(100, 100, Qt::KeepAspectRatio);
			m_thumbsLabels << new QLabel;
			m_thumbsLabels[i]->setPixmap(thumbs[i]);
			m_layout->addWidget(m_thumbsLabels[i], 1 + i / 2, i % 2);
		}
	}
}


bool LoadPage::validatePage()
{
	wizard()->setPage(1, new InfoPage(0));
	return true;
}

