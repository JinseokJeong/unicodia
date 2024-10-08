// Qt
#include <QApplication>
#include <QWidget>
#include <QTranslator>

// Libs
#include <Fonts/TempFont.h>

// Project-local
#include "d_Config.h"

// L10n
#include "LocList.h"
#include "LocManager.h"

// Qt forms
#include "FmMain.h"

namespace {

    ///
    ///  L10n of static data: Unicode DB and some wiki table
    ///
    class LocData : public loc::Subject
    {
    public:
        void translateMe() override;
        static LocData INST;
    };

    LocData LocData::INST;

    void LocData::translateMe()
    {
        // OK: translateMe requires currLang
        uc::finishTranslation(
                    loc::currLang->sortOrder,
                    loc::currLang->ellipsis.blocks,
                    loc::currLang->alphaFixup);
        mywiki::translateDatingLoc();
    }

}   // anon namespace


void initTranslation()
{
    loc::man.add(LocData::INST);
    auto dir = QApplication::applicationDirPath();
    std::filesystem::path pDir = dir.toStdWString();

    loc::allLangs.collect(pDir);
}


int main(int argc, char *argv[])
{
    //qputenv("QT_SCALE_FACTOR", "1.25");
    //qputenv("QT_QPA_PLATFORM", "windows:darkmode=2");
    QApplication a(argc, argv);
    //a.setStyle("fusion");

    uc::completeData();  // …runs once and should not depend on L10n
    initTranslation();

    FmMain w;
    w.installTempPrefix();
    loc::man.add(w);

    // Load config
    {
        auto order = BlockOrder::DEFAULT;
        auto rect = w.geometry();

        config::init(rect, order);

        w.chooseFirstLanguage();
        w.setBlockOrder(order);  // Strange interaction: first language, then order, not vice-versa
        w.configLoaded();

        /// @todo [config] We’ve got a bad position for window divider
        //setGeometry(w, rect);
        //if (config::window::isMaximized)
        //    w.setWindowState(w.windowState() | Qt::WindowMaximized);
    }

    w.show();

    { loc::AutoStop autoStop;
        int r = a.exec();
        config::save(w.normalGeometry(), w.isMaximized(), w.blockOrder());
        return r;
    }   // manager will stop erasing here → speed up exit
}
