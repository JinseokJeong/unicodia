// Qt
#include <QApplication>
#include <QWidget>

// Qt forms
#include "FmMain.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FmMain w;
    w.show();
    return a.exec();
}
