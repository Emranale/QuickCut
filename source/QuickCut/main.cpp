
#include "QuickCutShared/QuickCutPCH.h"
#include "Views/MainView.h"
#include "QuickCutShared/Utils/QSingleApplication/QSingleInstance.h"
#include <QtWidgets/QApplication>

int main(int argc, char * argv[])
{
    QSingleInstance instance("8c06d447-da32-447c-adfd-2d227605ae4b");
    if (!instance.tryToRun()) { return 0; }

    QCoreApplication::setApplicationName(QUICKCUT_NAME);
    QCoreApplication::setApplicationVersion(QUICKCUT_VERSION);

    QApplication a(argc, argv);
    MainView     w;
    w.show();
    return a.exec();
}
