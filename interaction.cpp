#include "interaction.h"

Interaction::Interaction(QWidget* parent) {
	ui.setupUi(this);
	connect(ui.ok, SIGNAL(clicked()), this, SLOT(changeInfo()));
}

void Interaction::changeInfo()
{
	GlobalSetting::instance()->setNetAddr(ui.ipAdd->toPlainText());
	GlobalSetting::instance()->setNetPort(ui.port->toPlainText().toInt());
	GlobalSetting::instance()->saveIniFile();
	this->hide();
}
