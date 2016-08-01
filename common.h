/**
 *  This work is distributed under the Lesser General Public License,
 *	see LICENSE for details
 *
 *  @author Gwennaël ARBONA
 **/

#ifndef COMMON_H
#define COMMON_H

/*----------------------------------------------
	     Includes
---------------------------------------------*/

#include <QMainWindow>
#include <QDialog>
#include <QThread>
#include <QDebug>
#include <QFile>
#include <QList>
#include <QDir>

#include "Res/project.h"


/*----------------------------------------------
	     Custom types
---------------------------------------------*/

typedef struct File_tag {
    QString     dir;
    QString     file;
    int         size;
} File_t;

#endif // COMMON_H
