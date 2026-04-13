/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF editor include.
#include "crop.hpp"
#include "drawarrow.hpp"
#include "drawrect.hpp"
#include "frameontape.hpp"
#include "mainwindow.hpp"
#include "mainwindow_private.hpp"
#include "tape.hpp"
#include "text.hpp"
#include "uistates.hpp"

// Qt include.
#include <QMenu>
#include <QSignalTransition>
#include <QStatusBar>
#include <QTimer>

//
// TipsState
//

TipsState::TipsState(MainWindowPrivate &impl, QState *parent)
    : QState(parent)
    , m_impl(impl)
{
}

void TipsState::onEntry(QEvent *)
{
    m_isEditActionsEnabled = m_impl.m_cancelEdit->isEnabled();
    m_isDrawArrowToolBarShow = m_impl.m_drawArrowToolBar->isVisible();
    m_isDrawToolBarShow = m_impl.m_drawToolBar->isVisible();
    m_isEditToolBarShown = m_impl.m_editToolBar->isVisible();
    m_isTextToolBarShown = m_impl.m_textToolBar->isVisible();
    m_currentStackWidget = m_impl.m_stack->currentWidget();

    m_impl.m_q->hidePenWidthSpinBox();

    m_impl.m_cancelEdit->setEnabled(false);
    m_impl.m_applyEdit->setEnabled(false);
    m_impl.m_editMenu->setEnabled(false);
    m_impl.m_cancelTips->setEnabled(true);

    m_impl.m_drawArrowToolBar->hide();
    m_impl.m_drawToolBar->hide();
    m_impl.m_editToolBar->hide();
    m_impl.m_textToolBar->hide();

    m_impl.m_tipsAction->setEnabled(false);

    m_impl.m_stack->setCurrentWidget(m_impl.m_tips);
}

void TipsState::onExit(QEvent *)
{
    m_impl.m_cancelTips->setEnabled(false);
    m_impl.m_cancelEdit->setEnabled(m_isEditActionsEnabled);
    m_impl.m_applyEdit->setEnabled(m_isEditActionsEnabled);
    m_impl.m_editMenu->setEnabled(m_currentStackWidget != m_impl.m_about);

    m_impl.m_stack->setCurrentWidget(m_currentStackWidget);

    m_impl.m_tipsAction->setEnabled(true);

    m_impl.m_drawArrowToolBar->setVisible(m_isDrawArrowToolBarShow);
    m_impl.m_drawToolBar->setVisible(m_isDrawToolBarShow);
    m_impl.m_editToolBar->setVisible(m_isEditToolBarShown);
    m_impl.m_textToolBar->setVisible(m_isTextToolBarShown);

    m_currentStackWidget = nullptr;
}

//
// BusyState
//

BusyState::BusyState(MainWindowPrivate &impl, QState *parent)
    : QState(parent)
    , m_impl(impl)
{
}

void BusyState::onEntry(QEvent *)
{
    m_impl.m_q->statusBar()->hide();

    m_impl.m_stack->setCurrentWidget(m_impl.m_busyPage);

    m_impl.m_busy->setRunning(true);

    m_impl.m_busyFlag = true;

    m_impl.disableActionsOnPlaying();

    m_impl.m_quit->setEnabled(false);

    m_impl.m_editToolBar->hide();
    m_impl.m_textToolBar->hide();
    m_impl.m_drawToolBar->hide();
    m_impl.m_drawArrowToolBar->hide();
    m_impl.m_tipsAction->setEnabled(false);
}

void BusyState::onExit(QEvent *)
{
    m_impl.m_busy->setRunning(false);

    m_impl.m_busyFlag = false;
}

//
// ReadyState
//

ReadyState::ReadyState(MainWindowPrivate &impl, QState *parent)
    : QState(parent)
    , m_impl(impl)
{
}

void ReadyState::onEntry(QEvent *)
{
    m_impl.m_q->statusBar()->show();

    m_impl.m_busyStatusLabel->clear();

    m_impl.m_stack->setCurrentWidget(m_impl.m_view);

    m_impl.enableActions();
    m_impl.m_tipsAction->setEnabled(true);
    m_impl.m_editMenu->setEnabled(true);

    m_impl.m_editToolBar->show();
}

void ReadyState::onExit(QEvent *)
{

}

//
// AboutState
//

AboutState::AboutState(MainWindowPrivate &impl, QState *parent)
    : QState(parent)
    , m_impl(impl)
{
}

void AboutState::onEntry(QEvent *)
{
    m_impl.setActionsToInitialState();

    m_impl.m_stack->setCurrentWidget(m_impl.m_about);

    m_impl.m_q->setWindowTitle(tr("GIF Editor"));
}

void AboutState::onExit(QEvent *)
{

}

//
// PlayingState
//

PlayingState::PlayingState(MainWindowPrivate &impl, QState *parent, QState *stopState)
    : QState(parent)
    , m_impl(impl)
    , m_stop(stopState)
{
    m_start = parent->addTransition(m_impl.m_playStop, &QAction::triggered, this);
    m_start->setTransitionType(QAbstractTransition::InternalTransition);
}

void PlayingState::onEntry(QEvent *)
{
    parentState()->removeTransition(m_start);
    m_start = parentState()->addTransition(m_impl.m_playStop, &QAction::triggered, m_stop);
    m_start->setTransitionType(QAbstractTransition::InternalTransition);

    m_impl.disableActionsOnPlaying();
    m_impl.m_playStop->setText(tr("Stop"));
    m_impl.m_playStop->setIcon(
        QIcon::fromTheme(QStringLiteral("media-playback-stop"), QIcon(":/img/media-playback-stop.png")));

    const auto &img = m_impl.m_view->tape()->frame(m_impl.m_view->tape()->currentFrame()->counter())->image();
    m_impl.m_playTimer->start(m_impl.m_frames.delay(img.m_pos));
}

void PlayingState::onExit(QEvent *)
{
    parentState()->removeTransition(m_start);
    m_start = parentState()->addTransition(m_impl.m_playStop, &QAction::triggered, this);
    m_start->setTransitionType(QAbstractTransition::InternalTransition);

    m_impl.m_playTimer->stop();
    m_impl.m_playStop->setText(tr("Play"));
    m_impl.m_playStop->setIcon(
        QIcon::fromTheme(QStringLiteral("media-playback-start"), QIcon(":/img/media-playback-start.png")));
    m_impl.enableActions();
}

//
// EditingState
//

EditingState::EditingState(MainWindowPrivate &impl, QState *parent)
    : QState(parent)
    , m_impl(impl)
{
}

void EditingState::onEntry(QEvent *)
{
    m_impl.m_q->hidePenWidthSpinBox();

    m_impl.enableActionsOnEdit(false);
}

void EditingState::onExit(QEvent *)
{
    m_impl.m_editMode = MainWindowPrivate::EditMode::Unknow;

    for (int i = 1; i <= m_impl.m_view->tape()->count(); ++i) {
        m_impl.m_view->tape()->frame(i)->setModified(false);
    }

    m_impl.enableActionsOnEdit();
}

//
// CropState
//

CropState::CropState(MainWindowPrivate &impl, QState *parent)
    : EditingState(impl, parent)
{
}

void CropState::onEntry(QEvent *e)
{
    EditingState::onEntry(e);

    m_impl.m_editMode = MainWindowPrivate::EditMode::Crop;

    m_impl.m_view->startCrop();

    connect(m_impl.m_view->cropFrame(), &CropFrame::started, m_impl.m_q, &MainWindow::onRectSelectionStarted);
}

void CropState::onExit(QEvent *e)
{
    m_impl.m_view->stopCrop();

    EditingState::onExit(e);
}

//
// DrawTextState
//

DrawTextState::DrawTextState(MainWindowPrivate &impl, QState *parent)
    : EditingState(impl, parent)
{
}

void DrawTextState::onEntry(QEvent *e)
{
    EditingState::onEntry(e);

    m_impl.m_editMode = MainWindowPrivate::EditMode::Text;

    m_impl.m_view->startText();

    connect(m_impl.m_view->textFrame(),
            &TextFrame::switchToTextEditingMode,
            m_impl.m_q,
            &MainWindow::onSwitchToTextEditMode);
    connect(m_impl.m_view->textFrame(),
            &TextFrame::switchToTextSelectionRectMode,
            m_impl.m_q,
            &MainWindow::onSwitchToTextSelectionRectMode);
    connect(m_impl.m_boldText, &QAction::triggered, m_impl.m_view->textFrame(), &TextFrame::boldText);
    connect(m_impl.m_italicText, &QAction::triggered, m_impl.m_view->textFrame(), &TextFrame::italicText);
    connect(m_impl.m_fontLess, &QAction::triggered, m_impl.m_view->textFrame(), &TextFrame::fontLess);
    connect(m_impl.m_fontMore, &QAction::triggered, m_impl.m_view->textFrame(), &TextFrame::fontMore);
    connect(m_impl.m_textColor, &QAction::triggered, m_impl.m_view->textFrame(), &TextFrame::textColor);
    connect(m_impl.m_clearFormat, &QAction::triggered, m_impl.m_view->textFrame(), &TextFrame::clearFormat);
    connect(m_impl.m_finishText, &QAction::triggered, m_impl.m_q, &MainWindow::applyText);
    connect(m_impl.m_view->textFrame(), &TextFrame::started, m_impl.m_q, &MainWindow::onRectSelectionStarted);
}

void DrawTextState::onExit(QEvent *e)
{
    m_impl.m_view->stopText();

    m_impl.m_textToolBar->hide();

    EditingState::onExit(e);
}

//
// DrawRectState
//

DrawRectState::DrawRectState(MainWindowPrivate &impl, QState *parent)
    : EditingState(impl, parent)
{
}

void DrawRectState::onEntry(QEvent *e)
{
    EditingState::onEntry(e);

    m_impl.m_editMode = MainWindowPrivate::EditMode::Rect;

    m_impl.m_view->startRect();

    m_impl.m_drawToolBar->show();

    connect(m_impl.m_view->rectFrame(), &RectFrame::started, m_impl.m_q, &MainWindow::onRectSelectionStarted);
    connect(m_impl.m_view->rectFrame(), &RectFrame::clicked, m_impl.m_q, &MainWindow::hidePenWidthSpinBox);
}

void DrawRectState::onExit(QEvent *e)
{
    m_impl.m_view->stopRect();

    m_impl.m_drawToolBar->hide();

    EditingState::onExit(e);
}

//
// DrawArrowState
//

DrawArrowState::DrawArrowState(MainWindowPrivate &impl, QState *parent)
    : EditingState(impl, parent)
{
}

void DrawArrowState::onEntry(QEvent *e)
{
    EditingState::onEntry(e);

    m_impl.m_editMode = MainWindowPrivate::EditMode::Arrow;

    m_impl.m_view->startArrow();

    m_impl.m_drawArrowToolBar->show();

    connect(m_impl.m_view->arrowFrame(), &ArrowFrame::started, m_impl.m_q, &MainWindow::onRectSelectionStarted);
    connect(m_impl.m_view->arrowFrame(), &ArrowFrame::clicked, m_impl.m_q, &MainWindow::hidePenWidthSpinBox);
}

void DrawArrowState::onExit(QEvent *e)
{
    m_impl.m_view->stopArrow();

    m_impl.m_drawArrowToolBar->hide();

    EditingState::onExit(e);
}
