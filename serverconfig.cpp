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
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QFormLayout* layout = new QFormLayout(this);

    // Fields
    editName = new QLineEdit(this);
    editName->setText(getenv("USERNAME"));
    editMap = new QLineEdit(this);
    editMap->setText("LEVEL_01");
    editEmail = new QLineEdit(this);
    editPassword = new QLineEdit(this);
    editPassword->setEchoMode(QLineEdit::Password);
    editClientPassword = new QLineEdit(this);
    editClientPassword->setEchoMode(QLineEdit::Password);
    editMode = new QComboBox(this);
    editMode->addItem("Team Deathmatch", QVariant("G_TeamDeathmatch"));
    editMode->addItem("Capture The Flag", QVariant("G_CaptureTheFlag"));
    comment = new QLabel(this);
    comment->setWordWrap(true);
    comment->setText("<i>Before a server can be displayed in the server browser, administrators must register at deepvoid.eu/play</i>");
    Load();

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

    // Form layout
    layout->addRow("Server name", editName);
    layout->addRow("Level name", editMap);
    layout->addRow("Game mode", editMode);
    layout->addRow("Admin email", editEmail);
    layout->addRow("Admin password", editPassword);
    layout->addRow("Server password", editClientPassword);

    // Main layout
    mainLayout->addLayout(layout);
    mainLayout->addWidget(comment);
    mainLayout->addWidget(buttons);
    setLayout(mainLayout);
    setFixedSize(350, 250);
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
    data += "?serverpassword=" + editPassword->text();
    data += "?GamePassword=" + editClientPassword->text();
    data += "?dedicated=true";
    argList << "server";
    argList << data;

    Save();
    udk.startDetached(UDK_EXE_PATH_32, argList);
    udk.waitForStarted();
    close();
}


/*--- Save the config to settings ---*/
void ServerConfig::Save()
{
    QFile settingFile(S_SERVER_SETTINGS);

    settingFile.open(QFile::WriteOnly);
    settingFile.write((editName->text() + "\n").toStdString().c_str());
    settingFile.write((editMap->text() + "\n").toStdString().c_str());
    settingFile.write((editEmail->text() + "\n").toStdString().c_str());
    settingFile.write((editClientPassword->text() + "\n").toStdString().c_str());
    settingFile.close();
}


/*--- Load the config from settings ---*/
void ServerConfig::Load()
{
    QList<QByteArray> content;
    QFile settingFile(S_SERVER_SETTINGS);

    settingFile.open(QFile::ReadOnly);
    content = settingFile.readAll().split('\n');
    if (content.length() > 3)
    {
        editName->setText(QString(content[0].data()));
        editMap->setText(QString(content[1].data()));
        editEmail->setText(QString(content[2].data()));
        editClientPassword->setText(QString(content[3].data()));
    }
    settingFile.close();
}

