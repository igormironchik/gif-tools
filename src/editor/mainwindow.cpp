/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF editor include.
#include "mainwindow.hpp"
#include "crop.hpp"
#include "drawarrow.hpp"
#include "drawrect.hpp"
#include "frameontape.hpp"
#include "mainwindow_private.hpp"
#include "settings.hpp"
#include "tape.hpp"
#include "text.hpp"
#include "uistates.hpp"
#include "version.hpp"

// Qt include.
#include <QActionGroup>
#include <QCloseEvent>
#include <QColorDialog>
#include <QFileDialog>
#include <QFinalState>
#include <QMenuBar>
#include <QMessageBox>
#include <QMetaMethod>
#include <QPainter>
#include <QSignalTransition>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTimer>
#include <QWindow>
#include <QtConcurrent>

// gif-widgets include.
#include "license_dialog.hpp"
#include "utils.hpp"

namespace /* anonymous */
{

void writeGIFFunc(QPromise<void> &,
                  BusyIndicator *receiver,
                  const QStringList &files,
                  const QVector<int> &delays,
                  const QString &fileName)
{
    QGifLib::Gif gif;

    QObject::connect(&gif, &QGifLib::Gif::writeProgress, receiver, &BusyIndicator::setPercent);

    gif.write(fileName, files, delays, 0);
}

void cropGIFFunc(QPromise<void> &,
                 BusyIndicator *receiver,
                 QGifLib::Gif *container,
                 const QRect &rect)
{
    const auto index = receiver->metaObject()->indexOfProperty("percent");
    auto property = receiver->metaObject()->property(index);

    int current = 0;
    const auto count = container->fileNames().size();

    property.write(receiver, 0);

    for (const auto &fileName : container->fileNames()) {
        QImage(fileName).copy(rect).save(fileName);
        ++current;
        property.write(receiver, qRound(((double)current / (double)count) * 100.0));
    }

    property.write(receiver, 100);
}

void applyTextFunc(QPromise<void> &,
                   BusyIndicator *receiver,
                   QGifLib::Gif *container,
                   const QRect &rect,
                   const TextFrame::Documents &docs,
                   const QVector<qsizetype> &unchecked)
{
    const auto index = receiver->metaObject()->indexOfProperty("percent");
    auto property = receiver->metaObject()->property(index);

    int current = 0;
    const auto count = docs.size();
    const auto fileNames = container->fileNames();

    property.write(receiver, 0);

    for (const auto idx : docs.keys()) {
        if (!unchecked.contains(idx + 1)) {
            QImage img(fileNames.at(idx));
            QPainter p(&img);
            QTextDocument *doc = docs[idx]->clone();
            doc->setPageSize(rect.size().toSizeF());
            doc->setTextWidth(rect.width());
            p.translate(rect.topLeft());
            doc->drawContents(&p);
            doc->deleteLater();
            img.save(fileNames.at(idx));
        }

        ++current;
        property.write(receiver, qRound(((double)current / (double)count) * 100.0));
    }

    property.write(receiver, 100);
}

void applyRectFunc(QPromise<void> &,
                   BusyIndicator *receiver,
                   QGifLib::Gif *container,
                   const QRect &rect,
                   const QSet<qsizetype> &frames,
                   const QVector<qsizetype> &unchecked)
{
    const auto index = receiver->metaObject()->indexOfProperty("percent");
    auto property = receiver->metaObject()->property(index);

    int current = 0;
    const auto count = frames.size();
    const auto fileNames = container->fileNames();

    property.write(receiver, 0);

    for (const auto idx : std::as_const(frames)) {
        if (!unchecked.contains(idx + 1)) {
            QImage img(fileNames.at(idx));
            QPainter p(&img);
            RectFrame::drawRect(p, rect);
            img.save(fileNames.at(idx));
        }

        ++current;
        property.write(receiver, qRound(((double)current / (double)count) * 100.0));
    }

    property.write(receiver, 100);
}

void applyArrowFunc(QPromise<void> &,
                    BusyIndicator *receiver,
                    QGifLib::Gif *container,
                    const QRect &rect,
                    ArrowFrame::Orientation o,
                    const QSet<qsizetype> &frames,
                    const QVector<qsizetype> &unchecked)
{
    const auto index = receiver->metaObject()->indexOfProperty("percent");
    auto property = receiver->metaObject()->property(index);

    int current = 0;
    const auto count = frames.size();
    const auto fileNames = container->fileNames();

    property.write(receiver, 0);

    for (const auto idx : std::as_const(frames)) {
        if (!unchecked.contains(idx + 1)) {
            QImage img(fileNames.at(idx));
            QPainter p(&img);
            ArrowFrame::drawArrow(p, rect, o);
            img.save(fileNames.at(idx));
        }

        ++current;
        property.write(receiver, qRound(((double)current / (double)count) * 100.0));
    }

    property.write(receiver, 100);
}

} /* namespace anonymous */

//
// MainWindow
//

MainWindow::MainWindow()
    : m_d(new MainWindowPrivate(this))
{
    initUi();
}

MainWindow::~MainWindow() noexcept
{
}

void MainWindow::initUi()
{
    setWindowTitle(tr("GIF Editor"));

    auto file = menuBar()->addMenu(tr("&File"));
    m_d->m_open = file->addAction(
        QIcon::fromTheme(QStringLiteral("document-open"), QIcon(QStringLiteral(":/img/document-open.png"))),
        tr("Open"),
        tr("Ctrl+O"),
        this,
        &MainWindow::openGif);
    file->addSeparator();
    m_d->m_save = file->addAction(
        QIcon::fromTheme(QStringLiteral("document-save"), QIcon(QStringLiteral(":/img/document-save.png"))),
        tr("Save"),
        tr("Ctrl+S"),
        this,
        &MainWindow::saveGif);
    m_d->m_saveAs = file->addAction(
        QIcon::fromTheme(QStringLiteral("document-save-as"), QIcon(QStringLiteral(":/img/document-save-as.png"))),
        tr("Save As"),
        this,
        &MainWindow::saveGifAs);
    file->addSeparator();
    m_d->m_quit = file->addAction(
        QIcon::fromTheme(QStringLiteral("application-exit"), QIcon(QStringLiteral(":/img/application-exit.png"))),
        tr("Quit"),
        tr("Ctrl+Q"),
        this,
        &MainWindow::quit);

    m_d->m_crop = new QAction(
        QIcon::fromTheme(QStringLiteral("transform-crop"), QIcon(QStringLiteral(":/img/transform-crop.png"))),
        tr("Crop"),
        this);
    m_d->m_crop->setShortcut(tr("Ctrl+C"));
    m_d->m_crop->setShortcutContext(Qt::ApplicationShortcut);
    m_d->m_crop->setCheckable(true);
    m_d->m_crop->setChecked(false);

    m_d->m_insertText =
        new QAction(QIcon::fromTheme(QStringLiteral("insert-text"), QIcon(QStringLiteral(":/img/insert-text.png"))),
                    tr("Insert text"),
                    this);
    m_d->m_insertText->setShortcut(tr("Ctrl+T"));
    m_d->m_insertText->setShortcutContext(Qt::ApplicationShortcut);
    m_d->m_insertText->setCheckable(true);
    m_d->m_insertText->setChecked(false);

    m_d->m_drawRect = new QAction(
        QIcon::fromTheme(QStringLiteral("draw-rectangle"), QIcon(QStringLiteral(":/img/draw-rectangle.png"))),
        tr("Draw rectangle"),
        this);
    m_d->m_drawRect->setShortcut(tr("Ctrl+R"));
    m_d->m_drawRect->setShortcutContext(Qt::ApplicationShortcut);
    m_d->m_drawRect->setCheckable(true);
    m_d->m_drawRect->setChecked(false);

    m_d->m_drawArrow =
        new QAction(QIcon::fromTheme(QStringLiteral("draw-path"), QIcon(QStringLiteral(":/img/draw-path.png"))),
                    tr("Draw arrow"),
                    this);
    m_d->m_drawArrow->setShortcut(tr("Ctrl+A"));
    m_d->m_drawArrow->setShortcutContext(Qt::ApplicationShortcut);
    m_d->m_drawArrow->setCheckable(true);
    m_d->m_drawArrow->setChecked(false);

    auto actionsGroup = new QActionGroup(this);
    actionsGroup->addAction(m_d->m_crop);
    actionsGroup->addAction(m_d->m_insertText);
    actionsGroup->addAction(m_d->m_drawRect);
    actionsGroup->addAction(m_d->m_drawArrow);
    actionsGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);

    m_d->m_playStop = new QAction(QIcon::fromTheme(QStringLiteral("media-playback-start"),
                                                   QIcon(QStringLiteral(":/img/media-playback-start.png"))),
                                  tr("Play"),
                                  this);
    m_d->m_playStop->setShortcut(Qt::Key_Space);
    m_d->m_playStop->setShortcutContext(Qt::ApplicationShortcut);

    m_d->m_applyEdit = new QAction(this);
    m_d->m_applyEdit->setShortcut(Qt::Key_Return);
    m_d->m_applyEdit->setShortcutContext(Qt::ApplicationShortcut);

    m_d->m_cancelTips = new QAction(this);
    m_d->m_cancelTips->setShortcut(Qt::Key_Escape);
    m_d->m_cancelTips->setShortcutContext(Qt::ApplicationShortcut);
    m_d->m_cancelTips->setEnabled(false);

    m_d->m_cancelEdit = new QAction(this);
    m_d->m_cancelEdit->setShortcut(Qt::Key_Escape);
    m_d->m_cancelEdit->setShortcutContext(Qt::ApplicationShortcut);

    addAction(m_d->m_applyEdit);
    addAction(m_d->m_cancelEdit);
    addAction(m_d->m_cancelTips);

    m_d->m_playTimer = new QTimer(this);

    connect(m_d->m_crop, &QAction::toggled, this, &MainWindow::crop);
    connect(m_d->m_drawRect, &QAction::toggled, this, &MainWindow::drawRect);
    connect(m_d->m_drawArrow, &QAction::toggled, this, &MainWindow::drawArrow);
    connect(m_d->m_playStop, &QAction::triggered, this, &MainWindow::playStop);
    connect(m_d->m_applyEdit, &QAction::triggered, this, &MainWindow::applyEdit);
    connect(m_d->m_playTimer, &QTimer::timeout, this, &MainWindow::showNextFrame);
    connect(m_d->m_view, &View::applyEdit, this, &MainWindow::applyEdit);

    m_d->m_editMenu = menuBar()->addMenu(tr("&Edit"));
    m_d->m_editMenu->addAction(m_d->m_crop);
    m_d->m_editMenu->addAction(m_d->m_insertText);
    m_d->m_editMenu->addAction(m_d->m_drawRect);
    m_d->m_editMenu->addAction(m_d->m_drawArrow);

    m_d->m_editToolBar = new QToolBar(tr("Tools"), this);
    m_d->m_editToolBar->addAction(m_d->m_playStop);
    m_d->m_editToolBar->addSeparator();
    m_d->m_editToolBar->addAction(m_d->m_crop);
    m_d->m_editToolBar->addAction(m_d->m_insertText);
    m_d->m_editToolBar->addAction(m_d->m_drawRect);
    m_d->m_editToolBar->addAction(m_d->m_drawArrow);

    addToolBar(Qt::LeftToolBarArea, m_d->m_editToolBar);

    m_d->m_editToolBar->hide();

    m_d->m_textToolBar = new QToolBar(tr("Text"), this);

    m_d->m_boldText = new QAction(
        QIcon::fromTheme(QStringLiteral("format-text-bold"), QIcon(QStringLiteral(":/img/format-text-bold.png"))),
        tr("Bold text"),
        this);
    m_d->m_italicText = new QAction(
        QIcon::fromTheme(QStringLiteral("format-text-italic"), QIcon(QStringLiteral(":/img/format-text-italic.png"))),
        tr("Italic text"),
        this);
    m_d->m_fontLess = new QAction(QIcon::fromTheme(QStringLiteral("format-font-size-less"),
                                                   QIcon(QStringLiteral(":/img/format-font-size-less.png"))),
                                  tr("Less font size"),
                                  this);
    m_d->m_fontMore = new QAction(QIcon::fromTheme(QStringLiteral("format-font-size-more"),
                                                   QIcon(QStringLiteral(":/img/format-font-size-more.png"))),
                                  tr("More font size"),
                                  this);
    m_d->m_textColor = new QAction(
        QIcon::fromTheme(QStringLiteral("format-text-color"), QIcon(QStringLiteral(":/img/format-text-color.png"))),
        tr("Text color"),
        this);
    m_d->m_clearFormat =
        new QAction(QIcon::fromTheme(QStringLiteral("edit-clear"), QIcon(QStringLiteral(":/img/edit-clear.png"))),
                    tr("Clear format"),
                    this);
    m_d->m_finishText = new QAction(
        QIcon::fromTheme(QStringLiteral("dialog-ok-apply"), QIcon(QStringLiteral(":/img/dialog-ok-apply.png"))),
        tr("Finish text"),
        this);

    m_d->m_textToolBar->addAction(m_d->m_boldText);
    m_d->m_textToolBar->addAction(m_d->m_italicText);
    m_d->m_textToolBar->addAction(m_d->m_fontMore);
    m_d->m_textToolBar->addAction(m_d->m_fontLess);
    m_d->m_textToolBar->addAction(m_d->m_textColor);
    m_d->m_textToolBar->addAction(m_d->m_clearFormat);
    m_d->m_textToolBar->addAction(m_d->m_finishText);

    addToolBar(Qt::LeftToolBarArea, m_d->m_textToolBar);

    m_d->m_textToolBar->hide();

    m_d->m_drawToolBar = new QToolBar(tr("Drawing"), this);

    m_d->m_penColor = new QAction(
        QIcon::fromTheme(QStringLiteral("format-stroke-color"), QIcon(QStringLiteral(":/img/format-stroke-color.png"))),
        tr("Stroke color"),
        this);
    m_d->m_brushColor =
        new QAction(QIcon::fromTheme(QStringLiteral("fill-color"), QIcon(QStringLiteral(":/img/fill-color.png"))),
                    tr("Fill color"),
                    this);
    m_d->m_penWidth = new QAction(QIcon::fromTheme(QStringLiteral("distribute-horizontal-x"),
                                                   QIcon(QStringLiteral(":/img/distribute-horizontal-x.png"))),
                                  tr("Pen width"),
                                  this);
    m_d->m_penWidth->setCheckable(true);
    m_d->m_penWidthBtnOnDrawToolBar = new QToolButton(this);
    m_d->m_penWidthBtnOnDrawToolBar->setDefaultAction(m_d->m_penWidth);

    m_d->m_drawToolBar->addAction(m_d->m_penColor);
    m_d->m_drawToolBar->addAction(m_d->m_brushColor);
    m_d->m_drawToolBar->addWidget(m_d->m_penWidthBtnOnDrawToolBar);

    addToolBar(Qt::LeftToolBarArea, m_d->m_drawToolBar);

    m_d->m_drawToolBar->hide();

    m_d->m_drawArrowToolBar = new QToolBar(tr("Drawing"), this);
    m_d->m_penWidthBtnOnDrawArrowToolBar = new QToolButton(this);
    m_d->m_penWidthBtnOnDrawArrowToolBar->setDefaultAction(m_d->m_penWidth);

    m_d->m_drawArrowToolBar->addAction(m_d->m_penColor);
    m_d->m_drawArrowToolBar->addWidget(m_d->m_penWidthBtnOnDrawArrowToolBar);

    addToolBar(Qt::LeftToolBarArea, m_d->m_drawArrowToolBar);

    m_d->m_drawArrowToolBar->hide();

    auto settings = menuBar()->addMenu(tr("&Settings"));
    settings->addAction(QIcon::fromTheme(QStringLiteral("configure"), QIcon(QStringLiteral(":/img/configure.png"))),
                        tr("Settings"),
                        this,
                        &MainWindow::onSettings);

    auto help = menuBar()->addMenu(tr("&Help"));
    help->addAction(QIcon(QStringLiteral(":/icon/icon_22x22.png")), tr("About"), this, &MainWindow::about);
    help->addAction(QIcon(QStringLiteral(":/img/Qt-logo-neon-transparent.png")),
                    tr("About Qt"),
                    this,
                    &MainWindow::aboutQt);
    help->addAction(
        QIcon::fromTheme(QStringLiteral("bookmarks-organize"), QIcon(QStringLiteral(":/img/bookmarks-organize.png"))),
        tr("Licenses"),
        this,
        &MainWindow::licenses);
    m_d->m_tipsAction =
        help->addAction(QIcon::fromTheme(QStringLiteral("help-hint"), QIcon(QStringLiteral(":/img/help-hint.png"))),
                        tr("Tips && Tricks"));

    m_d->m_stack->addWidget(m_d->m_about);
    m_d->m_stack->addWidget(m_d->m_view);
    m_d->m_stack->addWidget(m_d->m_busyPage);
    m_d->m_stack->addWidget(m_d->m_tips);

    setCentralWidget(m_d->m_stack);

    connect(m_d->m_view->tape(), &Tape::checkStateChanged, this, &MainWindow::frameChecked);
    connect(m_d->m_view->tape(), &Tape::currentFrameChanged, this, &MainWindow::onFrameSelected);
    connect(m_d->m_view->tape(), &Tape::frameChanged, this, &MainWindow::onFrameChanged);
    connect(m_d->m_penWidth, &QAction::toggled, this, &MainWindow::penWidth);
    connect(m_d->m_penColor, &QAction::triggered, this, &MainWindow::penColor);
    connect(m_d->m_brushColor, &QAction::triggered, this, &MainWindow::brushColor);

    m_d->m_status = new QLabel(statusBar());
    statusBar()->addWidget(m_d->m_status);
    statusBar()->hide();

    initStateMachine();
}

void MainWindow::initStateMachine()
{
    m_d->m_uiState = new QStateMachine(this);
    auto rootState = new QState(QState::ParallelStates, m_d->m_uiState);
    m_d->m_uiState->setInitialState(rootState);

    auto viewState = new QState(rootState);
    auto aboutState = new AboutState(*m_d, viewState);
    auto busyState = new BusyState(*m_d, viewState);
    auto readyState = new ReadyState(*m_d, viewState);

    viewState->setInitialState(aboutState);

    {
        auto t1 = viewState->addTransition(this, &MainWindow::openFileTriggered, busyState);
        t1->setTransitionType(QAbstractTransition::InternalTransition);
        auto t2 = viewState->addTransition(this, &MainWindow::saveFileTriggered, busyState);
        t2->setTransitionType(QAbstractTransition::InternalTransition);

        auto t3 = viewState->addTransition(this, &MainWindow::fileLoadedTriggered, readyState);
        t3->setTransitionType(QAbstractTransition::InternalTransition);

        auto t4 = viewState->addTransition(this, &MainWindow::fileLoadingFailed, aboutState);
        t4->setTransitionType(QAbstractTransition::InternalTransition);
        auto t5 = viewState->addTransition(this, &MainWindow::fileSavingFailed, aboutState);
        t5->setTransitionType(QAbstractTransition::InternalTransition);

        auto t6 = viewState->addTransition(this, &MainWindow::applyEditTriggered, busyState);
        t6->setTransitionType(QAbstractTransition::InternalTransition);

        auto t7 = viewState->addTransition(this, &MainWindow::graphicsAppliedTriggered, readyState);
        t7->setTransitionType(QAbstractTransition::InternalTransition);
    }

    auto editingState = new QState(rootState);
    auto idleEditing = new QState(editingState);
    editingState->setInitialState(idleEditing);

    idleEditing->assignProperty(m_d->m_insertText, "checked", false);
    idleEditing->assignProperty(m_d->m_drawArrow, "checked", false);
    idleEditing->assignProperty(m_d->m_drawRect, "checked", false);
    idleEditing->assignProperty(m_d->m_crop, "checked", false);

    idleEditing->assignProperty(m_d->m_insertText, "enabled", true);
    idleEditing->assignProperty(m_d->m_drawArrow, "enabled", true);
    idleEditing->assignProperty(m_d->m_drawRect, "enabled", true);
    idleEditing->assignProperty(m_d->m_crop, "enabled", true);

    auto textState = new DrawTextState(*m_d, editingState);

    {
        auto t1 = editingState->addTransition(m_d->m_insertText, &QAction::triggered, textState);
        t1->setTransitionType(QAbstractTransition::InternalTransition);

        auto t2 = editingState->addTransition(m_d->m_cancelEdit, &QAction::triggered, idleEditing);
        t2->setTransitionType(QAbstractTransition::InternalTransition);

        auto t3 = editingState->addTransition(this, &MainWindow::cancelEditTriggered, idleEditing);
        t3->setTransitionType(QAbstractTransition::InternalTransition);

        auto t4 = editingState->addTransition(this, &MainWindow::graphicsAppliedTriggered, idleEditing);
        t4->setTransitionType(QAbstractTransition::InternalTransition);
    }

    auto tipsModeState = new QState(rootState);
    auto tipsIdleState = new QState(tipsModeState);
    tipsModeState->setInitialState(tipsIdleState);
    auto tipsState = new TipsState(*m_d, tipsModeState);

    {
        auto t1 = tipsModeState->addTransition(m_d->m_tipsAction, &QAction::triggered, tipsState);
        t1->setTransitionType(QAbstractTransition::InternalTransition);

        auto t2 = tipsModeState->addTransition(m_d->m_cancelTips, &QAction::triggered, tipsIdleState);
        t2->setTransitionType(QAbstractTransition::InternalTransition);
    }

    m_d->m_uiState->start();
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    Settings::instance().setAppWinMaximized(isMaximized());
    Settings::instance().setAppWinRect(QRect(windowHandle()->x(), windowHandle()->y(), width(), height()));

    if (m_d->m_busyFlag) {
        const auto btn = QMessageBox::question(this,
                                               tr("GIF editor is busy..."),
                                               tr("GIF editor is busy.\nDo you want to terminate the application?"));

        if (btn == QMessageBox::Yes) {
            std::quick_exit(0);
        } else {
            e->ignore();
        }
    } else {
        e->accept();
    }

    quit();
}

void MainWindow::showEvent(QShowEvent *e)
{
    if (!m_d->m_shownAlready) {
        m_d->m_shownAlready = true;

        const auto r = Settings::instance().appWinRect();

        if (r.width() != -1) {
            resize(r.width(), r.height());

            windowHandle()->setX(r.x());
            windowHandle()->setY(r.y());
        }

        if (Settings::instance().isAppWinMaximized()) {
            showMaximized();
        }

        if (!m_d->m_fileNameToOpenAfterShow.isEmpty()) {
            QTimer::singleShot(0, [this]() {
                this->openFile(this->m_d->m_fileNameToOpenAfterShow);
            });
        }
    }

    e->accept();
}

void MainWindow::openFile(const QString &fileName,
                          bool afterShowEvent)
{
    if (afterShowEvent && !m_d->m_shownAlready) {
        m_d->m_fileNameToOpenAfterShow = fileName;

        return;
    }

    if (!fileName.isEmpty() && QFileInfo(fileName).suffix().toLower() == QStringLiteral("gif")) {
        if (isWindowModified()) {
            const auto btn = QMessageBox::question(this,
                                                   tr("GIF was changed..."),
                                                   tr("\"%1\" was changed.\n"
                                                      "Do you want to save it?")
                                                       .arg(fileName));

            if (btn == QMessageBox::Yes) {
                saveGif();
            }
        }

        m_d->openGif(fileName);
    }
}

void MainWindow::openGif()
{
    static const auto pictureLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);

    const auto fileName =
        QFileDialog::getOpenFileName(this,
                                     tr("Open GIF..."),
                                     (!pictureLocations.isEmpty() ? pictureLocations.first() : QString()),
                                     tr("GIF (*.gif)"));

    openFile(fileName);
}

void MainWindow::saveGif()
{
    try {
        emit saveFileTriggered();

        QStringList toSave;
        QVector<int> delays;
        const auto allFiles = m_d->m_frames.fileNames();

        for (int i = 0; i < m_d->m_view->tape()->count(); ++i) {
            if (m_d->m_view->tape()->frame(i + 1)->isChecked()) {
                toSave.push_back(allFiles.at(i));
                delays.push_back(m_d->m_frames.delay(i));
            }
        }

        if (!toSave.empty()) {
            m_d->m_busy->setShowPercent(true);
            m_d->m_busyStatusLabel->setText(tr("Saving GIF..."));

            connect(&m_d->m_watcher, &QFutureWatcher<void>::finished, this, qOverload<>(&MainWindow::gifSaved));
            auto future = QtConcurrent::run(writeGIFFunc, m_d->m_busy, toSave, delays, m_d->m_currentGif);
            m_d->m_watcher.setFuture(future);
        } else {
            emit fileSavingFailed();

            QMessageBox::information(this, tr("Can't save GIF..."), tr("Can't save GIF image with no frames."));
        }
    } catch (const std::bad_alloc &) {
        emit fileSavingFailed();

        QMessageBox::critical(this, tr("Failed to save GIF..."), tr("Out of memory."));
    }
}

void MainWindow::saveGifAs()
{
    auto fileName = QFileDialog::getSaveFileName(this, tr("Choose file to save to..."), QString(), tr("GIF (*.gif)"));

    if (!fileName.isEmpty()) {
        if (!fileName.endsWith(QStringLiteral(".gif"), Qt::CaseInsensitive)) {
            fileName.append(QStringLiteral(".gif"));
        }

        m_d->m_currentGif = fileName;

        QFileInfo info(fileName);

        setWindowTitle(tr("GIF Editor - %1[*]").arg(info.fileName()));

        saveGif();
    }
}

void MainWindow::quit()
{
    if (!m_d->m_busyFlag && !m_d->m_quitFlag) {
        auto delayQuit = false;

        if (isWindowModified()) {
            auto btn = QMessageBox::question(this,
                                             tr("GIF was changed..."),
                                             tr("GIF was changed. Do you want to save changes?"));

            if (btn == QMessageBox::Yes) {
                if (m_d->m_playing) {
                    playStop();
                }

                saveGif();

                delayQuit = true;
            }
        }

        m_d->m_quitFlag = true;

        if (!delayQuit) {
            QApplication::quit();
        }
    }
}

void MainWindow::frameChecked(int,
                              bool)
{
    m_d->setModified(true);
}

void MainWindow::crop(bool on)
{
    if (on) {
        hidePenWidthSpinBox();

        m_d->enableActionsOnEdit(false);

        m_d->m_editMode = MainWindowPrivate::EditMode::Crop;

        m_d->m_view->startCrop();

        connect(m_d->m_view->cropFrame(), &CropFrame::started, this, &MainWindow::onRectSelectionStarted);
    } else {
        m_d->m_view->stopCrop();

        m_d->m_editMode = MainWindowPrivate::EditMode::Unknow;

        m_d->enableActionsOnEdit();
    }
}

void MainWindow::penWidth(bool on)
{
    if (on) {
        if (!m_d->m_penWidthBox) {
            m_d->m_penWidthBox = new QSpinBox(this);
            m_d->m_penWidthBox->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
            m_d->m_penWidthBox->setMinimum(1);
            m_d->m_penWidthBox->setMaximum(10);

            connect(m_d->m_penWidthBox, &QSpinBox::valueChanged, [this](int v) {
                Settings::instance().setPenWidth(v);
                emit this->m_d->m_view->doRepaint();
            });
        }

        m_d->m_penWidthBox->setValue(Settings::instance().penWidth());

        if (m_d->m_penWidthBtnOnDrawToolBar->isVisible()) {
            m_d->m_penWidthBox->move(m_d->m_drawToolBar->mapToGlobal(
                QPoint(m_d->m_penWidthBtnOnDrawToolBar->x() + m_d->m_penWidthBtnOnDrawToolBar->width() + 1,
                       m_d->m_penWidthBtnOnDrawToolBar->y())));
        } else {
            m_d->m_penWidthBox->move(m_d->m_drawArrowToolBar->mapToGlobal(
                QPoint(m_d->m_penWidthBtnOnDrawArrowToolBar->x() + m_d->m_penWidthBtnOnDrawArrowToolBar->width() + 1,
                       m_d->m_penWidthBtnOnDrawArrowToolBar->y())));
        }

        m_d->m_penWidthBox->show();
    } else {
        if (m_d->m_penWidthBox) {
            m_d->m_penWidthBox->hide();
            m_d->m_penWidthBox->deleteLater();
            m_d->m_penWidthBox = nullptr;
        }
    }
}

void MainWindow::drawRect(bool on)
{
    if (on) {
        hidePenWidthSpinBox();

        m_d->enableActionsOnEdit(false);

        m_d->m_editMode = MainWindowPrivate::EditMode::Rect;

        m_d->m_view->startRect();

        m_d->m_drawToolBar->show();

        connect(m_d->m_view->rectFrame(), &RectFrame::started, this, &MainWindow::onRectSelectionStarted);
        connect(m_d->m_view->rectFrame(), &RectFrame::clicked, this, &MainWindow::hidePenWidthSpinBox);
    } else {
        m_d->m_view->stopRect();

        m_d->m_drawToolBar->hide();

        m_d->m_editMode = MainWindowPrivate::EditMode::Unknow;

        m_d->enableActionsOnEdit();
    }
}

void MainWindow::drawArrow(bool on)
{
    if (on) {
        hidePenWidthSpinBox();

        m_d->enableActionsOnEdit(false);

        m_d->m_editMode = MainWindowPrivate::EditMode::Arrow;

        m_d->m_view->startArrow();

        m_d->m_drawArrowToolBar->show();

        connect(m_d->m_view->arrowFrame(), &ArrowFrame::started, this, &MainWindow::onRectSelectionStarted);
        connect(m_d->m_view->arrowFrame(), &ArrowFrame::clicked, this, &MainWindow::hidePenWidthSpinBox);
    } else {
        m_d->m_view->stopArrow();

        m_d->m_drawArrowToolBar->hide();

        m_d->m_editMode = MainWindowPrivate::EditMode::Unknow;

        m_d->enableActionsOnEdit();
    }
}

void MainWindow::cancelEdit()
{
    // m_d->m_view->stopCrop();
    // m_d->m_view->stopText();
    // m_d->m_crop->setEnabled(true);
    // m_d->m_insertText->setEnabled(true);
    // m_d->m_drawRect->setEnabled(true);
    // m_d->m_drawArrow->setEnabled(true);

    // hidePenWidthSpinBox();

    // m_d->enableActionsOnEdit();

    // switch (m_d->m_editMode) {
    // case MainWindowPrivate::EditMode::Crop: {
    //     m_d->m_crop->setChecked(false);
    // } break;

    // case MainWindowPrivate::EditMode::Text: {
    //     m_d->m_textToolBar->hide();
    //     m_d->m_insertText->setChecked(false);
    // } break;

    // case MainWindowPrivate::EditMode::Rect: {
    //     m_d->m_drawToolBar->hide();
    //     m_d->m_drawRect->setChecked(false);
    // } break;

    // case MainWindowPrivate::EditMode::Arrow: {
    //     m_d->m_drawArrowToolBar->hide();
    //     m_d->m_drawArrow->setChecked(false);
    // } break;

    // default:
    //     break;
    // }

    // for (int i = 1; i <= m_d->m_view->tape()->count(); ++i) {
    //     m_d->m_view->tape()->frame(i)->setModified(false);
    // }

    // m_d->m_editMode = MainWindowPrivate::EditMode::Unknow;
}

void MainWindow::applyEdit()
{
    m_d->m_unchecked.clear();

    hidePenWidthSpinBox();

    switch (m_d->m_editMode) {
    case MainWindowPrivate::EditMode::Crop: {
        const auto rect = m_d->m_view->selectedRect();

        if (!rect.isNull() && rect != m_d->m_view->currentFrame()->imageRect()) {
            emit applyEditTriggered();

            m_d->m_busyStatusLabel->setText(tr("Cropping GIF..."));

            for (int i = 1; i <= m_d->m_view->tape()->count(); ++i) {
                if (!m_d->m_view->tape()->frame(i)->isChecked()) {
                    m_d->m_unchecked.append(i);
                }
            }

            m_d->m_busy->setShowPercent(true);

            connect(&m_d->m_watcher, &QFutureWatcher<void>::finished, this, &MainWindow::gifCropped);
            auto future = QtConcurrent::run(cropGIFFunc, m_d->m_busy, &m_d->m_frames, rect);
            m_d->m_watcher.setFuture(future);
        } else {
            emit cancelEditTriggered();
        }
    } break;

    case MainWindowPrivate::EditMode::Text: {
        const auto rect = m_d->m_view->selectedRect();

        if (!rect.isNull()) {
            m_d->m_view->startTextEditing();
        } else {
            emit cancelEditTriggered();
        }
    } break;

    case MainWindowPrivate::EditMode::Rect:
    case MainWindowPrivate::EditMode::Arrow: {
        const auto rect = m_d->m_view->selectedRect();

        if (!rect.isNull()) {
            emit applyEditTriggered();

            m_d->m_busy->setShowPercent(true);

            for (qsizetype i = 1; i <= m_d->m_view->tape()->count(); ++i) {
                if (!m_d->m_view->tape()->frame(i)->isChecked()) {
                    m_d->m_unchecked.append(i);
                }
            }

            connect(&m_d->m_watcher, &QFutureWatcher<void>::finished, this, &MainWindow::graphicsApplied);

            switch (m_d->m_editMode) {
            case MainWindowPrivate::EditMode::Rect: {
                m_d->m_busyStatusLabel->setText(tr("Drawing rectangle..."));

                auto future = QtConcurrent::run(applyRectFunc,
                                                m_d->m_busy,
                                                &m_d->m_frames,
                                                rect,
                                                m_d->m_view->rectFrame()->frames(),
                                                m_d->m_unchecked);
                m_d->m_watcher.setFuture(future);
            } break;

            case MainWindowPrivate::EditMode::Arrow: {
                m_d->m_busyStatusLabel->setText(tr("Drawing arrow..."));

                auto future = QtConcurrent::run(applyArrowFunc,
                                                m_d->m_busy,
                                                &m_d->m_frames,
                                                rect,
                                                m_d->m_view->arrowFrame()->orientation(),
                                                m_d->m_view->arrowFrame()->frames(),
                                                m_d->m_unchecked);
                m_d->m_watcher.setFuture(future);
            } break;

            default: {
                break;
            }
            }
        } else {
            emit cancelEditTriggered();
        }
    } break;

    default:
        break;
    }
}

void MainWindow::about()
{
    QMessageBox dlg(QMessageBox::Information,
                    tr("About GIF editor"),
                    tr("GIF editor.<br /><br />"
                       "Version: %1<br /><br />"
                       "Author - Igor Mironchik (<a href=\"mailto:igor.mironchik@gmail.com\">"
                       "igor.mironchik at gmail.com</a>).<br /><br />"
                       "Copyright (c) 2026 Igor Mironchik.<br /><br />"
                       "Licensed under GNU GPL 3.0.")
                        .arg(c_version),
                    QMessageBox::NoButton,
                    this);
    QIcon icon = dlg.windowIcon();
    dlg.setIconPixmap(icon.pixmap(QSize(64, 64), dlg.devicePixelRatio()));
    dlg.setTextFormat(Qt::RichText);

    dlg.exec();
}

void MainWindow::aboutQt()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::licenses()
{
    LicenseDialog msg(this);
    msg.addLicense(s_oxygenName, s_oxygenLicense);
    msg.addLicense(s_giflibName, s_giflibLicense);
    msg.addLicense(s_qgiflibName, s_qgiflibLicense);

    msg.exec();
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    static bool tapeHeightInit = false;

    if (!tapeHeightInit && m_d->m_stack->currentWidget() != m_d->m_view) {
        tapeHeightInit = true;

        m_d->m_view->resize(800, 600);

        QApplication::processEvents();

        m_d->m_view->tape()->setMinimumHeight(m_d->m_view->tape()->height());
    }

    e->accept();
}

void MainWindow::playStop()
{
    if (m_d->m_playing) {
        m_d->m_playTimer->stop();
        m_d->m_playStop->setText(tr("Play"));
        m_d->m_playStop->setIcon(
            QIcon::fromTheme(QStringLiteral("media-playback-start"), QIcon(":/img/media-playback-start.png")));
        m_d->enableActions();
    } else {
        m_d->disableActionsOnPlaying();
        m_d->m_playStop->setText(tr("Stop"));
        m_d->m_playStop->setIcon(
            QIcon::fromTheme(QStringLiteral("media-playback-stop"), QIcon(":/img/media-playback-stop.png")));

        const auto &img = m_d->m_view->tape()->frame(m_d->m_view->tape()->currentFrame()->counter())->image();
        m_d->m_playTimer->start(m_d->m_frames.delay(img.m_pos));
    }

    m_d->m_playing = !m_d->m_playing;
}

void MainWindow::showNextFrame()
{
    const auto next = m_d->nextCheckedFrame(m_d->m_view->tape()->currentFrame()->counter());

    if (next != -1) {
        const auto &img = m_d->m_view->tape()->frame(next)->image();
        m_d->m_playTimer->start(m_d->m_frames.delay(img.m_pos));

        m_d->m_view->tape()->setCurrentFrame(next);
        m_d->m_view->scrollTo(next);
    }
}

void MainWindow::onSwitchToTextEditMode()
{
    m_d->m_textToolBar->show();
}

void MainWindow::onSwitchToTextSelectionRectMode()
{
    m_d->m_textToolBar->hide();
}

void MainWindow::onRectSelectionStarted()
{
    if (m_d->m_crop->isChecked()) {
        m_d->m_insertText->setEnabled(false);
        m_d->m_drawRect->setEnabled(false);
        m_d->m_drawArrow->setEnabled(false);
    } else if (m_d->m_insertText->isChecked()) {
        m_d->m_crop->setEnabled(false);
        m_d->m_drawRect->setEnabled(false);
        m_d->m_drawArrow->setEnabled(false);
    } else if (m_d->m_drawRect->isChecked()) {
        m_d->m_crop->setEnabled(false);
        m_d->m_insertText->setEnabled(false);
        m_d->m_drawArrow->setEnabled(false);
    } else if (m_d->m_drawArrow->isChecked()) {
        m_d->m_crop->setEnabled(false);
        m_d->m_insertText->setEnabled(false);
        m_d->m_drawRect->setEnabled(false);
    }
}

void MainWindow::applyText()
{
    m_d->m_unchecked.clear();

    hidePenWidthSpinBox();

    const auto rect = m_d->m_view->selectedRect();

    if (!rect.isNull()) {
        emit applyEditTriggered();

        m_d->m_busyStatusLabel->setText(tr("Drawing text..."));

        m_d->m_busy->setShowPercent(true);

        for (qsizetype i = 1; i <= m_d->m_view->tape()->count(); ++i) {
            if (!m_d->m_view->tape()->frame(i)->isChecked()) {
                m_d->m_unchecked.append(i);
            }
        }

        connect(&m_d->m_watcher, &QFutureWatcher<void>::finished, this, &MainWindow::graphicsApplied);
        auto future = QtConcurrent::run(applyTextFunc,
                                        m_d->m_busy,
                                        &m_d->m_frames,
                                        rect,
                                        m_d->m_view->textFrame()->text(),
                                        m_d->m_unchecked);
        m_d->m_watcher.setFuture(future);
    } else {
        emit cancelEditTriggered();
    }
}

void MainWindow::onSettings()
{
    SettingsDlg dlg(this);

    dlg.exec();
}

void MainWindow::hidePenWidthSpinBox()
{
    QPointF scenePos(-1, -1);

    if (m_d->m_penWidthBtnOnDrawToolBar->isChecked()) {
        m_d->m_penWidthBtnOnDrawToolBar->setChecked(false);

        QHoverEvent ev(QEvent::HoverLeave,
                       scenePos,
                       m_d->m_penWidthBtnOnDrawToolBar->mapToGlobal(scenePos.toPoint()),
                       {0, 0});
        QApplication::sendEvent(m_d->m_penWidthBtnOnDrawToolBar, &ev);
    }

    if (m_d->m_penWidthBtnOnDrawArrowToolBar->isChecked()) {
        m_d->m_penWidthBtnOnDrawArrowToolBar->setChecked(false);

        QHoverEvent ev(QEvent::HoverLeave,
                       scenePos,
                       m_d->m_penWidthBtnOnDrawArrowToolBar->mapToGlobal(scenePos.toPoint()),
                       {0, 0});
        QApplication::sendEvent(m_d->m_penWidthBtnOnDrawArrowToolBar, &ev);
    }
}

void MainWindow::penColor()
{
    hidePenWidthSpinBox();

    QColorDialog dlg(Settings::instance().penColor(), this);

    if (dlg.exec() == QDialog::Accepted) {
        Settings::instance().setPenColor(dlg.currentColor());

        emit m_d->m_view->doRepaint();
    }
}

void MainWindow::brushColor()
{
    hidePenWidthSpinBox();

    QColorDialog dlg(Settings::instance().brushColor(), this);
    dlg.setOption(QColorDialog::ShowAlphaChannel, true);

    if (dlg.exec() == QDialog::Accepted) {
        Settings::instance().setBrushColor(dlg.currentColor());

        emit m_d->m_view->doRepaint();
    }
}

void MainWindow::gifLoaded()
{
    disconnect(&m_d->m_readWatcher, 0, this, 0);

    if (m_d->m_readWatcher.result()) {
        QFileInfo info(m_d->m_currentGif);

        setWindowTitle(MainWindow::tr("GIF Editor - %1[*]").arg(info.fileName()));

        m_d->m_busyStatusLabel->setText(tr("Preparing tape..."));

        QApplication::processEvents();

        m_d->initTape();

        m_d->calculateTimings();

        if (m_d->m_frames.count()) {
            m_d->m_view->tape()->setCurrentFrame(1);
            m_d->m_view->scrollTo(1);
        }

        emit fileLoadedTriggered();
    } else {
        emit fileLoadingFailed();

        QMessageBox::critical(this,
                              tr("Unable to load GIF..."),
                              tr("Unable to load GIF file. File \"%1\" is corrupted.").arg(m_d->m_currentGif));

        m_d->m_currentGif.clear();
    }
}

void MainWindow::gifSaved()
{
    disconnect(&m_d->m_watcher, 0, this, 0);

    m_d->m_busy->setShowPercent(false);

    if (!m_d->m_quitFlag) {
        m_d->openGif(m_d->m_currentGif);
    } else {
        QApplication::quit();
    }
}

void MainWindow::gifCropped()
{
    disconnect(&m_d->m_watcher, 0, this, 0);

    m_d->m_busy->setShowPercent(false);

    m_d->m_busyStatusLabel->setText(tr("Preparing tape..."));

    QApplication::processEvents();

    const auto current = m_d->m_view->tape()->currentFrame()->counter();
    m_d->m_view->tape()->clear();

    m_d->initTape();

    m_d->m_view->tape()->setCurrentFrame(current);

    for (const auto &i : std::as_const(m_d->m_unchecked)) {
        m_d->m_view->tape()->frame(i)->setChecked(false);
    }

    m_d->setModified(true);

    cancelEdit();

    m_d->ready();
}

void MainWindow::graphicsApplied()
{
    disconnect(&m_d->m_watcher, 0, this, 0);

    m_d->m_busy->setShowPercent(false);

    m_d->m_busyStatusLabel->setText(tr("Preparing tape..."));

    QApplication::processEvents();

    const auto current = m_d->m_view->tape()->currentFrame()->counter();
    m_d->m_view->tape()->clear();

    m_d->initTape();

    m_d->m_view->tape()->setCurrentFrame(current);

    m_d->setModified(true);

    emit graphicsAppliedTriggered();
}

void MainWindow::onFrameSelected(int idx)
{
    if (idx) {
        m_d->m_status->setText(
            tr("<b>Time:</b> %1 <b>Total Duration:</b> %2")
                .arg(QTime::fromMSecsSinceStartOfDay(m_d->m_timings[idx - 1]).toString(QStringLiteral("hh:mm:ss.zzz")),
                     m_d->m_totalDuration));
    }
}

void MainWindow::onFrameChanged(int)
{
    m_d->setModified(true);
    m_d->calculateTimings();

    onFrameSelected(m_d->m_view->currentFrame()->image().m_pos + 1);
}
