#include "mygraphicsitem.h"
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPointF>
#include <QGraphicsSceneDragDropEvent>
#include <QDrag>
#include <math.h>
#include <QPen>
#include "globalsetting.h"
cv::Rect rect;
MyGraphicsItem::MyGraphicsItem(){

}

MyGraphicsItem::MyGraphicsItem(QPixmap *pixmap)
{
	m_pix = *pixmap;
	setAcceptDrops(true);//If enabled is true, this item will accept hover events; otherwise, it will ignore them. By default, items do not accept hover events.
	m_scaleValue = 0;
	m_scaleDafault = 0;
	m_isMove = false;
	m_paintRect = false;
	m_state = 0;
	rightState = true;
	centerLine = false;
	aroundLine = false;
}

QRectF MyGraphicsItem::boundingRect() const
{
	return QRectF(-m_pix.width() / 2, -m_pix.height() / 2,
		m_pix.width(), m_pix.height());
}

void MyGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *,
	QWidget *)
{
	//painter->setCompositionMode(QPainter::CompositionMode_Darken);
	
	painter->drawPixmap(-m_pix.width() / 2, -m_pix.height() / 2, m_pix);
	if (m_state == 2)
	{
		QPen pen;
		pen.setColor(Qt::red);
		pen.setWidth(1);
		painter->setPen(pen);
		int x, y;
		if (m_endPos.x() > m_startPos.x())
		{
			x = m_startPos.x();
		}
		else
		{
			x = m_endPos.x();
		}
		if (m_endPos.y() > m_startPos.y())
		{
			y = m_startPos.y();
		}
		else
		{
			y = m_endPos.y();
		}
		painter->drawRect(x, y, abs(m_endPos.x() - m_startPos.x()), abs(m_endPos.y() - m_startPos.y()));
		//painter->drawEllipse(x, y, abs(m_endPos.x() - m_startPos.x()), abs(m_endPos.y() - m_startPos.y()));

			
		for (int i = 0; i < resultPoints.size(); i=i+4) {
			QPen pen1;
			pen1.setColor(Qt::blue);
			pen1.setWidth(5);
			painter->setPen(pen1);
			QPointF topLeft = QPoint(resultPoints.at(i).x() - m_pix.width() / 2, resultPoints.at(i).y() - m_pix.height() / 2);
			QPointF topright = QPoint(resultPoints.at(i+1).x() - m_pix.width() / 2, resultPoints.at(i+1).y() - m_pix.height() / 2);
			QPointF bottomright = QPoint(resultPoints.at(i+2).x() - m_pix.width() / 2, resultPoints.at(i+2).y() - m_pix.height() / 2);
			QPointF bottomleft = QPoint(resultPoints.at(i+3).x() - m_pix.width() / 2, resultPoints.at(i+3).y() - m_pix.height() / 2);
			painter->drawLine(topLeft, topright);
			painter->drawLine(topright, bottomright);
			painter->drawLine(bottomright, bottomleft);
			painter->drawLine(bottomleft, topLeft);
		}
			
			/*painter->drawRect(list[i].getTopleftX() - m_pix.width() / 2,
				list[i].getTopleftY() - m_pix.height() / 2,
				2 * (list[i].centerX() - list[i].getTopleftX()),
				2 * (list[i].centerY() - list[i].getTopleftY()));*/



		rect = cv::Rect(x+ m_pix.width() / 2, y+ m_pix.height() / 2, abs(m_endPos.x() - m_startPos.x()), abs(m_endPos.y() - m_startPos.y()));
		
		
	}
	//������state==1 ���ʾ����
	if (m_state == 1 && drawCenterLine) {
		QPen pen;
		pen.setColor(Qt::red);
		pen.setWidth(1);
		painter->setPen(pen);
		painter->drawLine(QPointF(m_startPos), QPointF(m_endPos));
		for (int i = 0; i < vec_centerLine.size(); i++) {
			QPointF p1 = vec_centerLine.at(i);
			QPointF p2 = vec_centerLine.at(++i);
			painter->drawLine(p1, p2);
		}
	}
	if (m_state == 1 && drawRange) {
		QPen pen;
		pen.setColor(Qt::red);
		pen.setWidth(1);
		painter->setPen(pen);
		painter->drawLine(QPointF(m_startPos), QPointF(m_endPos));
		for (int i = 0; i < vec_range.size(); i++) {
			QPointF p1 = vec_range.at(i);
			QPointF p2 = vec_range.at(++i);
			painter->drawLine(p1, p2);
		}
	}
	if (centerLine) {
		QPen pen;
		pen.setColor(Qt::red);
		pen.setWidth(3);
		painter->setPen(pen);
		for (int i = 0; i < GlobalSetting::instance()->vec_centerLine.size(); i++) {
			QPointF p1 = GlobalSetting::instance()->vec_centerLine.at(i);
			QPointF p2 = GlobalSetting::instance()->vec_centerLine.at(++i);
			painter->drawLine(p1, p2);
		}
	}
	if (aroundLine) {
		QPen pen;
		pen.setColor(Qt::red);
		pen.setWidth(3);
		painter->setPen(pen);
		for (int i = 0; i < GlobalSetting::instance()->vec_range.size(); i++) {
			QPointF p1 = GlobalSetting::instance()->vec_range.at(i);
			QPointF p2 = GlobalSetting::instance()->vec_range.at(++i);
			painter->drawLine(p1, p2);
		}
	}
	/*else
	{
		QPen pen;
		pen.setColor(Qt::red);
		pen.setWidth(5);
		painter->setPen(pen);
		
		painter->drawLine(m_startPos, m_endPos);
	}*/

}

void MyGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_startPos = event->pos();//������ʱ����ȡ��ǰ�����ͼƬ�е����꣬
		m_isMove = true;//���������������
		//emit startMove();

		linePoint.clear();
		if (m_state == 1)
		{
			linePoint.push_back(m_startPos);
		}
		
		//if (event->modifiers() == Qt::CTRL)
		//{
		//	m_paintRect = true;
		//}
		//else
		//{
		//	qDebug() << "myItem" << m_startPos;
		//	m_startPos = event->pos();//������ʱ����ȡ��ǰ�����ͼƬ�е����꣬
		//	m_isMove = true;//���������������
		//}

	}
	else if (event->button() == Qt::RightButton)
	{
		ResetItemPos();//�һ�������ô�С
		
	}


}

void MyGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	//״̬0 �϶�ͼ��
	if (m_isMove&&m_state==0)
	{
		
		movePos = event->pos();
		QPointF point = (event->pos() - m_startPos)*m_scaleValue;
		if (point != spyPoint)
		{
			//qDebug() << "111111" << point;
			moveBy(point.x(), point.y());
			emit startMove();
		}
		spyPoint = point;
	}

	if (m_isMove&&m_state!=0 )
	{
		m_endPos = event->pos();
		update();
		//qDebug() << "linePoint:" << m_endPos;
		linePoint.push_back(m_endPos);
	}
}

void MyGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	int bb = 0;
	if (event->button() == Qt::LeftButton)
	{
		m_endPos = event->pos();
		m_isMove = false;//����������Ѿ�̧��
		//״̬1 ���� ÿ�����������𷢳�һ���ź�
		if (m_state==1)
		{
			update();
			emit rectPoint();
		}
		else if (m_state == 2){
			update();
			if (rect.width != 0 && rect.height != 0)
			{
				emit captured(rect);
			}
		}
		else if (m_state == 3)
		{
			update();
			QPointF	point= QPointF();
			float a = m_startPos.x() + m_pix.width() / 2.0;
			float b = m_startPos.y() + m_pix.height() / 2.0;
			point.setX(point.x() +a);
			point.setY(point.y() + b);
			//point.x() = m_startPos.x() + m_pix.width() / 2;
			//point.y() = m_startPos.y() + m_pix.height() / 2;
			emit removeRect(point);
		}
		//emit feedbackPoint();
	}
	else if (event->button() == Qt::RightButton)
	{

		rightPoint = event->pos();
		emit rightPos();
		update();

		if (m_state == 2 && m_endPos.x() != m_startPos.x() && m_endPos.y() != m_startPos.y())
		{
			QPointF  cur_Pos = event->pos();
			if (((cur_Pos.x() > m_startPos.x() && cur_Pos.x()<m_endPos.x()) || (cur_Pos.x()<m_startPos.x() && cur_Pos.x()>m_endPos.x())) &&
				((cur_Pos.y()> m_startPos.y() && cur_Pos.y() < m_endPos.y()) || (cur_Pos.y()<m_startPos.y() && cur_Pos.y()>m_endPos.y())))
			{
				//�ڵ� ��Ϊǰ��
				emit paintState(1);
			}
			else
			{
				//��� ��Ϊ����
				emit paintState(0);
			}

		}
	}
	if (drawCenterLine) {
		vec_centerLine.push_back(m_startPos);
		vec_centerLine.push_back(m_endPos);
	}
	if (drawRange) {
		vec_range.push_back(m_startPos);
		vec_range.push_back(m_endPos);
	}

}


void MyGraphicsItem::wheelEvent(QGraphicsSceneWheelEvent *event)//�������¼�
{
	if ((event->delta() > 0) && (m_scaleValue >= 50))//���Ŵ�ԭʼͼ���50��
	{
		return;
	}
	else if ((event->delta() < 0) && (m_scaleValue <= m_scaleDafault*0.5))//ͼ����С������Ӧ��С֮��Ͳ�������С
	{
		return;
		//ResetItemPos();//����ͼƬ��С��λ�ã�ʹ֮����Ӧ�ؼ����ڴ�С
	}
	else
	{
		qreal qrealOriginScale = m_scaleValue;
		//qDebug() << "earlyScale" << qrealOriginScale;
		if (event->delta() > 0)//��������ǰ����
		{
			m_scaleValue *= 1.1;//ÿ�ηŴ�10%
		}
		else if (event->delta() < 0)
		{
			m_scaleValue *= 0.9;//ÿ����С10%
		}
		setScale(m_scaleValue);
		scalePos = event->pos();
		if (event->delta() > 0)
		{
			moveBy(-event->pos().x()*qrealOriginScale*0.1, -event->pos().y()*qrealOriginScale*0.1);//ʹͼƬ���ŵ�Ч��������������������ڵ�Ϊ���Ľ������ŵ�
		}
		else if (event->delta() < 0)
		{
			moveBy(event->pos().x()*qrealOriginScale*0.1, event->pos().y()*qrealOriginScale*0.1);//ʹͼƬ���ŵ�Ч��������������������ڵ�Ϊ���Ľ������ŵ�
		}
		emit startScale();
	}
}

void MyGraphicsItem::setQGraphicsViewWH(int nwidth, int nheight)//��������Ŀؼ�QGraphicsView��width��height���������У�������ͼ��ĳ���Ϳؼ��ĳ���ı�����ʹͼƬ���ŵ��ʺϿؼ��Ĵ�С
{
	int nImgWidth = m_pix.width();
	int nImgHeight = m_pix.height();
	qreal temp1 = nwidth*1.0 / nImgWidth;
	qreal temp2 = nheight*1.0 / nImgHeight;
	if (temp1 > temp2)
	{
		m_scaleDafault = temp2;
	}
	else
	{
		m_scaleDafault = temp1;
	}
	setScale(m_scaleDafault);
	m_scaleValue = m_scaleDafault;
}

void MyGraphicsItem::ResetItemPos()//����ͼƬλ��
{
	m_scaleValue = m_scaleDafault;//���ű����ص�һ��ʼ������Ӧ����
	setScale(m_scaleDafault);//���ŵ�һ��ʼ������Ӧ��С
	setPos(0, 0);
}

qreal MyGraphicsItem::getScaleValue() const
{
	return m_scaleValue;
}

QPointF	MyGraphicsItem::getStartPos(){
	return m_startPos;
}

QPointF	MyGraphicsItem::getEndPos(){
	return m_endPos;
}

QPointF MyGraphicsItem::getMovePos() {
	return movePos;
}

QPointF MyGraphicsItem::getScalePos() {
	return scalePos;
}

void	 MyGraphicsItem::setPaintRect(bool _paint){
	m_paintRect = _paint;
}

//�������ģʽ
void	 MyGraphicsItem::setState(int _state){
	m_state = _state;
}

void MyGraphicsItem::mScale(qreal scale, QPointF scalePoint)
{
	qreal qrealRign = m_scaleValue;
	if (scale > m_scaleValue)
	{
		m_scaleValue = scale;
		setScale(m_scaleValue);
		moveBy(-scalePoint.x()*qrealRign*0.1, -scalePoint.y()*qrealRign*0.1);
	}
	else if (scale < m_scaleValue)
	{
		m_scaleValue = scale;
		setScale(m_scaleValue);
		moveBy(scalePoint.x()*qrealRign*0.1, scalePoint.y()*qrealRign*0.1);
	}
	
}

void MyGraphicsItem::setmScale(qreal scale)
{
	m_scaleValue = scale;
	setScale(m_scaleValue);
}

std::vector<QPointF> MyGraphicsItem::getLinePoint()
{
	return linePoint;
}

void MyGraphicsItem::setMove(QPointF mpoint)
{
	moveBy(mpoint.x(), mpoint.y());
}

QPointF MyGraphicsItem::getRightPos()
{
	return rightPoint;
}

void MyGraphicsItem::setRightPos(QPointF mpoint)
{
	rightPoint = mpoint;
	update();
}
void MyGraphicsItem::setrightState(bool nState)
{
	rightState = nState;
	if (!nState || (nState && !rightPoint.isNull()))
	{
		rightPoint = QPointF(0,0);
	}
	update();
}