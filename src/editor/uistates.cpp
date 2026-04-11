/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF editor include.
#include "mainwindow_private.hpp"
#include "uistates.hpp"

//
// TipsState
//

TipsState::TipsState(MainWindowPrivate &impl, QState *parent)
    : QAbstractState(parent)
    , m_impl(impl)
{
}

void TipsState::onEntry(QEvent *)
{

}

void TipsState::onExit(QEvent *)
{

}

//
// ModifiedState
//

ModifiedState::ModifiedState(MainWindowPrivate &impl, QState *parent)
    : QAbstractState(parent)
    , m_impl(impl)
{
}

void ModifiedState::onEntry(QEvent *)
{

}

void ModifiedState::onExit(QEvent *)
{

}

//
// SavedState
//

SavedState::SavedState(MainWindowPrivate &impl, QState *parent)
    : QAbstractState(parent)
    , m_impl(impl)
{
}

void SavedState::onEntry(QEvent *)
{

}

void SavedState::onExit(QEvent *)
{

}

//
// BusyState
//

BusyState::BusyState(MainWindowPrivate &impl, QState *parent)
    : QAbstractState(parent)
    , m_impl(impl)
{
}

void BusyState::onEntry(QEvent *)
{

}

void BusyState::onExit(QEvent *)
{

}

//
// ReadyState
//

ReadyState::ReadyState(MainWindowPrivate &impl, QState *parent)
    : QAbstractState(parent)
    , m_impl(impl)
{
}

void ReadyState::onEntry(QEvent *)
{

}

void ReadyState::onExit(QEvent *)
{

}

//
// AboutState
//

AboutState::AboutState(MainWindowPrivate &impl, QState *parent)
    : QAbstractState(parent)
    , m_impl(impl)
{
}

void AboutState::onEntry(QEvent *)
{

}

void AboutState::onExit(QEvent *)
{

}

//
// PlayingState
//

PlayingState::PlayingState(MainWindowPrivate &impl, QState *parent)
    : QAbstractState(parent)
    , m_impl(impl)
{
}

void PlayingState::onEntry(QEvent *)
{

}

void PlayingState::onExit(QEvent *)
{

}

//
// EditingState
//

EditingState::EditingState(MainWindowPrivate &impl, QState *parent)
    : QAbstractState(parent)
    , m_impl(impl)
{
}

void EditingState::onEntry(QEvent *)
{

}

void EditingState::onExit(QEvent *)
{

}

//
// CropState
//

CropState::CropState(MainWindowPrivate &impl, QState *parent)
    : EditingState(impl, parent)
{
}

void CropState::onEntry(QEvent *)
{

}

void CropState::onExit(QEvent *)
{

}

//
// DrawTextState
//

DrawTextState::DrawTextState(MainWindowPrivate &impl, QState *parent)
    : EditingState(impl, parent)
{
}

void DrawTextState::onEntry(QEvent *)
{

}

void DrawTextState::onExit(QEvent *)
{

}

//
// DrawRectState
//

DrawRectState::DrawRectState(MainWindowPrivate &impl, QState *parent)
    : EditingState(impl, parent)
{
}

void DrawRectState::onEntry(QEvent *)
{

}

void DrawRectState::onExit(QEvent *)
{

}

//
// DrawArrowState
//

DrawArrowState::DrawArrowState(MainWindowPrivate &impl, QState *parent)
    : EditingState(impl, parent)
{
}

void DrawArrowState::onEntry(QEvent *)
{

}

void DrawArrowState::onExit(QEvent *)
{

}
