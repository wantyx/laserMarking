#include "targetdata.h"

TargetData::TargetData():
    m_centerX(0),
    m_centerY(0),
    m_angle(0),
    topleftX(0),
    topleftY(0)
{

}

TargetData::TargetData(const TargetData &refData)
{
    if(this != &refData) {
        m_centerX = refData.m_centerX;
        m_centerY = refData.m_centerY;
        m_angle = refData.m_angle;
        topleftX = refData.topleftX;
        topleftY = refData.topleftY;
    }
}

TargetData::~TargetData()
{
}

TargetData &TargetData::operator=(const TargetData &refData)
{
    if(this != &refData) {
        m_centerX = refData.m_centerX;
        m_centerY = refData.m_centerY;
        m_angle = refData.m_angle;
        topleftX = refData.topleftX;
        topleftY = refData.topleftY;
    }

    return *this;
}

bool TargetData::operator==(const TargetData &refData) const
{
    return ((m_centerX == refData.m_centerX)&&
            (m_centerY == refData.m_centerY)&&
            (m_angle == refData.m_angle)&&
            (topleftX == refData.topleftX)&&
            (topleftY == refData.topleftY));
}

bool TargetData::operator!=(const TargetData &refData) const
{
    return !(operator==(refData));
}

double TargetData::centerX()const
{
    return m_centerX;
}

void TargetData::setCenterX(const double &x)
{
    if(m_centerX != x){
        m_centerX = x;
    }
}

double TargetData::centerY()const
{
    return m_centerY;
}

void TargetData::setCenterY(const double &y)
{
    if(m_centerY != y){
        m_centerY = y;
    }
}

double TargetData::angle()const
{
    return m_angle;
}

void TargetData::setAngle(const double &an)
{
    if(m_angle != an){
        m_angle = an;
    }
}

double TargetData::getTopleftX() const
{
    return topleftX;
}

void TargetData::setTopleftX(const double& topleftx)
{
    topleftX = topleftx;
}

double TargetData::getTopleftY() const
{
    return topleftY;
}

void TargetData::setTopleftY(const double& toplefty)
{
    topleftY = toplefty;
}
