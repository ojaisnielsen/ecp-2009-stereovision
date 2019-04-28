#include "WavefrontWriter.h"

WavefrontWriter::WavefrontWriter(QString fileName)
{
	m_source = fopen(qPrintable(fileName), "w+");
}

WavefrontWriter::~WavefrontWriter()
{
	fclose(m_source);
}

void WavefrontWriter::addVertex(float x, float y, float z)
{
	QString line = QString("v %1 %2 %3 \n").arg(x).arg(y).arg(z);
	fputs(qPrintable(line), m_source);
}

void WavefrontWriter::addFace(int point0, int point1, int point2)
{
	QString line = QString("f %1 %2 %3 \n").arg(point0).arg(point1).arg(point2);
	fputs(qPrintable(line), m_source);
}

