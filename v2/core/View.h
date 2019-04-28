#ifndef DEF_VIEW
#define DEF_VIEW

#include <QtGui>
#include <CImg.h>
#include "../tools/tools.h"
#include "../tools/mathTools.h"
#include "../tools/regminmax.h"

using namespace cimg_library;

class View : public CImg<float>
{
public:
	View();
	View(QString fileName);
	void computeK(float focal, float ccdWidth);
	void setFocal(float focal);
	float getFocal();
	CImg<float> &getK();
	void setK(CImg<float> &K);
	QPixmap &getPixmap();
	void transform(CImg<float> &T, View *output, CImg<int> &region, int *nOper = NULL, int *status = NULL);
	void updatePixmap();
	QString &getFileName();
	
protected:
	QString m_fileName;
	CImg<float> m_K;
	float m_f;
	QPixmap m_pixmap;
};

#endif
