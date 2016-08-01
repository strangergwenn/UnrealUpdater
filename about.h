/**
 *  This work is distributed under the Lesser General Public License,
 *	see LICENSE for details
 *
 *  @author Gwennaël ARBONA
 **/

#ifndef ABOUT_H
#define ABOUT_H

/*----------------------------------------------
Includes & definitions
----------------------------------------------*/

#include "common.h"


/*----------------------------------------------
Class definitions
----------------------------------------------*/

namespace Ui {
	class About;
}

class About : public QDialog
{
	Q_OBJECT

	public:
		explicit About(QWidget *parent = 0);
		~About();

	private:
		Ui::About *ui;
};

extern QString TextToHtml(QString data);

#endif // ABOUT_H
