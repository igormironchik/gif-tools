/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF editor include.
#include "mainwindow_private.hpp"
#include "frameontape.hpp"
#include "mainwindow.hpp"
#include "tape.hpp"

// Qt include.
#include <QApplication>
#include <QMenu>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QtConcurrent>

namespace /* anonymous */
{

bool readGIFFunc(QGifLib::Gif *container,
                 const QString &fileName)
{
    return container->load(fileName);
}

} /* namespace anonymous */

//
// MainWindowPrivate
//

MainWindowPrivate::MainWindowPrivate(MainWindow *parent)
    : m_frames(QDir::tempPath() + QDir::separator() + QStringLiteral("gif-editor"))
    , m_editMode(EditMode::Unknow)
    , m_busyFlag(false)
    , m_quitFlag(false)
    , m_playing(false)
    , m_stack(new QStackedWidget(parent))
    , m_busyPage(new QWidget(m_stack))
    , m_busy(new BusyIndicator(m_busyPage))
    , m_busyStatusLabel(new QLabel(m_busyPage))
    , m_view(new View(m_frames,
                      m_stack))
    , m_about(new About(parent))
    , m_tips(new Tips(parent))
    , m_q(parent)
{
    m_busy->setRadius(75);
    auto f = m_busyStatusLabel->font();
    f.setPixelSize(35);
    m_busyStatusLabel->setFont(f);
    auto p = m_busyStatusLabel->palette();
    p.setColor(QPalette::WindowText, p.color(QPalette::Highlight));
    m_busyStatusLabel->setPalette(p);

    auto l = new QVBoxLayout(m_busyPage);
    l->addItem(new QSpacerItem(10, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));
    l->addWidget(m_busy);
    auto h = new QHBoxLayout;
    h->addItem(new QSpacerItem(0, 10, QSizePolicy::Expanding, QSizePolicy::Fixed));
    h->addWidget(m_busyStatusLabel);
    h->addItem(new QSpacerItem(0, 10, QSizePolicy::Expanding, QSizePolicy::Fixed));
    l->addLayout(h);
    l->addItem(new QSpacerItem(10, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));
}

void MainWindowPrivate::clearView()
{
    m_view->currentFrame()->clearImage();
    m_view->tape()->clear();
    m_frames.clean();
}

void MainWindowPrivate::enableActionsOnEdit(bool on)
{
    m_save->setEnabled(on && m_q->isWindowModified());
    m_saveAs->setEnabled(on);
    m_open->setEnabled(on);

    m_applyEdit->setEnabled(!on);
    m_cancelEdit->setEnabled(!on);

    m_playStop->setEnabled(on);
}

void MainWindowPrivate::initTape()
{
    for (qsizetype i = 0, last = m_frames.count(); i < last; ++i) {
        m_view->tape()->addFrame({m_frames, i, false});

        QApplication::processEvents();
    };
}

void MainWindowPrivate::setSaveAction()
{
    if (m_q->isWindowModified()) {
        m_save->setEnabled(true);
    } else {
        m_save->setEnabled(false);
    }
}

void MainWindowPrivate::enableActions()
{
    m_crop->setEnabled(true);
    m_insertText->setEnabled(true);
    m_drawRect->setEnabled(true);
    m_drawArrow->setEnabled(true);
    m_open->setEnabled(true);
    m_playStop->setEnabled(true);
    m_quit->setEnabled(true);

    if (!m_currentGif.isEmpty()) {
        setSaveAction();

        m_saveAs->setEnabled(true);
    }
}

void MainWindowPrivate::disableActionsOnPlaying()
{
    m_crop->setEnabled(false);
    m_insertText->setEnabled(false);
    m_drawRect->setEnabled(false);
    m_drawArrow->setEnabled(false);
    m_save->setEnabled(false);
    m_saveAs->setEnabled(false);
    m_open->setEnabled(false);
}

void MainWindowPrivate::busy()
{
    m_q->statusBar()->hide();

    m_busyFlag = true;

    m_stack->setCurrentWidget(m_busyPage);

    m_busy->setRunning(true);

    disableActionsOnPlaying();

    cancelTips(false);

    m_quit->setEnabled(false);

    m_editToolBar->hide();
    m_textToolBar->hide();
    m_drawToolBar->hide();
    m_drawArrowToolBar->hide();
    m_tipsAction->setEnabled(false);
}

void MainWindowPrivate::cancelTips(bool restoreWidget)
{
    if (m_currentUiState.m_currentStackWidget) {
        m_cancelTips->setEnabled(false);
        m_cancelEdit->setEnabled(m_currentUiState.m_isEditActionsEnabled);
        m_applyEdit->setEnabled(m_currentUiState.m_isEditActionsEnabled);
        m_editMenu->setEnabled(m_currentUiState.m_currentStackWidget != m_about);

        if (restoreWidget) {
            m_stack->setCurrentWidget(m_currentUiState.m_currentStackWidget);
        }

        m_drawArrowToolBar->setVisible(m_currentUiState.m_isDrawArrowToolBarShow);
        m_drawToolBar->setVisible(m_currentUiState.m_isDrawToolBarShow);
        m_editToolBar->setVisible(m_currentUiState.m_isEditToolBarShown);
        m_textToolBar->setVisible(m_currentUiState.m_isTextToolBarShown);

        m_currentUiState.m_currentStackWidget = nullptr;
    }
}

void MainWindowPrivate::ready()
{
    m_q->statusBar()->show();

    m_busyFlag = false;

    m_busyStatusLabel->clear();

    m_stack->setCurrentWidget(m_view);

    m_busy->setRunning(false);

    enableActions();
    m_tipsAction->setEnabled(true);

    m_editToolBar->show();
}

void MainWindowPrivate::setModified(bool on)
{
    m_q->setWindowModified(on);

    setSaveAction();
}

int MainWindowPrivate::nextCheckedFrame(int current) const
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

void MainWindowPrivate::openGif(const QString &fileName)
{
    clearView();

    setModified(false);

    m_currentGif = fileName;
    m_busyStatusLabel->setText(MainWindow::tr("Loading GIF..."));

    m_q->connect(&m_readWatcher, &QFutureWatcher<bool>::finished, m_q, &MainWindow::gifLoaded);
    auto future = QtConcurrent::run(readGIFFunc, &m_frames, fileName);
    m_readWatcher.setFuture(future);
}

void MainWindowPrivate::calculateTimings()
{
    m_timings.clear();
    m_timings.push_back(0);

    int ms = 0;
    int total = 0;

    for (qsizetype i = 0; i < m_frames.count(); ++i) {
        ms += m_frames.delay(i);

        if (i != m_frames.count() - 1) {
            total = ms;
        }

        m_timings.push_back(ms);
    }

    m_totalDuration = QTime::fromMSecsSinceStartOfDay(total).toString(QStringLiteral("hh:mm:ss.zzz"));
}

void MainWindowPrivate::setActionsToInitialState()
{
    m_save->setEnabled(false);
    m_saveAs->setEnabled(false);
    m_crop->setEnabled(false);
    m_insertText->setEnabled(false);
    m_drawRect->setEnabled(false);
    m_drawArrow->setEnabled(false);
    m_playStop->setEnabled(false);
    m_applyEdit->setEnabled(false);
    m_cancelEdit->setEnabled(false);

    m_open->setEnabled(true);
    m_quit->setEnabled(true);
    m_tipsAction->setEnabled(true);

    m_editMenu->setEnabled(false);
}