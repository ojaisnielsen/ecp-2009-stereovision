#ifndef DEF_CSVREADER
#define DEF_CSVREADER

#include <QtGui>
#include "../tools/regminmax.h"

class CsvReader
{
public:
	CsvReader(QString fileName);
	~CsvReader();
	QString getCell(int i, int j);
	int dimi();
	int dimj();

private:
	FILE *m_source;
	int m_dimi;
	int m_dimj;
	QStringList m_content;
	QList<int> m_rowBegining;
};

#endif