#ifndef DEF_POINT
#define DEF_POINT

#include <QtGui>
#include "../tools/regminmax.h"

class Point
{
public:
	Point(int nPoint, int x, int y, QGraphicsScene *scene);
	~Point();
	QTableWidgetItem *getCoordTableItem();

private:
	int m_nPoint;
	int m_x;
	int m_y;
	QGraphicsEllipseItem *circle;
	QGraphicsSimpleTextItem *label;
	QTableWidgetItem *m_coordTableItem;
};

#endif
