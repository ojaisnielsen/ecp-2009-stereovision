#include "PhotoInfo.h"

PhotoInfo::PhotoInfo(QString fileName)
{
	try
	{
	
		Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(qPrintable(fileName));
		image->readMetadata();
		Exiv2::ExifData ed = image->exifData();
		m_focal = ed["Exif.Photo.FocalLength"].toFloat();
		float xRes = ed["Exif.Image.XResolution"].toFloat();
		float width = ed["Exif.Image.ImageWidth"].toFloat();
		m_make = ed["Exif.Image.Model"].toString().c_str();
		int unit = ed["Exif.Image.ResolutionUnit"].toFloat();
		if (unit == 2.)
		{
			m_ccdWidth = 25.4 * width / xRes;
		}
		else
		{
			m_ccdWidth = width / xRes;
		}

	}

	catch (Exiv2::AnyError& e) 
	{
		m_focal = 0;
		m_make = "";
		m_ccdWidth = 0;
	}

}

QString PhotoInfo::getMake()
{
	return m_make;
}

float PhotoInfo::getFocal()
{
	return m_focal;
}

float PhotoInfo::getCcdWidth()
{
	return m_ccdWidth;
}

QString PhotoInfo::searchWeb()
{
	QString urlBase = "http://www.google.com/search";
	QString key0 = "btnI";
	QString key1 = "q";
	QString val0 = "I'm Feeling Lucky";
	QString val1 = "site:www.dpreview.com/reviews/specs/ " + m_make;

	QUrl url = QUrl(urlBase);
	url.addQueryItem(key0, val0);
	url.addQueryItem(key1, val1);
	return url.toString();
}
