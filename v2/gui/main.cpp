#include <QtGui>
//#include "../core/stereo.h"
#include "../gui/LoadPage.h"

 
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

	//Stereo *stereoEnv = new Stereo;
	
    LoadPage *loadPage = new LoadPage();//stereoEnv);
	QWizardPage *lastPage = new QWizardPage;

	QWizard wizard;
    wizard.setPage(0, loadPage);
    wizard.setPage(100, lastPage);
    wizard.setWindowTitle("Stéréovision");
    wizard.show();

    return app.exec();

}
