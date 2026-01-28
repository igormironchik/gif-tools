/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF editor include.
#include "mainwindow.hpp"
#include "about.hpp"
#include "busyindicator.hpp"
#include "crop.hpp"
#include "drawarrow.hpp"
#include "drawrect.hpp"
#include "frame.hpp"
#include "frameontape.hpp"
#include "settings.hpp"
#include "tape.hpp"
#include "text.hpp"
#include "view.hpp"

// Qt include.
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCloseEvent>
#include <QColorDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenuBar>
#include <QMessageBox>
#include <QMetaMethod>
#include <QPainter>
#include <QResizeEvent>
#include <QRunnable>
#include <QSpinBox>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QTextDocument>
#include <QThreadPool>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QVector>
#include <QWindow>

// C++ include.
#include <algorithm>
#include <utility>
#include <vector>

// gif-widgets include.
#include "license_dialog.hpp"
#include "utils.hpp"

namespace /* anonymous */
{

class ReadGIF final : public QRunnable
{
public:
    ReadGIF(QGifLib::Gif *container,
            const QString &fileName)
        : m_container(container)
        , m_fileName(fileName)
    {
        setAutoDelete(false);
    }

    void run() override
    {
        m_container->load(m_fileName);
    }

private:
    QGifLib::Gif *m_container;
    QString m_fileName;
}; // class ReadGIF

class CropGIF final : public QRunnable
{
public:
    CropGIF(BusyIndicator *receiver,
            QGifLib::Gif *container,
            const QRect &rect)
        : m_container(container)
        , m_rect(rect)
        , m_receiver(receiver)
    {
        setAutoDelete(false);
    }

    void run() override
    {
        const auto index = m_receiver->metaObject()->indexOfProperty("percent");
        auto property = m_receiver->metaObject()->property(index);

        int current = 0;
        const auto count = m_container->fileNames().size();

        property.write(m_receiver, 0);

        for (const auto &fileName : m_container->fileNames()) {
            QImage(fileName).copy(m_rect).save(fileName);
            ++current;
            property.write(m_receiver, qRound(((double)current / (double)count) * 100.0));
        }

        property.write(m_receiver, 100);
    }

private:
    QGifLib::Gif *m_container;
    QRect m_rect;
    BusyIndicator *m_receiver;
}; // class CropGIF

class ApplyText final : public QRunnable
{
public:
    ApplyText(BusyIndicator *receiver,
              QGifLib::Gif *container,
              const QRect &rect,
              const TextFrame::Documents &docs,
              const QVector<qsizetype> &unchecked)
        : m_container(container)
        , m_rect(rect)
        , m_receiver(receiver)
        , m_docs(docs)
        , m_unchecked(unchecked)
    {
        setAutoDelete(false);
    }

    void run() override
    {
        const auto index = m_receiver->metaObject()->indexOfProperty("percent");
        auto property = m_receiver->metaObject()->property(index);

        int current = 0;
        const auto count = m_docs.size();
        const auto fileNames = m_container->fileNames();

        property.write(m_receiver, 0);

        for (const auto idx : m_docs.keys()) {
            if (!m_unchecked.contains(idx + 1)) {
                QImage img(fileNames.at(idx));
                QPainter p(&img);
                QTextDocument *doc = m_docs[idx]->clone();
                doc->setPageSize(m_rect.size().toSizeF());
                doc->setTextWidth(m_rect.width());
                p.translate(m_rect.topLeft());
                doc->drawContents(&p);
                doc->deleteLater();
                img.save(fileNames.at(idx));
            }

            ++current;
            property.write(m_receiver, qRound(((double)current / (double)count) * 100.0));
        }

        property.write(m_receiver, 100);
    }

private:
    QGifLib::Gif *m_container;
    QRect m_rect;
    BusyIndicator *m_receiver;
    const TextFrame::Documents &m_docs;
    const QVector<qsizetype> &m_unchecked;
}; // class ApplyText

class ApplyRect final : public QRunnable
{
public:
    ApplyRect(BusyIndicator *receiver,
              QGifLib::Gif *container,
              const QRect &rect,
              const QSet<qsizetype> &frames,
              const QVector<qsizetype> &unchecked)
        : m_container(container)
        , m_rect(rect)
        , m_receiver(receiver)
        , m_frames(frames)
        , m_unchecked(unchecked)
    {
        setAutoDelete(false);
    }

    void run() override
    {
        const auto index = m_receiver->metaObject()->indexOfProperty("percent");
        auto property = m_receiver->metaObject()->property(index);

        int current = 0;
        const auto count = m_frames.size();
        const auto fileNames = m_container->fileNames();

        property.write(m_receiver, 0);

        for (const auto idx : std::as_const(m_frames)) {
            if (!m_unchecked.contains(idx + 1)) {
                QImage img(fileNames.at(idx));
                QPainter p(&img);
                RectFrame::drawRect(p, m_rect);
                img.save(fileNames.at(idx));
            }

            ++current;
            property.write(m_receiver, qRound(((double)current / (double)count) * 100.0));
        }

        property.write(m_receiver, 100);
    }

private:
    QGifLib::Gif *m_container;
    QRect m_rect;
    BusyIndicator *m_receiver;
    const QSet<qsizetype> &m_frames;
    const QVector<qsizetype> &m_unchecked;
}; // class ApplyRect

class ApplyArrow final : public QRunnable
{
public:
    ApplyArrow(BusyIndicator *receiver,
               QGifLib::Gif *container,
               const QRect &rect,
               ArrowFrame::Orientation o,
               const QSet<qsizetype> &frames,
               const QVector<qsizetype> &unchecked)
        : m_container(container)
        , m_rect(rect)
        , m_receiver(receiver)
        , m_frames(frames)
        , m_unchecked(unchecked)
        , m_orientation(o)
    {
        setAutoDelete(false);
    }

    void run() override
    {
        const auto index = m_receiver->metaObject()->indexOfProperty("percent");
        auto property = m_receiver->metaObject()->property(index);

        int current = 0;
        const auto count = m_frames.size();
        const auto fileNames = m_container->fileNames();

        property.write(m_receiver, 0);

        for (const auto idx : std::as_const(m_frames)) {
            if (!m_unchecked.contains(idx + 1)) {
                QImage img(fileNames.at(idx));
                QPainter p(&img);
                ArrowFrame::drawArrow(p, m_rect, m_orientation);
                img.save(fileNames.at(idx));
            }

            ++current;
            property.write(m_receiver, qRound(((double)current / (double)count) * 100.0));
        }

        property.write(m_receiver, 100);
    }

private:
    QGifLib::Gif *m_container;
    QRect m_rect;
    BusyIndicator *m_receiver;
    const QSet<qsizetype> &m_frames;
    const QVector<qsizetype> &m_unchecked;
    ArrowFrame::Orientation m_orientation;
}; // class ApplyRect

} /* namespace anonymous */

//
// MainWindowPrivate
//

class MainWindowPrivate
{
public:
    MainWindowPrivate(MainWindow *parent)
        : m_editMode(EditMode::Unknow)
        , m_busyFlag(false)
        , m_quitFlag(false)
        , m_playing(false)
        , m_stack(new QStackedWidget(parent))
        , m_busy(new BusyIndicator(m_stack))
        , m_view(new View(m_frames,
                          m_stack))
        , m_about(new About(parent))
        , m_crop(nullptr)
        , m_insertText(nullptr)
        , m_drawRect(nullptr)
        , m_drawArrow(nullptr)
        , m_playStop(nullptr)
        , m_save(nullptr)
        , m_saveAs(nullptr)
        , m_open(nullptr)
        , m_applyEdit(nullptr)
        , m_cancelEdit(nullptr)
        , m_quit(nullptr)
        , m_boldText(nullptr)
        , m_italicText(nullptr)
        , m_fontLess(nullptr)
        , m_fontMore(nullptr)
        , m_textColor(nullptr)
        , m_clearFormat(nullptr)
        , m_finishText(nullptr)
        , m_penColor(nullptr)
        , m_brushColor(nullptr)
        , m_penWidth(nullptr)
        , m_editToolBar(nullptr)
        , m_textToolBar(nullptr)
        , m_drawToolBar(nullptr)
        , m_drawArrowToolBar(nullptr)
        , m_penWidthBox(nullptr)
        , m_penWidthBtnOnDrawToolBar(nullptr)
        , m_penWidthBtnOnDrawArrowToolBar(nullptr)
        , m_q(parent)
    {
        m_busy->setRadius(75);
    }

    //! Edit mode.
    enum class EditMode {
        Unknow,
        Crop,
        Text,
        Rect,
        Arrow
    }; // enum class EditMode

    //! Clear view.
    void clearView();
    //! Enable file actions.
    void enableFileActions(bool on = true)
    {
        m_save->setEnabled(on);
        m_saveAs->setEnabled(on);
        m_open->setEnabled(on);

        m_applyEdit->setEnabled(!on);
        m_cancelEdit->setEnabled(!on);

        m_playStop->setEnabled(on);
    }
    //! Initialize tape.
    void initTape()
    {
        for (qsizetype i = 0, last = m_frames.count(); i < last; ++i) {
            m_view->tape()->addFrame({m_frames, i, false});

            QApplication::processEvents();
        };
    }
    //! Busy state.
    void busy()
    {
        m_busyFlag = true;

        m_stack->setCurrentWidget(m_busy);

        m_busy->setRunning(true);

        m_crop->setEnabled(false);
        m_insertText->setEnabled(false);
        m_drawRect->setEnabled(false);
        m_drawArrow->setEnabled(false);
        m_save->setEnabled(false);
        m_saveAs->setEnabled(false);
        m_open->setEnabled(false);
        m_quit->setEnabled(false);

        m_editToolBar->hide();
    }
    //! Ready state.
    void ready()
    {
        m_busyFlag = false;

        m_stack->setCurrentWidget(m_view);

        m_busy->setRunning(false);

        m_crop->setEnabled(true);
        m_insertText->setEnabled(true);
        m_drawRect->setEnabled(true);
        m_drawArrow->setEnabled(true);

        if (!m_currentGif.isEmpty()) {
            if (m_q->isWindowModified()) {
                m_save->setEnabled(true);
            }

            m_saveAs->setEnabled(true);
        }

        m_open->setEnabled(true);
        m_quit->setEnabled(true);

        m_editToolBar->show();
    }
    //! Wait for thread pool.
    void waitThreadPool()
    {
        while (!QThreadPool::globalInstance()->waitForDone(5)) {
            QApplication::processEvents();
        }
    }
    //! Set modified state.
    void setModified(bool on)
    {
        m_q->setWindowModified(on);

        if (on) {
            m_save->setEnabled(true);
        } else {
            m_save->setEnabled(false);
        }
    }

    //! \return Index of the next checked frame.
    int nextCheckedFrame(int current) const
    {
        for (int i = current + 1; i <= m_view->tape()->count(); ++i) {
            if (m_view->tape()->frame(i)->isChecked()) {
                return i;
            }
        }

        for (int i = 1; i < current; ++i) {
            if (m_view->tape()->frame(i)->isChecked()) {
                return i;
            }
        }

        return -1;
    }

    void openGif(const QString &fileName)
    {
        clearView();

        setModified(false);

        m_currentGif = fileName;

        ReadGIF read(&m_frames, fileName);
        QThreadPool::globalInstance()->start(&read);

        waitThreadPool();

        QFileInfo info(fileName);

        m_q->setWindowTitle(MainWindow::tr("GIF Editor - %1[*]").arg(info.fileName()));

        initTape();

        if (m_frames.count()) {
            m_view->tape()->setCurrentFrame(1);
            m_view->scrollTo(1);
        }

        m_crop->setEnabled(true);
        m_insertText->setEnabled(true);
        m_drawRect->setEnabled(true);
        m_drawArrow->setEnabled(true);
        m_playStop->setEnabled(true);
        m_saveAs->setEnabled(true);
    }

    //! Current file name.
    QString m_currentGif;
    //! Frames.
    QGifLib::Gif m_frames;
    //! Edit mode.
    EditMode m_editMode;
    //! Busy flag.
    bool m_busyFlag;
    //! Quit flag.
    bool m_quitFlag;
    //! Play/stop flag.
    bool m_playing;
    //! Was show evemt?
    bool m_shownAlready = false;
    //! File name to open after show event.
    QString m_fileNameToOpenAfterShow;
    //! Stacked widget.
    QStackedWidget *m_stack;
    //! Busy indicator.
    BusyIndicator *m_busy;
    //! View.
    View *m_view;
    //! Widget about.
    About *m_about;
    //! Crop action.
    QAction *m_crop;
    //! Insert text action.
    QAction *m_insertText;
    //! Draw rect.
    QAction *m_drawRect;
    //! Draw arrow.
    QAction *m_drawArrow;
    //! Play/stop action.
    QAction *m_playStop;
    //! Save action.
    QAction *m_save;
    //! Save as action.
    QAction *m_saveAs;
    //! Open action.
    QAction *m_open;
    //! Apply edit action.
    QAction *m_applyEdit;
    //! Cancel edit action.
    QAction *m_cancelEdit;
    //! Quit action.
    QAction *m_quit;
    //! Bold text action.
    QAction *m_boldText;
    //! Italic text.
    QAction *m_italicText;
    //! Font less.
    QAction *m_fontLess;
    //! Font more.
    QAction *m_fontMore;
    //! Text color.
    QAction *m_textColor;
    //! Clear text format.
    QAction *m_clearFormat;
    //! Show previous.
    QAction *m_finishText;
    //! Pen color.
    QAction *m_penColor;
    //! Brush color.
    QAction *m_brushColor;
    //! Pen width.
    QAction *m_penWidth;
    //! Edit toolbar.
    QToolBar *m_editToolBar;
    //! Text toolbar.
    QToolBar *m_textToolBar;
    //! Draw toolbar.
    QToolBar *m_drawToolBar;
    //! Draw arror toolbar.
    QToolBar *m_drawArrowToolBar;
    //! Play timer.
    QTimer *m_playTimer;
    //! Pen width box.
    QSpinBox *m_penWidthBox;
    //! Pen width tool button on draw tool bar.
    QToolButton *m_penWidthBtnOnDrawToolBar;
    //! Pen width tool button on draw arrow tool bar.
    QToolButton *m_penWidthBtnOnDrawArrowToolBar;
    //! Parent.
    MainWindow *m_q;
}; // class MainWindowPrivate

void MainWindowPrivate::clearView()
{
    m_view->currentFrame()->clearImage();
    m_view->tape()->clear();
    m_frames.clean();
}

//
// MainWindow
//

MainWindow::MainWindow()
    : m_d(new MainWindowPrivate(this))
{
    setWindowTitle(tr("GIF Editor"));

    auto file = menuBar()->addMenu(tr("&File"));
    m_d->m_open = file->addAction(QIcon(QStringLiteral(":/img/document-open.png")),
                                  tr("Open"),
                                  tr("Ctrl+O"),
                                  this,
                                  &MainWindow::openGif);
    file->addSeparator();
    m_d->m_save = file->addAction(QIcon(QStringLiteral(":/img/document-save.png")),
                                  tr("Save"),
                                  tr("Ctrl+S"),
                                  this,
                                  &MainWindow::saveGif);
    m_d->m_saveAs = file->addAction(QIcon(QStringLiteral(":/img/document-save-as.png")),
                                    tr("Save As"),
                                    this,
                                    &MainWindow::saveGifAs);
    file->addSeparator();
    m_d->m_quit = file->addAction(QIcon(QStringLiteral(":/img/application-exit.png")),
                                  tr("Quit"),
                                  tr("Ctrl+Q"),
                                  this,
                                  &MainWindow::quit);

    m_d->m_save->setEnabled(false);
    m_d->m_saveAs->setEnabled(false);

    m_d->m_crop = new QAction(QIcon(QStringLiteral(":/img/transform-crop.png")), tr("Crop"), this);
    m_d->m_crop->setShortcut(tr("Ctrl+C"));
    m_d->m_crop->setShortcutContext(Qt::ApplicationShortcut);
    m_d->m_crop->setCheckable(true);
    m_d->m_crop->setChecked(false);
    m_d->m_crop->setEnabled(false);

    m_d->m_insertText = new QAction(QIcon(QStringLiteral(":/img/insert-text.png")), tr("Insert text"), this);
    m_d->m_insertText->setShortcut(tr("Ctrl+T"));
    m_d->m_insertText->setShortcutContext(Qt::ApplicationShortcut);
    m_d->m_insertText->setCheckable(true);
    m_d->m_insertText->setChecked(false);
    m_d->m_insertText->setEnabled(false);

    m_d->m_drawRect = new QAction(QIcon(QStringLiteral(":/img/draw-rectangle.png")), tr("Draw rectangle"), this);
    m_d->m_drawRect->setShortcut(tr("Ctrl+R"));
    m_d->m_drawRect->setShortcutContext(Qt::ApplicationShortcut);
    m_d->m_drawRect->setCheckable(true);
    m_d->m_drawRect->setChecked(false);
    m_d->m_drawRect->setEnabled(false);

    m_d->m_drawArrow = new QAction(QIcon(QStringLiteral(":/img/draw-path.png")), tr("Draw arrow"), this);
    m_d->m_drawArrow->setShortcut(tr("Ctrl+A"));
    m_d->m_drawArrow->setShortcutContext(Qt::ApplicationShortcut);
    m_d->m_drawArrow->setCheckable(true);
    m_d->m_drawArrow->setChecked(false);
    m_d->m_drawArrow->setEnabled(false);

    auto actionsGroup = new QActionGroup(this);
    actionsGroup->addAction(m_d->m_crop);
    actionsGroup->addAction(m_d->m_insertText);
    actionsGroup->addAction(m_d->m_drawRect);
    actionsGroup->addAction(m_d->m_drawArrow);
    actionsGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);

    m_d->m_playStop = new QAction(QIcon(QStringLiteral(":/img/media-playback-start.png")), tr("Play"), this);
    m_d->m_playStop->setEnabled(false);

    m_d->m_applyEdit = new QAction(this);
    m_d->m_applyEdit->setShortcut(Qt::Key_Return);
    m_d->m_applyEdit->setShortcutContext(Qt::ApplicationShortcut);
    m_d->m_applyEdit->setEnabled(false);

    m_d->m_cancelEdit = new QAction(this);
    m_d->m_cancelEdit->setShortcut(Qt::Key_Escape);
    m_d->m_cancelEdit->setShortcutContext(Qt::ApplicationShortcut);
    m_d->m_cancelEdit->setEnabled(false);

    addAction(m_d->m_applyEdit);
    addAction(m_d->m_cancelEdit);

    m_d->m_playTimer = new QTimer(this);

    connect(m_d->m_crop, &QAction::toggled, this, &MainWindow::crop);
    connect(m_d->m_insertText, &QAction::toggled, this, &MainWindow::insertText);
    connect(m_d->m_drawRect, &QAction::toggled, this, &MainWindow::drawRect);
    connect(m_d->m_drawArrow, &QAction::toggled, this, &MainWindow::drawArrow);
    connect(m_d->m_playStop, &QAction::triggered, this, &MainWindow::playStop);
    connect(m_d->m_applyEdit, &QAction::triggered, this, &MainWindow::applyEdit);
    connect(m_d->m_cancelEdit, &QAction::triggered, this, &MainWindow::cancelEdit);
    connect(m_d->m_playTimer, &QTimer::timeout, this, &MainWindow::showNextFrame);
    connect(m_d->m_view, &View::applyEdit, this, &MainWindow::applyEdit);

    auto edit = menuBar()->addMenu(tr("&Edit"));
    edit->addAction(m_d->m_crop);
    edit->addAction(m_d->m_insertText);
    edit->addAction(m_d->m_drawRect);
    edit->addAction(m_d->m_drawArrow);

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

    m_d->m_boldText = new QAction(QIcon(QStringLiteral(":/img/format-text-bold.png")), tr("Bold text"), this);
    m_d->m_italicText = new QAction(QIcon(QStringLiteral(":/img/format-text-italic.png")), tr("Italic text"), this);
    m_d->m_fontLess = new QAction(QIcon(QStringLiteral(":/img/format-font-size-less.png")), tr("Less font size"), this);
    m_d->m_fontMore = new QAction(QIcon(QStringLiteral(":/img/format-font-size-more.png")), tr("More font size"), this);
    m_d->m_textColor = new QAction(QIcon(QStringLiteral(":/img/format-text-color.png")), tr("Text color"), this);
    m_d->m_clearFormat = new QAction(QIcon(QStringLiteral(":/img/edit-clear.png")), tr("Clear format"), this);
    m_d->m_finishText = new QAction(QIcon(QStringLiteral(":/img/dialog-ok-apply.png")), tr("Finish text"), this);

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

    m_d->m_penColor = new QAction(QIcon(QStringLiteral(":/img/format-stroke-color.png")), tr("Stroke color"), this);
    m_d->m_brushColor = new QAction(QIcon(QStringLiteral(":/img/fill-color.png")), tr("Fill color"), this);
    m_d->m_penWidth = new QAction(QIcon(QStringLiteral(":/img/distribute-horizontal-x.png")), tr("Pen width"), this);
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
    settings->addAction(QIcon(QStringLiteral(":/img/configure.png")), tr("Settings"), this, &MainWindow::onSettings);

    auto help = menuBar()->addMenu(tr("&Help"));
    help->addAction(QIcon(QStringLiteral(":/icon/icon_22x22.png")), tr("About"), this, &MainWindow::about);
    help->addAction(QIcon(QStringLiteral(":/img/Qt-logo-neon-transparent.png")),
                    tr("About Qt"),
                    this,
                    &MainWindow::aboutQt);
    help->addAction(QIcon(QStringLiteral(":/img/bookmarks-organize.png")), tr("Licenses"), this, &MainWindow::licenses);

    m_d->m_stack->addWidget(m_d->m_about);
    m_d->m_stack->addWidget(m_d->m_view);
    m_d->m_stack->addWidget(m_d->m_busy);

    setCentralWidget(m_d->m_stack);

    connect(m_d->m_view->tape(), &Tape::checkStateChanged, this, &MainWindow::frameChecked);
    connect(m_d->m_penWidth, &QAction::toggled, this, &MainWindow::penWidth);
    connect(m_d->m_penColor, &QAction::triggered, this, &MainWindow::penColor);
    connect(m_d->m_brushColor, &QAction::triggered, this, &MainWindow::brushColor);
}

MainWindow::~MainWindow() noexcept
{
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
            exit(-1);
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

        m_d->busy();

        m_d->openGif(fileName);

        m_d->ready();
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

namespace /* anonymous */
{

class WriteGIF final : public QRunnable
{
public:
    WriteGIF(BusyIndicator *receiver,
             const QStringList &files,
             const QVector<int> &delays,
             const QString &fileName)
        : m_files(files)
        , m_delays(delays)
        , m_fileName(fileName)
        , m_receiver(receiver)
    {
        setAutoDelete(false);
    }

    void run() override
    {
        QGifLib::Gif gif;

        QObject::connect(&gif, &QGifLib::Gif::writeProgress, m_receiver, &BusyIndicator::setPercent);

        gif.write(m_fileName, m_files, m_delays, 0);
    }

private:
    const QStringList &m_files;
    const QVector<int> &m_delays;
    QString m_fileName;
    BusyIndicator *m_receiver;
}; // class WriteGIF

} /* namespace anonymous */

void MainWindow::saveGif()
{
    try {
        m_d->busy();

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

            WriteGIF runnable(m_d->m_busy, toSave, delays, m_d->m_currentGif);
            QThreadPool::globalInstance()->start(&runnable);

            m_d->waitThreadPool();

            m_d->m_busy->setShowPercent(false);

            m_d->openGif(m_d->m_currentGif);
        } else {
            QMessageBox::information(this, tr("Can't save GIF..."), tr("Can't save GIF image with no frames."));
        }

        m_d->ready();
    } catch (const std::bad_alloc &) {
        m_d->ready();

        QMessageBox::critical(this, tr("Failed to save GIF..."), tr("Out of memory."));
    }
}

void MainWindow::saveGifAs()
{
    auto fileName = QFileDialog::getSaveFileName(this, tr("Choose file to save to..."), QString(), tr("GIF (*.gif)"));

    if (!fileName.isEmpty()) {
        if (!fileName.endsWith(QStringLiteral(".gif"), Qt::CaseInsensitive))
            fileName.append(QStringLiteral(".gif"));

        m_d->m_currentGif = fileName;

        QFileInfo info(fileName);

        setWindowTitle(tr("GIF Editor - %1[*]").arg(info.fileName()));

        saveGif();
    }
}

void MainWindow::quit()
{
    if (!m_d->m_busyFlag && !m_d->m_quitFlag) {
        if (isWindowModified()) {
            auto btn = QMessageBox::question(this,
                                             tr("GIF was changed..."),
                                             tr("GIF was changed. Do you want to save changes?"));

            if (btn == QMessageBox::Yes) {
                saveGif();
            }
        }

        m_d->m_quitFlag = true;

        QApplication::quit();
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

        m_d->enableFileActions(false);

        m_d->m_editMode = MainWindowPrivate::EditMode::Crop;

        m_d->m_view->startCrop();

        connect(m_d->m_view->cropFrame(), &CropFrame::started, this, &MainWindow::onRectSelectionStarted);
    } else {
        m_d->m_view->stopCrop();

        m_d->m_editMode = MainWindowPrivate::EditMode::Unknow;

        m_d->enableFileActions();
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
                QPoint(m_d->m_penWidthBtnOnDrawToolBar->x() + m_d->m_penWidthBtnOnDrawToolBar->width(),
                       m_d->m_penWidthBtnOnDrawToolBar->y() + 1)));
        } else {
            m_d->m_penWidthBox->move(m_d->m_drawArrowToolBar->mapToGlobal(
                QPoint(m_d->m_penWidthBtnOnDrawArrowToolBar->x() + m_d->m_penWidthBtnOnDrawArrowToolBar->width(),
                       m_d->m_penWidthBtnOnDrawArrowToolBar->y() + 1)));
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

void MainWindow::insertText(bool on)
{
    if (on) {
        hidePenWidthSpinBox();

        m_d->enableFileActions(false);

        m_d->m_editMode = MainWindowPrivate::EditMode::Text;

        m_d->m_view->startText();

        connect(m_d->m_view->textFrame(),
                &TextFrame::switchToTextEditingMode,
                this,
                &MainWindow::onSwitchToTextEditMode);
        connect(m_d->m_view->textFrame(),
                &TextFrame::switchToTextSelectionRectMode,
                this,
                &MainWindow::onSwitchToTextSelectionRectMode);
        connect(m_d->m_boldText, &QAction::triggered, m_d->m_view->textFrame(), &TextFrame::boldText);
        connect(m_d->m_italicText, &QAction::triggered, m_d->m_view->textFrame(), &TextFrame::italicText);
        connect(m_d->m_fontLess, &QAction::triggered, m_d->m_view->textFrame(), &TextFrame::fontLess);
        connect(m_d->m_fontMore, &QAction::triggered, m_d->m_view->textFrame(), &TextFrame::fontMore);
        connect(m_d->m_textColor, &QAction::triggered, m_d->m_view->textFrame(), &TextFrame::textColor);
        connect(m_d->m_clearFormat, &QAction::triggered, m_d->m_view->textFrame(), &TextFrame::clearFormat);
        connect(m_d->m_finishText, &QAction::triggered, this, &MainWindow::applyText);
        connect(m_d->m_view->textFrame(), &TextFrame::started, this, &MainWindow::onRectSelectionStarted);
    } else {
        m_d->m_view->stopText();

        m_d->m_editMode = MainWindowPrivate::EditMode::Unknow;

        m_d->enableFileActions();
    }
}

void MainWindow::drawRect(bool on)
{
    if (on) {
        hidePenWidthSpinBox();

        m_d->enableFileActions(false);

        m_d->m_editMode = MainWindowPrivate::EditMode::Rect;

        m_d->m_view->startRect();

        m_d->m_drawToolBar->show();

        connect(m_d->m_view->rectFrame(), &RectFrame::started, this, &MainWindow::onRectSelectionStarted);
        connect(m_d->m_view->rectFrame(), &RectFrame::clicked, this, &MainWindow::hidePenWidthSpinBox);
    } else {
        m_d->m_view->stopRect();

        m_d->m_drawToolBar->hide();

        m_d->m_editMode = MainWindowPrivate::EditMode::Unknow;

        m_d->enableFileActions();
    }
}

void MainWindow::drawArrow(bool on)
{
    if (on) {
        hidePenWidthSpinBox();

        m_d->enableFileActions(false);

        m_d->m_editMode = MainWindowPrivate::EditMode::Arrow;

        m_d->m_view->startArrow();

        m_d->m_drawArrowToolBar->show();

        connect(m_d->m_view->arrowFrame(), &ArrowFrame::started, this, &MainWindow::onRectSelectionStarted);
        connect(m_d->m_view->arrowFrame(), &ArrowFrame::clicked, this, &MainWindow::hidePenWidthSpinBox);
    } else {
        m_d->m_view->stopArrow();

        m_d->m_drawArrowToolBar->hide();

        m_d->m_editMode = MainWindowPrivate::EditMode::Unknow;

        m_d->enableFileActions();
    }
}

void MainWindow::cancelEdit()
{
    m_d->m_view->stopCrop();
    m_d->m_view->stopText();
    m_d->m_crop->setEnabled(true);
    m_d->m_insertText->setEnabled(true);
    m_d->m_drawRect->setEnabled(true);
    m_d->m_drawArrow->setEnabled(true);

    hidePenWidthSpinBox();

    m_d->enableFileActions();

    switch (m_d->m_editMode) {
    case MainWindowPrivate::EditMode::Crop: {
        m_d->m_crop->setChecked(false);
    } break;

    case MainWindowPrivate::EditMode::Text: {
        m_d->m_textToolBar->hide();
        m_d->m_insertText->setChecked(false);
    } break;

    case MainWindowPrivate::EditMode::Rect: {
        m_d->m_drawToolBar->hide();
        m_d->m_drawRect->setChecked(false);
    } break;

    case MainWindowPrivate::EditMode::Arrow: {
        m_d->m_drawArrowToolBar->hide();
        m_d->m_drawArrow->setChecked(false);
    } break;

    default:
        break;
    }

    for (int i = 1; i <= m_d->m_view->tape()->count(); ++i) {
        m_d->m_view->tape()->frame(i)->setModified(false);
    }

    m_d->m_editMode = MainWindowPrivate::EditMode::Unknow;

    QApplication::processEvents();
}

void MainWindow::applyEdit()
{
    switch (m_d->m_editMode) {
    case MainWindowPrivate::EditMode::Crop: {
        const auto rect = m_d->m_view->selectedRect();

        if (!rect.isNull() && rect != m_d->m_view->currentFrame()->imageRect()) {
            m_d->busy();

            QVector<int> unchecked;

            for (int i = 1; i <= m_d->m_view->tape()->count(); ++i) {
                if (!m_d->m_view->tape()->frame(i)->isChecked()) {
                    unchecked.append(i);
                }
            }

            QApplication::processEvents();

            m_d->m_busy->setShowPercent(true);

            CropGIF crop(m_d->m_busy, &m_d->m_frames, rect);
            QThreadPool::globalInstance()->start(&crop);

            m_d->waitThreadPool();

            m_d->m_busy->setShowPercent(false);

            const auto current = m_d->m_view->tape()->currentFrame()->counter();
            m_d->m_view->tape()->clear();

            QApplication::processEvents();

            m_d->initTape();

            m_d->m_view->tape()->setCurrentFrame(current);

            for (const auto &i : std::as_const(unchecked)) {
                m_d->m_view->tape()->frame(i)->setChecked(false);
            }

            m_d->setModified(true);

            cancelEdit();

            m_d->ready();
        } else {
            cancelEdit();
        }
    } break;

    case MainWindowPrivate::EditMode::Text: {
        const auto rect = m_d->m_view->selectedRect();

        if (!rect.isNull()) {
            m_d->m_view->startTextEditing();
        } else {
            cancelEdit();
        }
    } break;

    case MainWindowPrivate::EditMode::Rect:
    case MainWindowPrivate::EditMode::Arrow: {
        const auto rect = m_d->m_view->selectedRect();

        if (!rect.isNull()) {
            m_d->busy();

            QApplication::processEvents();

            m_d->m_busy->setShowPercent(true);

            QVector<qsizetype> unchecked;

            for (qsizetype i = 1; i <= m_d->m_view->tape()->count(); ++i) {
                if (!m_d->m_view->tape()->frame(i)->isChecked()) {
                    unchecked.append(i);
                }
            }

            switch (m_d->m_editMode) {
            case MainWindowPrivate::EditMode::Rect: {
                ApplyRect apply(m_d->m_busy, &m_d->m_frames, rect, m_d->m_view->rectFrame()->frames(), unchecked);
                QThreadPool::globalInstance()->start(&apply);
            } break;

            case MainWindowPrivate::EditMode::Arrow: {
                ApplyArrow apply(m_d->m_busy,
                                 &m_d->m_frames,
                                 rect,
                                 m_d->m_view->arrowFrame()->orientation(),
                                 m_d->m_view->arrowFrame()->frames(),
                                 unchecked);
                QThreadPool::globalInstance()->start(&apply);
            } break;

            default: {
                break;
            }
            }

            m_d->waitThreadPool();

            m_d->m_busy->setShowPercent(false);

            const auto current = m_d->m_view->tape()->currentFrame()->counter();
            m_d->m_view->tape()->clear();

            QApplication::processEvents();

            m_d->initTape();

            m_d->m_view->tape()->setCurrentFrame(current);

            m_d->setModified(true);

            cancelEdit();

            m_d->ready();
        } else {
            cancelEdit();
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
                       "Author - Igor Mironchik (<a href=\"mailto:igor.mironchik@gmail.com\">"
                       "igor.mironchik at gmail.com</a>).<br /><br />"
                       "Copyright (c) 2026 Igor Mironchik.<br /><br />"
                       "Licensed under GNU GPL 3.0."),
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
        m_d->m_playStop->setIcon(QIcon(":/img/media-playback-start.png"));
    } else {
        m_d->m_playStop->setText(tr("Stop"));
        m_d->m_playStop->setIcon(QIcon(":/img/media-playback-stop.png"));
        const auto &img = m_d->m_view->tape()->currentFrame()->image();
        m_d->m_playTimer->start(static_cast<int>(img.m_gif.delay(img.m_pos)));
    }

    m_d->m_playing = !m_d->m_playing;
}

void MainWindow::showNextFrame()
{
    const auto next = m_d->nextCheckedFrame(m_d->m_view->tape()->currentFrame()->counter());

    if (next != -1) {
        const auto nextDelay = m_d->nextCheckedFrame(next);

        if (nextDelay != -1) {
            const auto &img = m_d->m_view->tape()->frame(nextDelay)->image();
            m_d->m_playTimer->start(m_d->m_frames.delay(img.m_pos));
        }

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
    const auto rect = m_d->m_view->selectedRect();

    if (!rect.isNull()) {
        m_d->busy();

        QApplication::processEvents();

        m_d->m_busy->setShowPercent(true);

        QVector<qsizetype> unchecked;

        for (qsizetype i = 1; i <= m_d->m_view->tape()->count(); ++i) {
            if (!m_d->m_view->tape()->frame(i)->isChecked()) {
                unchecked.append(i);
            }
        }

        ApplyText apply(m_d->m_busy, &m_d->m_frames, rect, m_d->m_view->textFrame()->text(), unchecked);
        QThreadPool::globalInstance()->start(&apply);

        m_d->waitThreadPool();

        m_d->m_busy->setShowPercent(false);

        const auto current = m_d->m_view->tape()->currentFrame()->counter();
        m_d->m_view->tape()->clear();

        QApplication::processEvents();

        m_d->initTape();

        m_d->m_view->tape()->setCurrentFrame(current);

        m_d->setModified(true);

        cancelEdit();

        m_d->ready();
    } else {
        cancelEdit();
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
