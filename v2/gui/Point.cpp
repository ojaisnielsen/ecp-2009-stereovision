#include "Point.h"

Point::Point(int nPoint, int x, int y, QGraphicsScene *scene)
{
	m_nPoint = nPoint;
	m_x = x;
	m_y = y;
	circle = new QGraphicsEllipseItem(m_x - 1, m_y - 1, 3, 3, NULL, scene);
	circle->setPen(QPen(QColor("red")));
	circle->setZValue(1);
	label = new QGraphicsSimpleTextItem(QString("%1").arg(m_nPoint), NULL, scene); 
	label->setPos(m_x + 1, m_y + 1);
	label->setPen(QPen(QColor("red")));
	label->setZValue(1);
	m_coordTableItem = new QTableWidgetItem(QString("(%1, %2)").arg(m_x).arg(m_y));
}

Point::~Point()
{
	delete circle;
	delete label;
	delete m_coordTableItem;
}

QTableWidgetItem *Point::getCoordTableItem()
{
	return m_coordTableItem;
}
