/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QState>

class MainWindowPrivate;

//
// TipsState
//

//! Tips state.
class TipsState : public QState
{
public:
    explicit TipsState(MainWindowPrivate &impl,
                       QState *parent = nullptr);
    ~TipsState() override = default;

protected:
    void onEntry(QEvent *event) override;
    void onExit(QEvent *event) override;

private:
    MainWindowPrivate &m_impl;

    bool m_isEditActionsEnabled = false;
    bool m_isEditToolBarShown = false;
    bool m_isTextToolBarShown = false;
    bool m_isDrawToolBarShow = false;
    bool m_isDrawArrowToolBarShow = false;
    QWidget *m_currentStackWidget = nullptr;
}; // class TipsState

//
// BusyState
//

//! Busy state.
class BusyState : public QState
{
public:
    explicit BusyState(MainWindowPrivate &impl,
                       QState *parent = nullptr);
    ~BusyState() override = default;

protected:
    void onEntry(QEvent *event) override;
    void onExit(QEvent *event) override;

private:
    MainWindowPrivate &m_impl;
}; // class BusyState

//
// ReadyState
//

//! Ready state.
class ReadyState : public QState
{
public:
    explicit ReadyState(MainWindowPrivate &impl,
                        QState *parent = nullptr);
    ~ReadyState() override = default;

protected:
    void onEntry(QEvent *event) override;
    void onExit(QEvent *event) override;

private:
    MainWindowPrivate &m_impl;
}; // class ReadyState

//
// AboutState
//

//! "About" state.
class AboutState : public QState
{
public:
    explicit AboutState(MainWindowPrivate &impl,
                        QState *parent = nullptr);
    ~AboutState() override = default;

protected:
    void onEntry(QEvent *event) override;
    void onExit(QEvent *event) override;

private:
    MainWindowPrivate &m_impl;
}; // class AboutState

//
// PlayingState
//

//! Playing GIF state.
class PlayingState : public QState
{
public:
    explicit PlayingState(MainWindowPrivate &impl,
                          QState *parent,
                          QState *stopState);
    ~PlayingState() override = default;

protected:
    void onEntry(QEvent *event) override;
    void onExit(QEvent *event) override;

private:
    MainWindowPrivate &m_impl;
    QSignalTransition *m_start = nullptr;
    QState *m_stop = nullptr;
}; // class PlayingState

//
// EditingState
//

//! Base of all editing states.
class EditingState : public QState
{
public:
    explicit EditingState(MainWindowPrivate &impl,
                          QState *parent = nullptr);
    ~EditingState() override = default;

protected:
    void onEntry(QEvent *event) override;
    void onExit(QEvent *event) override;

protected:
    MainWindowPrivate &m_impl;
}; // class EditingState

//
// CropState
//

//! Cropping state.
class CropState : public EditingState
{
public:
    explicit CropState(MainWindowPrivate &impl,
                       QState *parent = nullptr);
    ~CropState() override = default;

protected:
    void onEntry(QEvent *event) override;
    void onExit(QEvent *event) override;
}; // class CropState

//
// DrawTextState
//

//! Drawing text state.
class DrawTextState : public EditingState
{
public:
    explicit DrawTextState(MainWindowPrivate &impl,
                           QState *parent = nullptr);
    ~DrawTextState() override = default;

protected:
    void onEntry(QEvent *event) override;
    void onExit(QEvent *event) override;
}; // class DrawTextState

//
// DrawRectState
//

//! Drawing rect state.
class DrawRectState : public EditingState
{
public:
    explicit DrawRectState(MainWindowPrivate &impl,
                           QState *parent = nullptr);
    ~DrawRectState() override = default;

protected:
    void onEntry(QEvent *event) override;
    void onExit(QEvent *event) override;
}; // class DrawRectState

//
// DrawArrowState
//

//! Drawing arrow state.
class DrawArrowState : public EditingState
{
public:
    explicit DrawArrowState(MainWindowPrivate &impl,
                            QState *parent = nullptr);
    ~DrawArrowState() override = default;

protected:
    void onEntry(QEvent *event) override;
    void onExit(QEvent *event) override;
}; // class DrawArrowState
