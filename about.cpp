/**
 *  This work is distributed under the General Public License,
 *	see LICENSE for details
 *
 *  @author Gwennaël ARBONA
 **/

#include "about.h"
#include "ui_about.h"

/*----------------------------------------------
	 Constructor & destructor
----------------------------------------------*/

About::About(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::About)
{
	ui->setupUi(this);
	ui->aboutText->setHtml(TextToHtml(ABOUT_CONTENT));
	setWindowTitle(ABOUT_TITLE);
    connect(ui->exitButton, &QPushButton::clicked, this, &About::close);
}

About::~About()
{
	delete ui;
}
