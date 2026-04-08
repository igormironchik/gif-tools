/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QObject>
#include <QString>

static const QString s_cropHelp = QObject::tr(
    "Select a region for cropping with the mouse, when ready press Enter. "
    "Press Escape for cancelling.");
static const QString s_textHelp = QObject::tr(
    "Select a region for text with the mouse, when ready press Enter or use "
    "context menu. You can switch between text mode and rectangle selection with "
    "context meny at any time. You can choose any frame from the tape to apply "
    "text on that frame. Text may be different on each frame. If you clicked on "
    "the frame, but don't want the text to be on it - uncheck this frame on the "
    "tape. When ready click \"Apply\" button on the tool bar. "
    "Press Escape for cancelling.");
static const QString s_rectHelp = QObject::tr(
    "Select a region for drawing a rectangle with the mouse, when ready press Enter. "
    "You can choose any frame from the tape to apply "
    "rectangle on that frame. If you clicked on "
    "the frame, but don't want the rectangle to be on it - uncheck this frame on the "
    "tape. Press Escape for cancelling.");
static const QString s_arrowHelp = QObject::tr(
    "Select a region for drawing an arrow with the mouse, when ready press Enter. "
    "You can choose any frame from the tape to apply "
    "arrow on that frame. If you clicked on "
    "the frame, but don't want the arrow to be on it - uncheck this frame on the "
    "tape. Press Escape for cancelling.");