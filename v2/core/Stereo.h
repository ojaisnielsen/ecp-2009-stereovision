#ifndef DEF_STEREO
#define DEF_STEREO

#include <QtGui>
#include <CImg.h>
#include "../core/View.h"
#include "../core/ViewPair.h"
#include "../core/Singleton.h"
//#include "../tools/mathTools.h"
#include "../tools/regminmax.h"

using namespace cimg_library;


class Stereo : public Singleton<Stereo>
{
public:
	friend Stereo *Singleton<Stereo>::GetInstance();
	friend void Singleton<Stereo>::Kill();

	void loadFiles(QStringList fileNames);
	View *getView(int nView);
	ViewPair *getPair(int nPair);
	int getNViews();
	int getNPairs();
	CImgList<float> &getTempPointPairs();
	void addTempPointPairs(CImgList<float> &pairs);
	void dropTempPointPairs();
	void setMatrices(int nPair);

private:
	Stereo();
	~Stereo();
	QList<View*> m_views;
	QList<ViewPair*> m_pairs;
	CImgList<float> m_tempPointPairs;
};

#endif
