#include "CsvWriter.h"

CsvWriter::CsvWriter(QString fileName)
{
	m_source = fopen(qPrintable(fileName), "w+");
}

void CsvWriter::addCell(QString &cell)
{
	fputs(qPrintable(cell + ","), m_source);
}

void CsvWriter::addCell(float f)
{
	QString cell = QString("%1").arg(f);
	addCell(cell);
}

void CsvWriter::newLine()
{
	fputs("\n", m_source);
}

CsvWriter::~CsvWriter()
{
	fclose(m_source);
}

