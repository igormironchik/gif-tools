/*!
    SPDX-FileCopyrightText: 2018-2024 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF editor include.
#include "frameontape.hpp"

// Qt include.
#include <QCheckBox>
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QVBoxLayout>

//
// FrameOnTapePrivate
//

class FrameOnTapePrivate
{
public:
    FrameOnTapePrivate(const ImageRef &img, int counter, int height, FrameOnTape *parent)
        : m_counter(counter)
        , m_current(false)
        , m_label(new QLabel(parent))
        , m_checkBox(new QCheckBox(parent))
        , m_vlayout(new QVBoxLayout(parent))
        , m_frame(nullptr)
        , m_q(parent)
    {
        m_checkBox->setChecked(true);

        m_label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        m_label->setText(FrameOnTape::tr("#%1").arg(m_counter));
        m_vlayout->setSpacing(0);
        m_vlayout->setContentsMargins(0, 0, 0, 0);
    }

    //! Set current state.
    void setCurrent(bool on);

    //! Counter.
    int m_counter;
    //! Is current?
    bool m_current;
    //! Counter label.
    QLabel *m_label;
    //! Check box.
    QCheckBox *m_checkBox;
    //! Layout.
    QVBoxLayout *m_vlayout;
    //! Frame.
    Frame *m_frame;
    //! Parent.
    FrameOnTape *m_q;
}; // class FrameOnTapePrivate

void FrameOnTapePrivate::setCurrent(bool on)
{
    m_current = on;

    if (m_current) {
        m_q->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    } else {
        m_q->setFrameStyle(QFrame::Panel | QFrame::Raised);
    }
}

//
// FrameOnTape
//

FrameOnTape::FrameOnTape(const ImageRef &img, int counter, int height, QWidget *parent)
    : QFrame(parent)
    , m_d(new FrameOnTapePrivate(img, counter, height, this))
{
    setLineWidth(2);
    m_d->setCurrent(false);

    m_d->m_frame = new Frame(img,
                           Frame::ResizeMode::FitToHeight,
                           parent,
                           height - qMax(m_d->m_label->sizeHint().height(), m_d->m_checkBox->sizeHint().height()) - frameWidth() * 2);

    m_d->m_vlayout->addWidget(m_d->m_frame);

    auto hlayout = new QHBoxLayout;
    hlayout->setContentsMargins(0, 0, 0, 0);
    hlayout->addWidget(m_d->m_checkBox);
    hlayout->addWidget(m_d->m_label);

    m_d->m_vlayout->addLayout(hlayout);

    m_d->setCurrent(false);

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    connect(m_d->m_checkBox, &QCheckBox::stateChanged, this, [this](int state) {
        emit this->checked(this->m_d->m_counter, state != 0);
    });
    connect(m_d->m_frame, &Frame::clicked, this, [this]() {
        this->m_d->setCurrent(true);

        emit this->clicked(this->m_d->m_counter);
    });
}

FrameOnTape::~FrameOnTape() noexcept
{
}

const ImageRef &FrameOnTape::image() const
{
    return m_d->m_frame->image();
}

void FrameOnTape::setImagePos(qsizetype pos)
{
    m_d->m_frame->setImagePos(pos);
}

void FrameOnTape::clearImage()
{
    m_d->m_frame->clearImage();
}

void FrameOnTape::applyImage()
{
    m_d->m_frame->applyImage();
}

bool FrameOnTape::isChecked() const
{
    return m_d->m_checkBox->isChecked();
}

void FrameOnTape::setChecked(bool on)
{
    m_d->m_checkBox->setChecked(on);
}

int FrameOnTape::counter() const
{
    return m_d->m_counter;
}

void FrameOnTape::setCounter(int c)
{
    m_d->m_counter = c;

    m_d->m_label->setText(tr("#%1").arg(c));
}

bool FrameOnTape::isCurrent() const
{
    return m_d->m_current;
}

void FrameOnTape::setCurrent(bool on)
{
    m_d->setCurrent(on);
}

void FrameOnTape::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu menu(this);

    if (!m_d->m_frame->image().m_isEmpty) {
        menu.addAction(QIcon(QStringLiteral(":/img/document-save-as.png")), tr("Save this frame"), [this]() {
            auto fileName = QFileDialog::getSaveFileName(this, tr("Choose file to save to..."), QString(), tr("PNG (*.png)"));

            if (!fileName.isEmpty()) {
                if (!fileName.endsWith(QStringLiteral(".png"), Qt::CaseInsensitive)) {
                    fileName.append(QStringLiteral(".png"));
                }

                const auto img = this->m_d->m_frame->image().m_gif.at(this->m_d->m_frame->image().m_pos);
                img.save(fileName);
            }
        });

        menu.addSeparator();
    }

    menu.addAction(QIcon(QStringLiteral(":/img/list-remove.png")), tr("Uncheck till end"), [this]() {
        emit this->checkTillEnd(this->m_d->m_counter, false);
    });

    menu.addAction(QIcon(QStringLiteral(":/img/list-add.png")), tr("Check till end"), [this]() {
        emit this->checkTillEnd(this->m_d->m_counter, true);
    });

    menu.exec(e->globalPos());
}
