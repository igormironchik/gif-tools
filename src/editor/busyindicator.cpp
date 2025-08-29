/*
    SPDX-FileCopyrightText: 2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF editor include.
#include "busyindicator.hpp"

// Qt include.
#include <QPainter>
#include <QPainterPath>
#include <QVariantAnimation>

//
// BusyIndicatorPrivate
//

class BusyIndicatorPrivate
{
public:
    BusyIndicatorPrivate(BusyIndicator *parent)
        : m_outerRadius(10)
        , m_innerRadius(static_cast<int>(m_outerRadius * 0.6))
        , m_size(m_outerRadius * 2,
                 m_outerRadius * 2)
        , m_running(true)
        , m_animation(nullptr)
        , m_q(parent)
    {
    }

    void init();

    int m_outerRadius;
    int m_innerRadius;
    int m_percent = 0;
    QSize m_size;
    QColor m_color;
    bool m_running;
    bool m_showPercent = false;
    QVariantAnimation *m_animation;
    BusyIndicator *m_q;
}; // class BusyIndicatorPrivate

void BusyIndicatorPrivate::init()
{
    m_animation = new QVariantAnimation(m_q);
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(359.0);
    m_animation->setDuration(1000);
    m_animation->setLoopCount(-1);

    QObject::connect(m_animation, &QVariantAnimation::valueChanged, m_q, &BusyIndicator::_q_update);

    m_color = m_q->palette().color(QPalette::Highlight);

    m_animation->start();
}

//
// BusyIndicator
//

BusyIndicator::BusyIndicator(QWidget *parent)
    : QWidget(parent)
    , m_d(new BusyIndicatorPrivate(this))
{
    m_d->init();
}

BusyIndicator::~BusyIndicator() noexcept
{
    m_d->m_animation->stop();
}

bool BusyIndicator::isRunning() const
{
    return m_d->m_running;
}

void BusyIndicator::setRunning(bool on)
{
    if (on != m_d->m_running) {
        m_d->m_running = on;

        if (m_d->m_running) {
            show();
            m_d->m_animation->start();
        } else {
            hide();
            m_d->m_animation->stop();
        }
    }
}

const QColor &BusyIndicator::color() const
{
    return m_d->m_color;
}

void BusyIndicator::setColor(const QColor &c)
{
    if (m_d->m_color != c) {
        m_d->m_color = c;
        update();
    }
}

int BusyIndicator::radius() const
{
    return m_d->m_outerRadius;
}

void BusyIndicator::setRadius(int r)
{
    if (m_d->m_outerRadius != r) {
        m_d->m_outerRadius = r;
        m_d->m_innerRadius = static_cast<int>(m_d->m_outerRadius * 0.6);
        m_d->m_size = QSize(m_d->m_outerRadius * 2, m_d->m_outerRadius * 2);

        updateGeometry();
    }
}

int BusyIndicator::percent() const
{
    return m_d->m_percent;
}

void BusyIndicator::setPercent(int p)
{
    if (m_d->m_percent != p) {
        m_d->m_percent = p;

        if (m_d->m_showPercent) {
            update();
        }
    }
}

bool BusyIndicator::showPercent() const
{
    return m_d->m_showPercent;
}

void BusyIndicator::setShowPercent(bool on)
{
    if (m_d->m_showPercent != on) {
        m_d->m_showPercent = on;

        update();
    }
}

QSize BusyIndicator::minimumSizeHint() const
{
    return m_d->m_size;
}

QSize BusyIndicator::sizeHint() const
{
    return m_d->m_size;
}

void BusyIndicator::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.translate(width() / 2, height() / 2);

    QPainterPath path;
    path.setFillRule(Qt::OddEvenFill);
    path.addEllipse(-m_d->m_outerRadius, -m_d->m_outerRadius, m_d->m_outerRadius * 2, m_d->m_outerRadius * 2);
    path.addEllipse(-m_d->m_innerRadius, -m_d->m_innerRadius, m_d->m_innerRadius * 2, m_d->m_innerRadius * 2);

    p.setPen(Qt::NoPen);

    QConicalGradient gradient(0, 0, -m_d->m_animation->currentValue().toReal());
    gradient.setColorAt(0.0, Qt::transparent);
    gradient.setColorAt(0.05, m_d->m_color);
    gradient.setColorAt(1.0, Qt::transparent);

    p.setBrush(gradient);

    p.drawPath(path);

    if (m_d->m_showPercent) {
        p.setBrush(m_d->m_color);
        p.setPen(m_d->m_color);
        auto f = p.font();
        f.setPixelSize(qRound((double)m_d->m_innerRadius * 0.8));
        p.setFont(f);
        p.drawText(QRect(-m_d->m_innerRadius, -m_d->m_innerRadius, m_d->m_innerRadius * 2, m_d->m_innerRadius * 2),
                   Qt::AlignHCenter | Qt::AlignVCenter,
                   QString("%1%2").arg(QString::number(m_d->m_percent), QStringLiteral("%")));
    }
}

void BusyIndicator::_q_update(const QVariant &)
{
    update();
}
