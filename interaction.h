#pragma once
#include "ui_NetSettings.h"
#include "globalsetting.h"
using namespace std;

class Interaction :public QWidget {
	Q_OBJECT
public:
	Interaction(QWidget* parent = Q_NULLPTR);
public slots:
	void changeInfo();
private:
	Ui::Form ui;
};