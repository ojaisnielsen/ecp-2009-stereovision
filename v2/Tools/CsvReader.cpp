#include "CsvReader.h"

CsvReader::CsvReader(QString fileName)
{
	m_dimi = 0;
	m_dimj = 0;
	m_source = fopen(qPrintable(fileName), "r");
	char currentChar = fgetc(m_source);
	QString currentCell = "";
	m_rowBegining << 0;
	int nextRowBegining = 0;
	int currentDimj = 0;
	while (currentChar != EOF)
	{
		if (currentChar == ',' || currentChar == '\n')
		{
			m_content << currentCell;
			currentDimj++;
			nextRowBegining++;
			currentCell = "";
			if (currentChar == '\n')
			{
				m_rowBegining << nextRowBegining;
				m_dimj = regmax(m_dimj, currentDimj);
				currentDimj = 0;
				m_dimi++;
			}
		}
		else
		{
			currentCell += currentChar;
		}
		currentChar = fgetc(m_source);
	}
}

CsvReader::~CsvReader()
{
	fclose(m_source);
}

QString CsvReader::getCell(int i, int j)
{
	if (m_rowBegining[i] + j > m_content.size())
	{
		return "";
	}
	else if (m_rowBegining[i] + j >= m_rowBegining[i + 1])
	{
		return "";
	}
	else
	{
		return m_content[m_rowBegining[i] + j];
	}
}

int CsvReader::dimi()
{
	return m_dimi;
}

int CsvReader::dimj()
{
	return m_dimj;
}