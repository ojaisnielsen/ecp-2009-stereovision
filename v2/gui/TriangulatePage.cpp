#include "TriangulatePage.h"

#include <QtGui>
#include "../core/Stereo.h"
#include "../tools/tools.h"

TriangulatePage::TriangulatePage(int nPair) : QWizardPage()
{
	m_nPair = nPair;
	setTitle("Triangulation");

	Stereo::GetInstance()->getPair(nPair)->getDispMap().get_normalize(0, 254).save("dispmap.bmp");
	Stereo::GetInstance()->getPair(m_nPair)->compute3dCoords();
	coordsMapToWaveFront(Stereo::GetInstance()->getPair(m_nPair)->get3dCoordsMap(), "testos.obj");


	m_disparityLabel = new QLabel;
	QPixmap disparityMap = cimgToQpixmap(Stereo::GetInstance()->getPair(nPair)->getDispMap());
	m_disparityLabel->setPixmap(disparityMap);

	m_displayButton = new QPushButton("Visualiser le modèle 3D");
	m_saveButton = new QPushButton("Sauvegarder le modèle 3D au format WaveFront");

	m_layout = new QGridLayout;
	m_layout->addWidget(m_disparityLabel, 0, 0);
	m_layout->addWidget(m_displayButton, 1, 0);
	m_layout->addWidget(m_saveButton, 2, 0);
	setLayout(m_layout);

	connect(m_saveButton, SIGNAL(clicked()), this, SLOT(saveModel()));

}

void TriangulatePage::saveModel()
{
	m_fileName = QFileDialog::getSaveFileName(this, "Enregistrer le modèle", m_fileName, "Objet WaveFront (*.obj)");
	if (m_fileName != "")
	{
		Stereo::GetInstance()->getPair(m_nPair)->compute3dCoords();
		coordsMapToWaveFront(Stereo::GetInstance()->getPair(m_nPair)->get3dCoordsMap(), m_fileName);
	}
}

TriangulatePage::~TriangulatePage()
{
	delete m_disparityLabel;
	delete m_displayButton;
	delete m_saveButton;
	delete m_layout;
}