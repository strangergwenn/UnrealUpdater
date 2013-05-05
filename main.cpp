/**
 *  This work is distributed under the General Public License,
 *	see LICENSE for details
 *
 *  @author Gwennaël ARBONA
 **/

#include <QApplication>
#include <QRegExp>
#include "common.h"
#include "updater.h"


/*----------------------------------------------
                 Main program
----------------------------------------------*/

/*--- Main program ---*/
int main(int argc, char *argv[])
{
    // Config directory
    QDir tempDir(".");
    tempDir.mkpath(S_CONFIG_DIR);
    QString path = QDir::currentPath();

    // Standard launch
    if (!Updater::GetLock())
    {
        QApplication a(argc, argv);

        // First launch
        if (!QFile::exists(S_DVL_INSTALLED))
        {
            QMessageBox msgBox;
            msgBox.setText("The game will be installed in " + path + "/.\n"
                           + "To change this, press Cancel, then copy this program in the right folder.");
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            int ret = msgBox.exec();
            if (ret == QMessageBox::Cancel)
            {
                return 0;
            }
        }

        // Normal launch
        Updater w;
        w.show();
        return a.exec();
    }

    // Ignore multi-launch
    else
    {
        return 0;
    }
}


/*--- Parse _text_, *text*, !text! to HTML --*/
QString TextToHtml(QString data)
{
    QRegExp rxH("\\*(.*)\\*");
    QRegExp rxM("_(.*)_");
    QRegExp rxI("!(.*)!");

    rxH.setMinimal(true);
    rxM.setMinimal(true);
    rxI.setMinimal(true);

    data.replace(rxH, QString(HTML_TITLE_S) + QString("\\1") + QString(HTML_TITLE_E));
    data.replace(rxM, QString(HTML_MEDIUM_S) + QString("\\1") + QString(HTML_MEDIUM_E));
    data.replace(rxI, QString(HTML_IT_S) + QString("\\1") + QString(HTML_IT_E));
    data.replace("\n", "<br/>");

    return data;
}
