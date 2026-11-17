/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF editor include.
#include "toolbar.hpp"

// Qt include.
#include <QFrame>
#include <QGridLayout>
#include <QToolBar>
#include <QToolButton>

enum class Type {
    Widget = 0,
    Line
}; // enum class Type

class ToolBarPrivate
{
public:
    ToolBarPrivate(ToolBar *parent)
        : m_q(parent)
        , m_l(new QGridLayout(m_q))
    {
        m_l->setContentsMargins(0, 0, 0, 0);
        m_l->setSpacing(3);
    }

    //! Parent.
    ToolBar *m_q;
    //! Layout.
    QGridLayout *m_l;
    //! Tool bar.
    QToolBar *m_bar = nullptr;

    //! Aux data for widget.
    struct Widget {
        Widget(Type t,
               QWidget *w,
               QWidget *aw)
            : m_type(t)
            , m_widget(w)
            , m_auxWidget(aw)
        {
        }

        Type m_type = Type::Widget;
        QWidget *m_widget = nullptr;
        QWidget *m_auxWidget = nullptr;
    }; // struct Widget

    //! List of widgets.
    QList<Widget> m_widgets;
};

//
// ToolBar
//

ToolBar::ToolBar(QWidget *parent)
    : QWidget(parent)
    , m_d(new ToolBarPrivate(this))
{
}

ToolBar::~ToolBar() = default;

inline int row(QGridLayout *l,
               Qt::Orientation o,
               int column)
{
    switch (o) {
    case Qt::Horizontal:
        return column;

    case Qt::Vertical:
        return l->rowCount() - column;
    }

    return 0;
}

inline int column(QGridLayout *l,
                  Qt::Orientation o,
                  int column)
{
    switch (o) {
    case Qt::Horizontal:
        return l->columnCount() - column;

    case Qt::Vertical:
        return column;
    }

    return 0;
}

inline void addWidgets(QGridLayout *l,
                       QWidget *button,
                       Qt::Orientation o,
                       QWidget *widget)
{
    l->addWidget(button, row(l, o, 0), column(l, o, 0));

    if (widget) {
        l->addWidget(widget, row(l, o, 1), column(l, o, 1));
    }
}

void ToolBar::addPair(QToolButton *button,
                      QWidget *widget)
{
    addWidgets(m_d->m_l, button, m_d->m_bar->orientation(), widget);

    m_d->m_widgets.append(ToolBarPrivate::Widget(Type::Widget, button, widget));
}

QFrame *makeLine(ToolBar *bar,
                 Qt::Orientation o)
{
    auto frame = new QFrame(bar);
    frame->setFrameShadow(QFrame::Raised);

    switch (o) {
    case Qt::Vertical: {
        frame->setFrameShape(QFrame::HLine);
    } break;

    case Qt::Horizontal: {
        frame->setFrameShape(QFrame::VLine);
    } break;

    default:
        break;
    }

    return frame;
}

inline void addSeparatorImpl(ToolBarPrivate::Widget &w,
                             Qt::Orientation o,
                             ToolBar *bar,
                             QGridLayout *l)
{
    auto line = makeLine(bar, o);

    l->addWidget(line, row(l, o, 0), column(l, o, 0));

    w.m_widget = line;
}

void ToolBar::addSeparator()
{
    m_d->m_widgets.append(ToolBarPrivate::Widget(Type::Line, nullptr, nullptr));

    addSeparatorImpl(m_d->m_widgets.back(), m_d->m_bar->orientation(), this, m_d->m_l);
}

void ToolBar::setQToolBar(QToolBar *bar)
{
    if (m_d->m_bar) {
        disconnect(m_d->m_bar, 0, this, 0);
    }

    m_d->m_bar = bar;

    connect(m_d->m_bar, &QToolBar::orientationChanged, this, &ToolBar::onOrientationChanged);
}

void ToolBar::onOrientationChanged(Qt::Orientation o)
{
    QLayoutItem *child;
    while ((child = m_d->m_l->takeAt(0)) != nullptr) {
        delete child;
    }

    for (auto it = m_d->m_widgets.begin(), last = m_d->m_widgets.end(); it != last; ++it) {
        switch (it->m_type) {
        case Type::Widget: {
            addWidgets(m_d->m_l, it->m_widget, m_d->m_bar->orientation(), it->m_auxWidget);
        } break;

        case Type::Line: {
            it->m_widget->deleteLater();
            it->m_widget = nullptr;
            addSeparatorImpl(*it, m_d->m_bar->orientation(), this, m_d->m_l);
        }
        }
    }
}
