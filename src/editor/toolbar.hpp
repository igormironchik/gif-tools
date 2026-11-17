/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef GIF_EDITOR_TOOLBAR_HPP_INCLUDED
#define GIF_EDITOR_TOOLBAR_HPP_INCLUDED

// Qt include.
#include <QScopedPointer>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QToolButton;
class QToolBar;
QT_END_NAMESPACE

//
// ToolBar
//

class ToolBarPrivate;

//! Tool bar with actions.
class ToolBar final : public QWidget
{
    Q_OBJECT

public:
    explicit ToolBar(QWidget *parent);
    ~ToolBar() override;

public slots:
    void addPair(QToolButton *button,
                 QWidget *widget = nullptr);
    void addSeparator();
    void setQToolBar(QToolBar *bar);

private slots:
    void onOrientationChanged(Qt::Orientation o);

private:
    friend class ToolBarPrivate;

    Q_DISABLE_COPY(ToolBar)

    QScopedPointer<ToolBarPrivate> m_d;
}; // ToolBar

#endif // GIF_EDITOR_TOOLBAR_HPP_INCLUDED
