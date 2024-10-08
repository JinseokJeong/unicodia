#ifndef FMMAIN_H
#define FMMAIN_H

#include <QMainWindow>
#include <QAbstractItemModel>

#include "MemStructure.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FmMain; }
QT_END_NAMESPACE

class TreeModel : public QAbstractItemModel
{
public:
    MemFont font;
    mfs::Font structure;

    const mfs::Obj& objOf(const QModelIndex& index) const;

    // QAbstractItemModel
    QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    using QAbstractItemModel::beginResetModel;
    using QAbstractItemModel::endResetModel;
protected:
    QModelIndex indexOf(int row, const mfs::Obj& obj) const noexcept
        { return indexOf(row, &obj); }
    QModelIndex indexOf(int row, const mfs::Obj* obj) const noexcept;
};

class FmMain : public QMainWindow
{
    Q_OBJECT
    using Super = QMainWindow;
    using This = FmMain;
public:
    FmMain(QWidget *parent = nullptr);
    ~FmMain() override;

private:
    Ui::FmMain *ui;
    TreeModel treeModel;

    void selCurrentUpdate();
    void removeDataSpan();
private slots:
    void doOpen();
    void selCurrentChanged(const QModelIndex& nw);
};

#endif // FMMAIN_H
