#ifndef DEF_PHOTOINFO
#define DEF_PHOTOINFO

#include <QtGui>
#include <exiv2/image.hpp>
#include <exiv2/exif.hpp>
#include "../tools/regminmax.h"

class PhotoInfo
{
public:
	PhotoInfo(QString fileName);
	QString getMake();
	float getFocal();
	float getCcdWidth();
	QString searchWeb();

private:
	QString m_make;
	float m_focal;
	float m_ccdWidth;
};

#endif