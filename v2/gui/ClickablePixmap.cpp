#include "ClickablePixmap.h"
#include "../gui/PairsPage.h"

#include <QtGui>

ClickablePixmap::ClickablePixmap(const QPixmap &pixmap, PairsPage *page, int nView) : QGraphicsPixmapItem(pixmap)
{
	setAcceptHoverEvents(true);
	m_page = page;
	m_nView = nView;
}

void ClickablePixmap::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
	QPointF pos = mapToItem(this, event->pos());
	m_page->showPoint(m_nView, pos.x(), pos.y());
	//emit mouseMove(event->pos());
}

void ClickablePixmap::mousePressEvent(QGraphicsSceneMouseEvent  *event)
{
	QPointF pos = mapToItem(this, event->pos());
	m_page->addPoint(m_nView, pos.x(), pos.y());
	//emit mouseClick(event->pos());
} 
