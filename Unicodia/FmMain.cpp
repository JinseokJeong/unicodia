// My header
#include "FmMain.h"
#include "ui_FmMain.h"

// Qt
#include <QTableView>
#include <QTextFrame>

// Unicode data
#include "UcDefines.h"

// Misc
#include "u_Strings.h"
#include "u_Qstrings.h"

// Forms
#include "FmPopup.h"

using namespace std::string_view_literals;


///// RowCache /////////////////////////////////////////////////////////////////

RowCache::RowCache(int anCols)
    : fnCols(anCols), fColMask(anCols - 1), fRowMask(~fColMask) {}


const uc::Cp* RowCache::charAt(size_t iRow, unsigned iCol) const
{
    // Have row?
    if (iRow >= rows.size() || iCol >= NCOLS)
        return nullptr;
    auto& rw = rows[iRow];

    return rw.cps[iCol];
}


int RowCache::startingCpAt(size_t iRow) const
{
    if (iRow >= rows.size())
        return uc::NO_CHAR;
    return rows[iRow].startingCp;
}


RowCache::Row& RowCache::ensureRow(unsigned aStartingCp)
{
    if (!rows.empty()) {
        Row& bk = rows.back();
        if (bk.startingCp == aStartingCp)
            return bk;
    }
    Row& newBk = rows.emplace_back(aStartingCp);
    return newBk;
}


void RowCache::addCp(const uc::Cp& aCp)
{
    unsigned code = aCp.subj;
    int rowCp = code & fRowMask;
    int iCol = code & fColMask;

    Row& bk = ensureRow(rowCp);
    bk.cps[iCol] = &aCp;
}


///// CharsModel ///////////////////////////////////////////////////////////////


CharsModel::CharsModel() : rows(NCOLS) {}


int CharsModel::rowCount(const QModelIndex&) const
{
    return rows.nRows();
}


int CharsModel::columnCount(const QModelIndex&) const
    { return NCOLS; }


QVariant CharsModel::data(const QModelIndex& index, int role) const
{
    switch (role) {
    case Qt::DisplayRole: {
            auto cp = rows.charAt(index.row(), index.column());
            if (!cp)
                return {};
            return str::toQ(cp->proxy);
        }

    case Qt::FontRole:
        return fontBasic;

    case Qt::TextAlignmentRole:
        return Qt::AlignCenter;
    default:
        return {};
    }
}


namespace {

    template <int Ncols>
    QString horzHeaderOf(int section);

    template <>
    [[maybe_unused]] inline QString horzHeaderOf<8>(int section)
    {
        return QString::number(section, 16) + QString::number(section + 8, 16);
    }

    template <>
    [[maybe_unused]] inline QString horzHeaderOf<16>(int section)
    {
        return QString::number(section, 16);
    }

}   // anon namespace


QVariant CharsModel::headerData(int section, Qt::Orientation orientation,
                    int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        if (orientation == Qt::Horizontal) {
            return horzHeaderOf<NCOLS>(section);
        } else {
            int c = rows.startingCpAt(section);
            if (static_cast<size_t>(section) >= rows.nRows())
                return {};
            char buf[20];
            snprintf(buf, std::size(buf), "%04x", c);
            return buf;
        }
        break;
    default:
        return {};
    }
}


void CharsModel::addCp(const uc::Cp& aCp)
{
    rows.addCp(aCp);
}


void CharsModel::initFonts()
{
    fontBasic = QFont("Cambria", FONT_SIZE);
    fontBasic.setStyleStrategy(
                static_cast<QFont::StyleStrategy>(QFont::PreferAntialias | QFont::PreferQuality));
}


///// FmMain ///////////////////////////////////////////////////////////////////

FmMain::FmMain(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FmMain)
{
    ui->setupUi(this);

    // Table
    ui->tableChars->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableChars->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableChars->setModel(&model);

    // Fill chars
    auto n = uc::nCps();
    for (unsigned i = 0; i < n; ++i) {
        const auto& cp = uc::cpInfo[i];
        model.addCp(cp);
    }

    // Set fonts
    model.initFonts();
    QFont fontBasicBigger = model.fontBasic;
    fontBasicBigger.setPointSize(50);
    ui->lbTypoEngine->setFont(fontBasicBigger);

    // Connect events
    connect(ui->tableChars->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &This::charChanged);
}

FmMain::~FmMain()
{
    delete ui;
}

namespace {

    template <class T, class Name1, class Name2>
    inline void appendValuePopup(
            QString& text, const T& value, Name1 name, Name2 scheme)
    {
        str::append(text, name);
        str::append(text, ": <a href='");
        str::append(text, scheme);
        str::append(text, ':');
        str::append(text, value.id);
        str::append(text, "' style='color:ForestGreen'>");
        str::append(text, value.locName);
        str::append(text, "</a>");
    }

}


void FmMain::showCp(const uc::Cp& cp)
{
    // Code
    char buf[10];
    snprintf(buf, std::size(buf), "U+%04X", static_cast<unsigned>(cp.subj));
    ui->lbCharCode->setText(buf);

    // OS char
    ui->lbTypoEngine->setText(str::toQ(cp.proxy));

    // Text
    // Header
    QString text;
    str::append(text, "<h1>");
    str::append(text, cp.name.tech);
    str::append(text, "</h1>");

    {   // Info box
        str::append(text, "<p>");
        str::QSep sp(text, "<br>");

        // Script
        auto& scr = cp.script();
        appendValuePopup(text, scr, u8"Письменность", "pop_scr");

        // Unicode version
        sp.sep();
        str::append(text, u8"Версия Юникода: ");
        str::append(text, cp.version().name);
        str::append(text, " (");
        str::append(text, cp.version().year);
        str::append(text, ")");

        // Character type
        sp.sep();
        appendValuePopup(text, cp.category(), u8"Тип", "pop_cat");

        // Numeric value
        if (cp.numeric.isPresent()) {
            sp.sep();
            str::append(text, cp.numeric.type().locName);
            str::append(text, ": ");
            str::append(text, cp.numeric.num);
            if (cp.numeric.denom != 1) {
                str::append(text, "/");
                str::append(text, cp.numeric.denom);
            }
        }

        // Bidi writing
        sp.sep();
        appendValuePopup(text, cp.bidiClass(), u8"В двунаправленном письме", "pop_bidi");

        // UTF-8
        sp.sep();
        auto sChar = str::toQ(cp.subj);
        str::append(text, u8"UTF-8:");
        auto u8 = sChar.toUtf8();
        char buf[10];
        for (unsigned char v : u8) {
            snprintf(buf, 10, " %02X", static_cast<int>(v));
            str::append(text, buf);
        }

        // UTF-16
        sp.sep();
        str::append(text, u8"UTF-16:");
        for (auto v : sChar) {
            snprintf(buf, 10, " %04X", static_cast<int>(v.unicode()));
            str::append(text, buf);
        }

        text.append("</p>");
    }
    ui->vwInfo->setText(text);
}


void FmMain::charChanged(const QModelIndex& current)
{
    if (auto cp = model.charAt(current))
        showCp(*cp);
}


namespace {
    template <class T>
    inline void appendHeader(QString& text, const T& x)
    {
        str::append(text, "<p><nobr><b>");
        str::append(text, x.locName);
        str::append(text, "</b> ("sv);
        str::append(text, x.nChars);
        str::append(text, " шт.)</nobr>");
        str::append(text, "</p>");
    }
}


void FmMain::popupText(const QString& text, QWidget* widget, TinyOpt<QRect> rect)
{
    FmPopup::ensure(popup, this)
            .setText(text)
            .popup(widget, rect);
}


template <class T>
void FmMain::showPopupT(const T& x, QWidget* widget, TinyOpt<QRect> rect)
{
    QString text;
    appendHeader(text, x);
    str::append(text, "<p>");
    str::append(text, x.locDescription);
    str::append(text, "</p>");
    popupText(text, widget, rect);
}


void FmMain::showPopup(
        const uc::BidiClass& x, QWidget* widget, TinyOpt<QRect> rect)
{
    showPopupT(x, widget, rect);
}


void FmMain::showPopup(
        const uc::Category& x, QWidget* widget, TinyOpt<QRect> rect)
{
    showPopupT(x, widget, rect);
}


void FmMain::showPopup(
        const uc::Script& x, QWidget* widget, TinyOpt<QRect> rect)
{
    QString text;
    appendHeader(text, x);

    if (!x.locLangs.empty()) {
        str::append(text, u8"<p>• Языки: ");
        str::append(text, x.locLangs);
        str::append(text, "</p>");
    }

    str::append(text, "<p>");
    str::append(text, x.locDescription);
    str::append(text, "</p>");

    popupText(text, widget, rect);
}


void FmMain::linkClicked(std::string_view scheme, std::string_view target,
                 QWidget* widget, TinyOpt<QRect> rect)
{
    if (scheme == "pop_bidi"sv) {
        if (auto* bidi = uc::findBidiClass(target))
            showPopup(*bidi, widget, rect);
    } else if (scheme == "pop_cat"sv) {
        if (auto* cat = uc::findCategory(target))
            showPopup(*cat, widget, rect);
    } else if (scheme == "pop_scr"sv) {
        if (auto* script = uc::findScript(target))
            showPopup(*script, widget, rect);
    }
}


void FmMain::on_vwInfo_anchorClicked(const QUrl &arg)
{
    auto str = arg.path().toStdString();
    auto things = str::splitSv(str, ':');
    if (things.size() < 2)
        return;

    // Get some parody for link rect
    // Unglitch: we don’t know how to get EXACT coords of link,
    // so improvise somehow
    auto rect = ui->vwInfo->cursorRect();
    rect.setLeft(rect.left() - 80);

    linkClicked(things[0], things[1], ui->vwInfo, rect);
}
