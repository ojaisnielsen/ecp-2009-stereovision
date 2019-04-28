#ifndef DEF_CLICKABLEPIXMAP
#define DEF_CLICKABLEPIXMAP

#include <QGraphicsPixmapItem>

class PairsPage;

class ClickablePixmap : public QGraphicsPixmapItem //public QObject, 
{
	//Q_OBJECT

public:
	ClickablePixmap(const QPixmap &pixmap, PairsPage *page, int nView);

private:
	PairsPage *m_page;
	int m_nView;
	void hoverMoveEvent(QGraphicsSceneHoverEvent * event);
	void mousePressEvent(QGraphicsSceneMouseEvent * event);

//signals:
//	void mouseMove(QPointF);
//	void mouseClick(QPointF);


};


#endif