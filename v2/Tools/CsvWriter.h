#ifndef DEF_CSVWRITER
#define DEF_CSVWRITER

#include <QtGui>
#include "../tools/regminmax.h"

class CsvWriter
{
public:
	CsvWriter(QString fileName);
	~CsvWriter();
	void addCell(QString &cell);
	void addCell(float cell);
	void newLine();

private:
	FILE *m_source;
};

#endif