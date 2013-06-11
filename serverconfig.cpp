/**
 *  This work is distributed under the General Public License,
 *	see LICENSE for details
 *
 *  @author Gwennaël ARBONA
 **/

#include <stdlib.h>
#include "serverconfig.h"

/*----------------------------------------------
      Constructor & destructor
----------------------------------------------*/

ServerConfig::ServerConfig(QWidget *parent) :
    QDialog(parent)
{
    QGridLayout* formGridLayout = new QGridLayout(this);

    // Fields
    editName = new QLineEdit(this);
    editName->setText(getenv("USERNAME"));
    editMap = new QLineEdit(this);
    editMap->setText("LEVEL_01");
    editEmail = new QLineEdit(this);
    editPassword = new QLineEdit(this);
    editPassword->setEchoMode(QLineEdit::Password);
    editMode = new QComboBox(this);
    editMode->addItem("Team Deathmatch", QVariant("G_TeamDeathmatch"));
    editMode->addItem("Capture The Flag", QVariant("G_CaptureTheFlag"));

    // Labels
    labelName = new QLabel(this);
    labelName->setText("Server name");
    labelMap = new QLabel(this);
    labelMap->setText("Level name");
    labelMode = new QLabel(this);
    labelMode->setText("Game mode");
    labelEmail = new QLabel(this);
    labelEmail->setText("Admin email");
    labelPassword = new QLabel(this);
    labelPassword->setText("Server password");

    // Buttons
    buttons = new QDialogButtonBox(this);
    buttons->addButton(QDialogButtonBox::Ok);
    buttons->addButton(QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setText("Launch");
    buttons->button(QDialogButtonBox::Cancel)->setText("Abort and quit");

    // Signals
    connect(buttons->button(QDialogButtonBox::Cancel),
        SIGNAL(clicked()),
        this,
        SLOT(close()));
    connect(buttons->button(QDialogButtonBox::Ok),
        SIGNAL(clicked()),
        this,
        SLOT(SlotEntered()));

    // Layout
    formGridLayout->addWidget(labelName, 0, 0);
    formGridLayout->addWidget(labelMap, 1, 0);
    formGridLayout->addWidget(labelMode, 2, 0);
    formGridLayout->addWidget(labelEmail, 3, 0);
    formGridLayout->addWidget(labelPassword, 4, 0);
    formGridLayout->addWidget(editName, 0, 1);
    formGridLayout->addWidget(editMap, 1, 1);
    formGridLayout->addWidget(editMode, 2, 1);
    formGridLayout->addWidget(editEmail, 3, 1);
    formGridLayout->addWidget(editPassword, 4, 1);

    // Buttons and end
    formGridLayout->addWidget(buttons, 5, 0, 5, 2);
    setLayout(formGridLayout);
    setWindowTitle("Launch server");
    setModal(true);
}

/*----------------------------------------------
           Slots
----------------------------------------------*/

/*--- Signal the data entry ---*/
void ServerConfig::SlotEntered()
{
    QString data;
    QProcess udk(this);
    QStringList argList;

    data = editMap->text();
    data += "?game=" + QString(UDK_GAME_NAME) + "." + editMode->itemData(editMode->currentIndex()).toString();
    data += "?servername=" + editName->text();
    data += "?serveremail=" + editEmail->text();
    data += "?password=" + editPassword->text();
    data += "?dedicated=true";
    argList << "server";
    argList << data;

    udk.startDetached(UDK_EXE_PATH_32, argList);
    udk.waitForStarted();
    close();
}
