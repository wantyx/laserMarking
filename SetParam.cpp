#include "SetParam.h"

SetParam::SetParam(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.lineEdit->setText(GlobalSetting::instance()->getTemplateName());
	ui.spinBox->setValue(int(GlobalSetting::instance()->getScoreNumber()));
	ui.lineEdit_3->setText(QString::number(GlobalSetting::instance()->getFeatureNumber()));
	ui.lineEdit_4->setText(QString::number(GlobalSetting::instance()->getPadding()));
	connect(ui.save, SIGNAL(clicked()), this, SLOT(save()));
	connect(ui.quit, SIGNAL(clicked()), this, SLOT(exit()));
}

SetParam::~SetParam()
{
}

void SetParam::save()
{
	QString _templateName = ui.lineEdit->text();
	int _scoreNumber = ui.spinBox->value();
	QString _featureNumber = ui.lineEdit_3->text();
	QString _padding = ui.lineEdit_4->text();
	GlobalSetting::instance()->setTemplateName(_templateName);
	GlobalSetting::instance()->setFeatureNumber(_featureNumber.toInt());
	GlobalSetting::instance()->setPadding(_padding.toInt());
	GlobalSetting::instance()->setScoreNumber(_scoreNumber);
}

void SetParam::exit()
{
	this->close();
}

void SetParam::preview()
{
}
