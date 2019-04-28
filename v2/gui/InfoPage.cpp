#include "InfoPage.h"

InfoPage::InfoPage(int nView) : QWizardPage()
{
	m_nView = nView;
	m_currentView = Stereo::GetInstance()->getView(nView);

	setTitle(QString("Informations sur la photo n°%1").arg(m_nView + 1));

	QPixmap thumb = m_currentView->getPixmap().scaled(200, 200, Qt::KeepAspectRatio);
	m_thumbLabel = new QLabel;
	m_thumbLabel->setPixmap(thumb);

	PhotoInfo info(m_currentView->getFileName());

	QString link = "<html><a href=\"" + info.searchWeb() + "\">Chercher " + info.getMake() + " sur Internet</a></html>"; 
	m_searchLinkLabel = new QLabel(link);
	m_searchLinkLabel->setTextFormat(Qt::RichText);

	m_focalLabel = new QLabel("Distance focale (mm) : ");
	m_focalBox = new QDoubleSpinBox;
	m_focalBox->setMaximum(200);
	m_focalBox->setValue(info.getFocal());

	m_ccdWidthLabel = new QLabel("Largeur du capteur (mm) : ");
	m_ccdWidthBox = new QDoubleSpinBox;
	m_ccdWidthBox->setMinimum(0.01);
	m_ccdWidthBox->setValue(info.getCcdWidth());


	m_layout = new QGridLayout;
	m_layout->addWidget(m_thumbLabel, 0, 0);

	m_layout->addWidget(m_focalLabel, 1, 0);
	m_layout->addWidget(m_focalBox, 1, 1);

	m_layout->addWidget(m_ccdWidthLabel, 2, 0);
	m_layout->addWidget(m_ccdWidthBox, 2, 1);

	if (info.getMake() != "")
	{
		m_layout->addWidget(m_searchLinkLabel, 0, 1);
	}
	setLayout(m_layout);

	connect(m_searchLinkLabel, SIGNAL(linkActivated(QString)), this, SLOT(followUrl(QString)));

}

void InfoPage::followUrl(QString url)
{
	ShellExecuteA(NULL, "open", qPrintable(url), 0, 0, SW_NORMAL);
}

InfoPage::~InfoPage()
{
	delete m_thumbLabel;
	delete m_focalLabel;
	delete m_ccdWidthLabel;
	delete m_focalBox;
	delete m_ccdWidthBox;
	delete m_layout;
	delete m_searchLinkLabel;
}

bool InfoPage::validatePage()
{
	m_currentView->computeK(m_focalBox->value(), m_ccdWidthBox->value());

	if (m_nView < Stereo::GetInstance()->getNViews() - 1)
	{
		wizard()->setPage(wizard()->currentId () + 1, new InfoPage(m_nView + 1));
	}
	else
	{
		wizard()->setPage(wizard()->currentId () + 1, new RectifyPage(0));
	}
	return true;
}