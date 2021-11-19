#pragma once

#include <QWidget>
#include "ui_SetParam.h"
#include "globalsetting.h"

class SetParam : public QWidget
{
	Q_OBJECT

public:
	SetParam(QWidget *parent = Q_NULLPTR);
	~SetParam();

public slots:
	void save();
	void exit();
	void preview();



private:
	Ui::SetParam ui;
};
