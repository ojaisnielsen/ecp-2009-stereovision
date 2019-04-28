#ifndef DEF_WAVEFRONTWRITER
#define DEF_WAVEFRONTWRITER

#include <QtGui>
#include "../tools/regminmax.h"

class WavefrontWriter
{
public:
	WavefrontWriter(QString fileName);
	~WavefrontWriter();
	void addVertex(float x, float y, float z);
	void addFace(int point0, int point1, int point2);

private:
	FILE *m_source;
};

#endif