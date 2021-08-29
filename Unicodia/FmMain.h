#ifndef FMMAIN_H
#define FMMAIN_H

// C++

// Qt
#include <QMainWindow>
#include <QAbstractTableModel>
#include <QStyledItemDelegate>

// My libs
#include "u_Vector.h"
#include "u_TinyOpt.h"

// Project-local
#include "FontMatch.h"
#include "MyWiki.h"

// Forms
#include "FmPopup.h"

#include "UcData.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FmMain; }
QT_END_NAMESPACE


constexpr int NCOLS = 8;
constexpr int FSZ_TABLE = 15;
constexpr int FSZ_BIG = 50;

class FmPopup2;

struct MaybeChar {
    char32_t code = 0;
    const uc::Cp* cp = nullptr;
    explicit operator bool() const { return cp; }
    const uc::Cp& operator * () const { return *cp; }
    const uc::Cp* operator ->() const { return  cp; }
    bool hasCp() const { return cp; }
};


struct CacheCoords {
    size_t row = 0;
    unsigned col = 0;
};


class RowCache
{
public:
    RowCache(int anCols);

    size_t nRows() const { return rows.size(); }
    size_t nCols() const { return fnCols; }

    void addCp(const uc::Cp& aCp);

    /// @return  code point if it’s really present
    MaybeChar charAt(size_t iRow, unsigned iCol) const;

    /// @return  starting code point of row; or NO_CHAR if bad row
    int startingCpAt(size_t iRow) const;

    CacheCoords findCode(char32_t code) const;
    void clear() { rows.clear(); }
protected:
    const int fnCols, fColMask, fRowMask;

    struct Row
    {
        unsigned startingCp = 0xFFFFFF;

        const uc::Cp* cps[NCOLS];

        Row() { clear(); }
        Row(int aStartingCp) : startingCp(aStartingCp) { clear(); }

        void clear() { std::fill(std::begin(cps), std::end(cps), nullptr); }
    };

    SafeVector<Row> rows;
    Row& ensureRow(unsigned aStartingCp);
    static bool isLessRC(const Row& x, char32_t y);
};


enum class TableColors { NO, YES };


class CharsModel : public QAbstractTableModel
{
    using Super = QAbstractTableModel;
public:
    QWidget* const owner;
    FontMatch match;
    bool isCjkCollapsed = true;

    CharsModel(QWidget* aOwner);

    int rowCount(const QModelIndex& = {}) const override;
    int columnCount(const QModelIndex&) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    const QFont* fontAt(const QModelIndex& index) const;
    const QFont* fontAt(const uc::Cp& cp) const;
    QColor fgAt(const QModelIndex& index, TableColors tcl) const;
    QColor fgAt(const uc::Cp& cp, TableColors tcl) const;
    QString textAt(const QModelIndex& index) const;
    QString textAt(const uc::Cp& cp) const;
    void addCp(const uc::Cp& aCp);
    MaybeChar charAt(const QModelIndex& index) const
            { return rows.charAt(index.row(), index.column()); }
    QModelIndex indexOf(char32_t code);
    void build();
    using Super::beginResetModel;
    using Super::endResetModel;
private:
    RowCache rows;
    mutable struct Hint {
        const uc::Block* cell = &uc::blocks[0];
    } hint;
};


class BlocksModel : public QAbstractTableModel
{
public:
    int rowCount(const QModelIndex&) const override;
    int columnCount(const QModelIndex&) const override;
    QVariant data(const QModelIndex& index, int role) const override;
};


class WiCustomDraw : public QWidget
{
    using Super = QWidget;
public:
    using Super::Super;
    void setAbbreviation(std::u8string_view x, char32_t aSubj);
protected:
    void paintEvent(QPaintEvent *event);
private:
    enum class Mode { NONE, ABBREVIATION };
    Mode mode = Mode::NONE;
    std::u8string_view abbreviation;
    char32_t subj = 0;
};


class FmMain : public QMainWindow,
               private mywiki::Gui
{
    Q_OBJECT
    using Super = QMainWindow;
    using This = FmMain;
    friend class FmPopup2;

public:
    FmMain(QWidget *parent = nullptr);
    ~FmMain() override;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::FmMain *ui;
    CharsModel model;
    BlocksModel blocksModel;
    std::unique_ptr<FmPopup2> popup;
    QFont fontBig;
    char32_t shownCp = uc::NO_CHAR;
    mutable struct Hint {
        const uc::Block* sample = &uc::blocks[0];
    } hint;

    class CharsDelegate : public QStyledItemDelegate
    {
    private:
        using Super = QStyledItemDelegate;
    public:
        FmMain& owner;

        CharsDelegate(FmMain& aOwner) : owner(aOwner) {}
        void paint(QPainter *painter, const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
    protected:
        void tryDrawCustom(QPainter* painter, const QRect& rect,
                    const QModelIndex& index, const QColor& color) const;
        void initStyleOption(QStyleOptionViewItem *option,
                             const QModelIndex &index) const override;
    } charsDelegate;

    void showCp(MaybeChar ch);
    void linkClicked(std::string_view scheme, std::string_view target,
                     QWidget* widget, TinyOpt<QRect> rect);
    void linkClicked(std::string_view link, QWidget* widget, TinyOpt<QRect> rect);
    template <class T>
    [[deprecated]] void showPopupT(const T& x, QWidget* widget, TinyOpt<QRect> rect);
    [[deprecated]] void showPopup(const uc::Block& x, QWidget* widget, TinyOpt<QRect> rect);
    [[deprecated]] void showPopup(const uc::Term& x, QWidget* widget, TinyOpt<QRect> rect);
    [[deprecated]] void showFonts(char32_t cp, QFontDatabase::WritingSystem ws,
                   QWidget* widget, TinyOpt<QRect> rect);
    [[deprecated]] void popupText(const QString& text, QWidget* widget, TinyOpt<QRect> rect);
    void selectChar(char32_t code);
    void drawSampleWithQt(const uc::Cp& cp);
    void initTerms();
    void reflectCjkCollapseState();

    // mywiki::Gui
    void popupAtAbs(
            QWidget* widget, const QRect& absRect, const QString& html) override;
    FontList allSysFonts(
            char32_t cp, QFontDatabase::WritingSystem ws, size_t maxCount) override;
private slots:
    void charChanged(const QModelIndex& current);
    void copyCurrentChar();
    void copyCurrentSample();
    void popupLinkActivated(const QString& link);
    void anchorClicked(const QUrl &arg1);
    void cjkExpandCollapse();
    void on_comboBlock_currentIndexChanged(int index);
};


class FmPopup2 : public FmPopup
{
    using Super = FmPopup;
    using This = FmPopup2;
public:
    FmPopup2(FmMain* owner);
    ~FmPopup2() override = default;
};

#endif // FMMAIN_H
