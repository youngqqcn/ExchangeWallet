#include <QApplication>
#include <QFile>
#include "generate_addr_tool.h"
#include "log4qt/logger.h"
#include "log4qt/logmanager.h"
#include "log4qt/propertyconfigurator.h"
#include "log4qt/ttcclayout.h"
#include "log4qt/fileappender.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString strAppPath = QCoreApplication::applicationDirPath();
    QString strConfigLoc = strAppPath + "/LogFenPage.conf";
    if (QFile::exists(strConfigLoc))
    {
        Log4Qt::PropertyConfigurator::configureAndWatch(strConfigLoc);
    }
    Log4Qt::LogManager::setHandleQtMessages(true);

    GenerateAddrTool genAddrToolWin;
    genAddrToolWin.show();

    return a.exec();
}
