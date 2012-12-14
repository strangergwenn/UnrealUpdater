/**
 *  This work is distributed under the General Public License,
 *	see LICENSE for details
 *
 *  @author Gwennaël ARBONA
 **/

#ifndef PASSWORD_H
#define PASSWORD_H

/*----------------------------------------------
    Includes & definitions
----------------------------------------------*/

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QComboBox>
#include <QGridLayout>
#include <QStringList>
#include <QDebug>

/*----------------------------------------------
    Class definitions
----------------------------------------------*/

class Password : public QDialog
{
    Q_OBJECT
public:
    explicit Password(QWidget *parent = 0);
    
signals:
    void PasswordEntered(QString password);
    
public slots:
    void SlotPassword();

private:
    QLineEdit* editPassword;
    QDialogButtonBox* buttons;
    
};

#endif // PASSWORD_H
