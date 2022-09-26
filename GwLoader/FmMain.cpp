#include "FmMain.h"
#include "ui_FmMain.h"

// Qt
#include <QRegularExpression>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>

FmMain::FmMain(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FmMain)
{
    ui->setupUi(this);

    netMan = std::make_unique<QNetworkAccessManager>();

    connect(ui->btStart, &QPushButton::clicked, this, &This::start);
    connect(ui->btStop, &QPushButton::clicked, this, &This::stop);
    connect(netMan.get(), &QNetworkAccessManager::finished, this, &This::requestFinished);

    stopUi();
}

FmMain::~FmMain()
{
    delete ui;
}


void FmMain::clearConsole()
{
    ui->memoConsole->clear();
}


void FmMain::prepareTasks()
{
    auto text = ui->memoGlyphs->toPlainText();
    auto lines = text.split(QRegularExpression("\\s"), Qt::SkipEmptyParts);

    tasks.clear();
    for (auto& p : lines) {
        tasks.push_back(std::make_shared<Task>(p));
    }
}


void FmMain::initPaths()
{
    QDir currentDir = QDir::current();
    work.path = currentDir.filePath(ui->edDirectory->text());
    work.path.mkpath(work.path.absolutePath());
}


void FmMain::start()
{
    clearConsole();
    initPaths();
    prepareTasks();
    ui->btStart->setEnabled(false);
    ui->btStop->setEnabled(true);
    work.isOn = true;
    tryEnqueueReply();
}


void FmMain::stop()
{
    work.isOn = false;
    work.task.load({});
    tasks.clear();
}


void FmMain::stopUi()
{
    ui->btStart->setEnabled(true);
    ui->btStop->setEnabled(false);
}


bool FmMain::isTaskOk(Task* task)
{
    if (!task)
        return false;

    if (work.path.exists(task->fname()))
        return false;

    return true;
}


void FmMain::tryEnqueueReply()
{
    while (!tasks.empty()) {
        auto task = std::move(tasks.front());
        tasks.pop_front();
        if (isTaskOk(task.get())) {
            work.task = std::move(task);
            return;
        } else {
            /// @todo [urgent] print to console
        }
    }
    stop();
    stopUi();
}


void FmMain::processReply(const PTask& task, QNetworkReply* reply)
{
    if (!work.isOn || !task)
        return;

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode != 200) {
        /// @todo [urgent] print to console
        return;
    }

    QFile file(work.path.filePath(task->fname()));
    file.open(QFile::ReadWrite);
}


void FmMain::requestFinished(QNetworkReply* reply)
{
    if (auto task = work.task.exchange({})) {
        processReply(task, reply);
    }
    tryEnqueueReply();
}
