#ifndef MYGRAPHICSITEM_H
#define MYGRAPHICSITEM_H
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/types_c.h"
#include <QObject>
#include <QWidget>
#include <QPixmap>
#include <QPainter>
#include <QRectF>
#include <QMouseEvent>
#include <QPointF>
#include <QDragEnterEvent>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsItem>
#include "pch.h"
#include "targetdata.h"

class MyGraphicsItem :public QObject, public QGraphicsItem
{
	Q_OBJECT
		Q_INTERFACES(QGraphicsItem)

public:
	MyGraphicsItem();
	MyGraphicsItem(QPixmap *pixmap);
	QRectF  boundingRect() const;
	void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void    wheelEvent(QGraphicsSceneWheelEvent *event);
	void    ResetItemPos();
	void    mousePressEvent(QGraphicsSceneMouseEvent *event);
	void    mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void    mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	qreal   getScaleValue() const;
	void    setQGraphicsViewWH(int nwidth, int nheight);
	QPointF	getStartPos();
	QPointF	getEndPos();
	void		setPaintRect(bool _paint);
	void		setState(int _state);
	QPixmap     m_pix;
	void mScale(qreal scale,QPointF scalePoint);
	std::vector<QPointF> linePoint;
	std::vector<QPointF> getLinePoint();
	void setMove(QPointF mpoint);
	QPointF	getMovePos();
	QPointF getScalePos();
	QPointF getRightPos();
	void setRightPos(QPointF mpoint);
	void setmScale(qreal scale);
	void setrightState(bool nState);
	qreal get_m_scaleDafault() { return m_scaleDafault; }
	QList<TargetData> list;
	void setCenterLine(bool _) { centerLine = _; }
	void setAroundLine(bool _) { aroundLine = _; }
	void setDrawCenterLine(bool _) { drawCenterLine = _; }
	void setDrawRange(bool _) { drawRange = _; }
	QVector<QPointF> vec_centerLine;
	QVector<QPointF> vec_range;
	QVector<QPointF> resultPoints;

signals:
	void startMove();
	void rectPoint();
	void paintState(int);
	void feedbackPoint();
	void rightPos();
	void startScale();

	void captured(cv::Rect rect);
	void removeRect(QPointF point);

private:
	qreal       m_scaleValue;
	qreal       m_scaleDafault;
	
	int         m_zoomState;
	bool        m_isMove;
	QPointF     m_startPos;
	QPointF     m_endPos;
	QPointF     movePos;
	QPointF scalePos;
	bool			m_paintRect;
	int m_state;//0ÒÆ¶¯ 1»­¿× 2»­Ïß 3ÒÆ³ý
	/*QPoint     m_start;
	int width, height;*/

	QPointF spyPoint;
	QPointF rightPoint;
	bool centerLine;
	bool aroundLine;
	bool rightState;
	bool drawCenterLine = false;
	bool drawRange = false;
	
};

#endif // MYGRAPHICSITEM_H
