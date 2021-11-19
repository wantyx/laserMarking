#ifndef TARGETDATA_H
#define TARGETDATA_H

class TargetData
{
public:
    TargetData();
    TargetData(const TargetData &refData);
    ~TargetData();

    TargetData &operator=(const TargetData &refData);
    bool operator==(const TargetData &refData) const;
    bool operator!=(const TargetData &refData) const;

    double centerX()const;
    void setCenterX(const double &x);

    double centerY()const;
    void setCenterY(const double &y);

    double angle()const;
    void setAngle(const double &an);

    double getTopleftX()const;
    void setTopleftX(const double& topleftx);

    double getTopleftY()const;
    void setTopleftY(const double& toplefty);

private:
    double m_centerX;
    double m_centerY;
    double m_angle;
    double topleftX;
    double topleftY;
};

#endif // TARGETDATA_H
