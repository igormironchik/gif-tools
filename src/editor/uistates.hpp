/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QAbstractState>

class MainWindowPrivate;

//
// TipsState
//

//! Tips state.
class TipsState : public QAbstractState
{
public:
    explicit TipsState(MainWindowPrivate &impl, QState *parent = nullptr);
    ~TipsState() override = default;

protected:
    void onEntry(QEvent *event) override;
    void onExit(QEvent *event) override;

private:
    MainWindowPrivate &m_impl;
}; // class TipsState

//
// ModifiedState
//

//! Modified state.
class ModifiedState : public QAbstractState
{
public:
    explicit ModifiedState(MainWindowPrivate &impl, QState *parent = nullptr);
    ~ModifiedState() override = default;

protected:
    void onEntry(QEvent *event) override;
    void onExit(QEvent *event) override;

private:
    MainWindowPrivate &m_impl;
}; // class ModifiedState

//
// SavedState
//

//! Saved state.
class SavedState : public QAbstractState
{
public:
    explicit SavedState(MainWindowPrivate &impl, QState *parent = nullptr);
    ~SavedState() override = default;

protected:
    void onEntry(QEvent *event) override;
    void onExit(QEvent *event) override;

private:
    MainWindowPrivate &m_impl;
}; // class SavedState

//
// BusyState
//

//! Busy state.
class BusyState : public QAbstractState
{
public:
    explicit BusyState(MainWindowPrivate &impl, QState *parent = nullptr);
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
class ReadyState : public QAbstractState
{
public:
    explicit ReadyState(MainWindowPrivate &impl, QState *parent = nullptr);
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
class AboutState : public QAbstractState
{
public:
    explicit AboutState(MainWindowPrivate &impl, QState *parent = nullptr);
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
class PlayingState : public QAbstractState
{
public:
    explicit PlayingState(MainWindowPrivate &impl, QState *parent = nullptr);
    ~PlayingState() override = default;

protected:
    void onEntry(QEvent *event) override;
    void onExit(QEvent *event) override;

private:
    MainWindowPrivate &m_impl;
}; // class PlayingState

//
// EditingState
//

//! Base of all editing states.
class EditingState : public QAbstractState
{
public:
    explicit EditingState(MainWindowPrivate &impl, QState *parent = nullptr);
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
    explicit CropState(MainWindowPrivate &impl, QState *parent = nullptr);
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
    explicit DrawTextState(MainWindowPrivate &impl, QState *parent = nullptr);
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
    explicit DrawRectState(MainWindowPrivate &impl, QState *parent = nullptr);
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
    explicit DrawArrowState(MainWindowPrivate &impl, QState *parent = nullptr);
    ~DrawArrowState() override = default;

protected:
    void onEntry(QEvent *event) override;
    void onExit(QEvent *event) override;
}; // class DrawArrowState
