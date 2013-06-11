/**
 *  This work is distributed under the General Public License,
 *	see LICENSE for details
 *
 *  @author Gwennaël ARBONA
 **/

#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

/*----------------------------------------------
    Includes & definitions
----------------------------------------------*/

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QStringList>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QProcess>
#include <QLabel>

#include "common.h"


/*----------------------------------------------
    Class definitions
----------------------------------------------*/

#include <QDialog>

class ServerConfig : public QDialog
{
    Q_OBJECT
    public:
        explicit ServerConfig(QWidget *parent = 0);

    public slots:
        void SlotEntered();

    private:
        QLabel* labelMap;
        QLabel* labelMode;
        QLabel* labelName;
        QLabel* labelEmail;
        QLabel* labelPassword;

        QLineEdit* editMap;
        QLineEdit* editName;
        QLineEdit* editEmail;
        QLineEdit* editPassword;
        QComboBox* editMode;

        QDialogButtonBox* buttons;
    
};

#endif // SERVERCONFIG_H
