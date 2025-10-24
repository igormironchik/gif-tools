/*
    SPDX-FileCopyrightText: 2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF recorder include.
#include "mainwindow.hpp"
#include "event_monitor.hpp"
#include "settings.hpp"
#include "sizedlg.hpp"

// qgiflib include.
#include <qgiflib.hpp>

// Qt include.
#include <QApplication>
#include <QCloseEvent>
#include <QFileDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QMessageBox>
#include <QMetaMethod>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QResizeEvent>
#include <QRunnable>
#include <QScreen>
#include <QSpacerItem>
#include <QStandardPaths>
#include <QThreadPool>

#ifdef Q_OS_LINUX
#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>
#endif

#ifdef Q_OS_WINDOWS
#include <Windows.h>
#endif

static const int s_handleRadius = 9;

//
// Title
//

TitleWidget::TitleWidget(MainWindow *mainWindow, QWidget *parent)
    : QFrame(parent)
    , m_mainWindow(mainWindow)
    , m_recordButton(new QToolButton(this))
    , m_settingsButton(new QToolButton(this))
    , m_transparentForMouseButton(new QToolButton(this))
    , m_closeButton(new CloseButton(this))
    , m_msg(new QLabel(this))
    , m_progress(new QProgressBar(this))
{
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    m_recordButton->setText(tr("Record"));
    m_recordButton->setToolTip(tr("Start recording"));
    layout->addWidget(m_recordButton);
    layout->addItem(new QSpacerItem(10, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
    layout->addWidget(m_msg);
    m_progress->setMinimum(0);
    m_progress->setMaximum(100);
    m_progress->hide();
    layout->addWidget(m_progress);
    layout->addItem(new QSpacerItem(10, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
    m_settingsButton->setIcon(QIcon(":/img/applications-system.png"));
    m_settingsButton->setToolTip(tr("Settings"));
    m_transparentForMouseButton->setToolTip(tr("Turn on/off transparency for mouse events"));
    m_transparentForMouseButton->setCheckable(true);
    m_transparentForMouseButton->setChecked(false);
    m_transparentForMouseButton->setIcon(QIcon(":/img/edit-select.png"));
    layout->addWidget(m_transparentForMouseButton);
    layout->addWidget(m_settingsButton);
    layout->addWidget(m_closeButton);

    setAutoFillBackground(true);
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMouseTracking(true);
}

QToolButton *TitleWidget::recordButton() const
{
    return m_recordButton;
}

QToolButton *TitleWidget::settingsButton() const
{
    return m_settingsButton;
}

QToolButton *TitleWidget::transparentForMouseButton() const
{
    return m_transparentForMouseButton;
}

CloseButton *TitleWidget::closeButton() const
{
    return m_closeButton;
}

QLabel *TitleWidget::msg() const
{
    return m_msg;
}

QProgressBar *TitleWidget::progressBar() const
{
    return m_progress;
}

bool TitleWidget::isMouseEnabled() const
{
    return m_mouseEnabled;
}

void TitleWidget::disableMouse()
{
    m_mouseEnabled = false;
}

void TitleWidget::enableMouse()
{
    m_mouseEnabled = true;
}

void TitleWidget::mousePressEvent(QMouseEvent *e)
{
    if (m_mouseEnabled) {
        if (e->button() == Qt::LeftButton) {
            m_leftButtonPressed = true;
            m_pos = e->globalPosition();
        }

        e->accept();
    } else {
        e->ignore();
    }
}

void TitleWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_mouseEnabled) {
        if (e->button() == Qt::LeftButton && m_leftButtonPressed) {
            handleMouseMove(e);

            m_leftButtonPressed = false;
        }

        e->accept();
    } else {
        e->ignore();
    }
}

void TitleWidget::mouseMoveEvent(QMouseEvent *e)
{
    m_mainWindow->restoreCursor(MainWindow::Unknown);

    if (m_mouseEnabled) {
        if (m_leftButtonPressed) {
            handleMouseMove(e);
        }

        e->accept();
    } else {
        e->ignore();
    }
}

void TitleWidget::handleMouseMove(QMouseEvent *e)
{
    auto delta = e->globalPosition() - m_pos;
    m_pos = e->globalPosition();

    move(x() + qRound(delta.x()), y() + qRound(delta.y()));
}

void TitleWidget::contextMenuEvent(QContextMenuEvent *e)
{
    if (m_mouseEnabled) {
        QMenu menu(this);
        menu.addAction(tr("Resize Grab Area"), this, &TitleWidget::resizeRequested);

        menu.exec(e->globalPos());

        e->accept();
    } else {
        e->ignore();
    }
}

//
// CloseButton
//

CloseButton::CloseButton(QWidget *parent)
    : QAbstractButton(parent)
{
    setCheckable(false);

    m_activePixmap = QPixmap(QStringLiteral(":/img/dialog-close.png"));

    auto source = m_activePixmap.toImage();
    QImage target = QImage(source.width(), source.height(), QImage::Format_ARGB32);

    for (int x = 0; x < source.width(); ++x) {
        for (int y = 0; y < source.height(); ++y) {
            const auto g = qGray(source.pixel(x, y));
            target.setPixelColor(x, y, QColor(g, g, g, source.pixelColor(x, y).alpha()));
        }
    }

    m_inactivePixmap = QPixmap::fromImage(target);

    setFocusPolicy(Qt::NoFocus);

    setToolTip(tr("Close application"));
}

QSize CloseButton::sizeHint() const
{
    return {16, 16};
}

void CloseButton::paintEvent(QPaintEvent *e)
{
    QPainter p(this);

    if (m_hovered && isEnabled()) {
        p.drawPixmap(rect(), m_activePixmap);
    } else {
        p.drawPixmap(rect(), m_inactivePixmap);
    }
}

void CloseButton::enterEvent(QEnterEvent *event)
{
    m_hovered = true;

    update();

    event->accept();
}

void CloseButton::leaveEvent(QEvent *event)
{
    m_hovered = false;

    update();

    event->accept();
}

//
// MainWindow
//

MainWindow::MainWindow(EventMonitor *eventMonitor)
    : QWidget(nullptr,
              Qt::Window
                  | Qt::FramelessWindowHint
                  | Qt::NoDropShadowWindowHint
                  | Qt::ExpandedClientAreaHint
                  | Qt::WindowStaysOnTopHint)
    , m_title(new TitleWidget(this, this))
    , m_timer(new QTimer(this))
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowState(Qt::WindowFullScreen);

    const auto screenSize = qApp->primaryScreen()->size();
    const auto width = screenSize.width() / 3;
    const auto height = screenSize.height() / 4;
    const auto x = (screenSize.width() - width) / 2;
    const auto y = (screenSize.height() - height) / 2;

    resize(screenSize);

    m_rect = QRect(x, y, width, height);

    m_title->setMinimumWidth(width);
    m_title->move(screenSize.width() / 2 - width / 2, s_handleRadius);

    connect(m_title->closeButton(), &CloseButton::clicked, qApp, &QApplication::quit);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::onTimer);
    connect(eventMonitor, &EventMonitor::buttonPress, this, &MainWindow::onMousePressed);
    connect(eventMonitor, &EventMonitor::buttonRelease, this, &MainWindow::onMouseReleased);
    connect(m_title, &TitleWidget::resizeRequested, this, &MainWindow::onResizeRequested);
    connect(m_title->recordButton(), &QToolButton::clicked, this, &MainWindow::onRecord);
    connect(m_title->settingsButton(), &QToolButton::clicked, this, &MainWindow::onSettings);
    connect(m_title->transparentForMouseButton(), &QToolButton::toggled, this, &MainWindow::onTransparentForMouse);

    auto mask = QBitmap(s_handleRadius * 2, s_handleRadius * 2);
    mask.fill(Qt::color0);

    QPainter p(&mask);
    p.setBrush(Qt::color1);
    p.setPen(Qt::color1);

    // left
    p.drawPie(QRectF(QPointF(0, 0), QSizeF(s_handleRadius * 2, s_handleRadius * 2)), 90 * 16, 180 * 16);
    m_left = QRegion(mask);
    mask.fill(Qt::color0);
    // right
    p.drawPie(QRectF(QPointF(0, 0), QSizeF(s_handleRadius * 2, s_handleRadius * 2)), 90 * 16, -180 * 16);
    m_right = QRegion(mask);
    mask.fill(Qt::color0);
    // top
    p.drawPie(QRectF(QPointF(0, 0), QSizeF(s_handleRadius * 2, s_handleRadius * 2)), 0, 180 * 16);
    m_top = QRegion(mask);
    mask.fill(Qt::color0);
    // bottom
    p.drawPie(QRectF(QPointF(0, 0), QSizeF(s_handleRadius * 2, s_handleRadius * 2)), 0, -180 * 16);
    m_bottom = QRegion(mask);
    mask.fill(Qt::color0);
    // top-left
    p.drawPie(QRectF(QPointF(0, 0), QSizeF(s_handleRadius * 2, s_handleRadius * 2)), 0, 270 * 16);
    m_topLeft = QRegion(mask);
    mask.fill(Qt::color0);
    // bottom-right
    p.drawPie(QRectF(QPointF(0, 0), QSizeF(s_handleRadius * 2, s_handleRadius * 2)), 90 * 16, -270 * 16);
    m_bottomRight = QRegion(mask);
    mask.fill(Qt::color0);
    // bottom-left
    p.drawPie(QRectF(QPointF(0, 0), QSizeF(s_handleRadius * 2, s_handleRadius * 2)), 90 * 16, 270 * 16);
    m_bottomLeft = QRegion(mask);
    mask.fill(Qt::color0);
    // top-right
    p.drawPie(QRectF(QPointF(0, 0), QSizeF(s_handleRadius * 2, s_handleRadius * 2)), 180 * 16, -270 * 16);
    m_topRight = QRegion(mask);

    setMouseTracking(true);

    m_color = palette().color(QPalette::Highlight);
}

void MainWindow::onSettings()
{
    Settings dlg(m_fps, m_grabCursor, m_drawMouseClick, this);

    if (dlg.exec() == QDialog::Accepted) {
        m_fps = dlg.fps();
        m_grabCursor = dlg.grabCursor();
        m_drawMouseClick = dlg.drawMouseClicks();
    }
}

void MainWindow::onRecord()
{
    if (m_recording) {
        m_skipQuitEvent = false;
        m_title->recordButton()->setText(tr("Record"));
        m_title->recordButton()->setToolTip(tr("Start recording"));
        m_title->settingsButton()->setEnabled(true);
        m_title->closeButton()->setEnabled(true);

        if (!m_isMouseDisabledByUser) {
            clearMask();
            m_title->enableMouse();
        }

        update();

        m_timer->stop();

        const auto dirs = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        const auto defaultDir = dirs.first();

        auto fileName = QFileDialog::getSaveFileName(this, tr("Save As"), defaultDir, tr("GIF (*.gif)"));

        if (!fileName.isEmpty()) {
            if (!fileName.toLower().endsWith(".gif")) {
                fileName.append(".gif");
            }

            save(fileName);
        }

        m_frames.clear();
        m_dir.remove();
        m_counter = 0;
        m_elapsed.invalidate();
        m_delays.clear();
    } else {
        m_skipQuitEvent = true;
        m_title->recordButton()->setText(tr("Stop"));
        m_title->recordButton()->setToolTip(tr("Stop recording"));
        m_title->settingsButton()->setEnabled(false);
        m_title->closeButton()->setEnabled(false);

        if (!m_isMouseDisabledByUser) {
            restoreCursor(Unknown);
            makeAndSetMask();
            m_title->disableMouse();
        }

        update();

        m_timer->start(1000 / m_fps);
        m_dir = QTemporaryDir("./");
        m_elapsed.start();
        makeFrame();
    }

    m_recording = !m_recording;
}

void MainWindow::onTimer()
{
    makeFrame();
}

void MainWindow::onMousePressed()
{
    m_isMouseButtonPressed = true;
}

void MainWindow::onMouseReleased()
{
    m_isMouseButtonPressed = false;
}

void MainWindow::onResizeRequested()
{
    SizeDlg dlg(m_rect.width(), m_rect.height(), this);

    if (dlg.exec() == QDialog::Accepted) {
        m_rect.setSize(QSize(dlg.requestedWidth(), dlg.requestedHeight()));

        update();
    }
}

void MainWindow::onTransparentForMouse(bool checked)
{
    m_isMouseDisabledByUser = checked;

    if (checked) {
        restoreCursor(Unknown);
        makeAndSetMask();
        m_title->disableMouse();
    } else {
        clearMask();
        m_title->enableMouse();
    }

    update();
}

namespace /* anonymous */
{

#ifdef Q_OS_LINUX

QImage qimageFromXImage(XImage *xi)
{
    QImage::Format format = QImage::Format_ARGB32_Premultiplied;
    if (xi->depth == 24) {
        format = QImage::Format_RGB32;
    } else if (xi->depth == 16) {
        format = QImage::Format_RGB16;
    }

    QImage image =
        QImage(reinterpret_cast<uchar *>(xi->data), xi->width, xi->height, xi->bytes_per_line, format).copy();

    if ((QSysInfo::ByteOrder == QSysInfo::LittleEndian && xi->byte_order == MSBFirst)
        || (QSysInfo::ByteOrder == QSysInfo::BigEndian && xi->byte_order == LSBFirst)) {
        for (int i = 0; i < image.height(); ++i) {
            if (xi->depth == 16) {
                ushort *p = reinterpret_cast<ushort *>(image.scanLine(i));
                ushort *end = p + image.width();
                while (p < end) {
                    *p = ((*p << 8) & 0xff00) | ((*p >> 8) & 0x00ff);
                    ++p;
                }
            } else {
                uint *p = reinterpret_cast<uint *>(image.scanLine(i));
                uint *end = p + image.width();
                while (p < end) {
                    *p = ((*p << 24) & 0xff000000)
                        | ((*p << 8) & 0x00ff0000)
                        | ((*p >> 8) & 0x0000ff00)
                        | ((*p >> 24) & 0x000000ff);
                    ++p;
                }
            }
        }
    }

    if (format == QImage::Format_RGB32) {
        QRgb *p = reinterpret_cast<QRgb *>(image.bits());
        for (int y = 0; y < xi->height; ++y) {
            for (int x = 0; x < xi->width; ++x) {
                p[x] |= 0xff000000;
            }
            p += xi->bytes_per_line / 4;
        }
    }

    return image;
}

#endif // Q_OS_LINUX

std::tuple<QImage,
           QRect,
           QPoint>
grabMouseCursor(const QRect &r,
                const QImage &i)
{
    QImage cursorImage;
    QPoint cursorPos(-1, -1);
    QPoint clickPos(-1, -1);
    int w = 0;
    int h = 0;

#ifdef Q_OS_LINUX
    Display *display = XOpenDisplay(nullptr);

    if (!display) {
        return {cursorImage, {cursorPos, QSize(w, h)}, clickPos};
    }

    XFixesCursorImage *cursor = XFixesGetCursorImage(display);

    cursorPos =
        r.intersects({QPoint(cursor->x - cursor->xhot, cursor->y - cursor->yhot), QSize(cursor->width, cursor->height)})
        ? QPoint(cursor->x - cursor->xhot - r.x(), cursor->y - cursor->yhot - r.y())
        : QPoint(-1, -1);

    std::vector<uint32_t> pixels(cursor->width * cursor->height);

    w = cursorPos.x() != -1 ? cursor->width : 0;
    h = cursorPos.y() != -1 ? cursor->height : 0;

    clickPos = QPoint(cursor->x - r.x(), cursor->y - r.y());

    for (size_t i = 0; i < pixels.size(); ++i) {
        pixels[i] = cursor->pixels[i];
    }

    cursorImage = QImage((uchar *)(pixels.data()), w, h, QImage::Format_ARGB32_Premultiplied).copy();

    XFree(cursor);

    XCloseDisplay(display);

#elif defined(Q_OS_WINDOWS)
    CURSORINFO cursor = {sizeof(cursor)};

    if (GetCursorInfo(&cursor) && cursor.flags == CURSOR_SHOWING) {
        ICONINFO info = {sizeof(info)};

        if (GetIconInfo(cursor.hCursor, &info)) {
            HWND hWnd = GetDesktopWindow();
            HDC hDC = GetWindowDC(hWnd);
            HDC hdcMem = CreateCompatibleDC(hDC);
            BITMAP bmpCursor = {0};
            GetObject(info.hbmColor ? info.hbmColor : info.hbmMask, sizeof(bmpCursor), &bmpCursor);
            HBITMAP hBitmap = CreateCompatibleBitmap(hDC, bmpCursor.bmWidth, bmpCursor.bmHeight);
            auto original = SelectObject(hdcMem, hBitmap);

            const QPoint ctl(cursor.ptScreenPos.x - info.xHotspot, cursor.ptScreenPos.y - info.yHotspot);
            w = bmpCursor.bmWidth;
            h = bmpCursor.bmHeight;

            for (int x = 0; x < w; ++x) {
                for (int y = 0; y < h; ++y) {
                    const QPoint c = QPoint(x, y) + ctl;

                    if (r.contains(c)) {
                        const auto color = i.pixelColor(c - r.topLeft());
                        SetPixel(hdcMem, x, y, RGB(color.red(), color.green(), color.blue()));
                    }
                }
            }

            DrawIconEx(hdcMem, 0, 0, cursor.hCursor, 0, 0, 0, nullptr, DI_DEFAULTSIZE | DI_NORMAL);

            QImage img(bmpCursor.bmWidth, bmpCursor.bmHeight, QImage::Format_ARGB32);
            img.fill(Qt::transparent);

            for (int x = 0; x < w; ++x) {
                for (int y = 0; y < h; ++y) {
                    const QPoint c = QPoint(x, y) + ctl;

                    if (r.contains(c)) {
                        const auto winColor = GetPixel(hdcMem, x, y);
                        const auto color1 = i.pixelColor(c - r.topLeft());
                        const auto color2 = QColor(GetRValue(winColor), GetGValue(winColor), GetBValue(winColor));

                        if (color1 != color2) {
                            img.setPixelColor(x, y, color2);
                        }
                    }
                }
            }

            cursorPos = r.intersects({ctl, QSize(w, h)}) ? ctl - r.topLeft() : QPoint(-1, -1);
            w = cursorPos.x() != -1 ? w : 0;
            h = cursorPos.y() != -1 ? h : 0;
            clickPos = QPoint(cursor.ptScreenPos.x - r.x(), cursor.ptScreenPos.y - r.y());
            cursorImage = img.copy();

            if (info.hbmMask) {
                DeleteObject(info.hbmMask);
            }

            if (info.hbmColor) {
                DeleteObject(info.hbmColor);
            }

            SelectObject(hdcMem, original);

            DeleteDC(hdcMem);
            DeleteObject(hBitmap);
        }

        if (cursor.hCursor) {
            DeleteObject(cursor.hCursor);
        }
    }
#endif

    return {cursorImage, {cursorPos, QSize(w, h)}, clickPos};
}

#ifdef Q_OS_WINDOWS

bool isMouseButtonPressed()
{
    if (GetKeyState(VK_LBUTTON) & 0xF000) {
        return true;
    } else if (GetKeyState(VK_RBUTTON) & 0xF000) {
        return true;
    } else if (GetKeyState(VK_MBUTTON) & 0xF000) {
        return true;
    } else {
        return false;
    }
}

#endif // Q_OS_WINDOWS

} /* namespace anonymous */

void MainWindow::makeFrame()
{
    m_delays.push_back(m_elapsed.elapsed());
    m_elapsed.restart();

    const auto p = mapToGlobal(QPoint(m_rect.x(), m_rect.y()));
    const auto s = QSize(m_rect.width(), m_rect.height());

    auto qimg = QApplication::primaryScreen()->grabWindow(0, p.x(), p.y(), s.width(), s.height()).toImage();

    if (m_grabCursor) {
        QImage ci;
        QRect cr;
        QPoint cp;
        std::tie(ci, cr, cp) = grabMouseCursor(QRect(p, s), qimg);

        QPainter p(&qimg);

        if (m_drawMouseClick) {
#ifdef Q_OS_WINDOWS
            m_isMouseButtonPressed = isMouseButtonPressed();
#endif // Q_OS_WINDOWS
            if (m_isMouseButtonPressed) {
                QRadialGradient gradient(cp, cr.width() / 2);
                gradient.setColorAt(0, Qt::transparent);
                gradient.setColorAt(1, Qt::yellow);

                p.setPen(Qt::NoPen);
                p.setBrush(QBrush(gradient));
                p.drawEllipse(cp.x() - cr.width() / 2, cp.y() - cr.width() / 2, cr.width(), cr.width());
            }
        }

        p.drawImage(cr, ci, ci.rect());
    }

    m_frames.push_back(m_dir.filePath(QString("%1.png").arg(++m_counter)));
    qimg.save(m_frames.back());
}

namespace /* anonymous */
{

class WriteGIF final : public QRunnable
{
public:
    WriteGIF(MainWindow *progressReceiver,
             const QStringList &frames,
             const QVector<int> &delays,
             const QString &fileName)
        : m_frames(frames)
        , m_delays(delays)
        , m_fileName(fileName)
        , m_progressReceiver(progressReceiver)
    {
        setAutoDelete(false);
    }

    ~WriteGIF() noexcept override = default;

    void run() override
    {
        QGifLib::Gif gif;

        QObject::connect(&gif, &QGifLib::Gif::writeProgress, m_progressReceiver, &MainWindow::onWritePercent);

        if (!gif.write(m_fileName, m_frames, m_delays, 0)) {
            int methodIndex = m_progressReceiver->metaObject()->indexOfMethod("onWritePercent(int)");
            QMetaMethod method = m_progressReceiver->metaObject()->method(methodIndex);
            method.invoke(m_progressReceiver, Qt::QueuedConnection, 100);
        }
    }

private:
    const QStringList &m_frames;
    const QVector<int> &m_delays;
    QString m_fileName;
    MainWindow *m_progressReceiver = nullptr;
}; // class WriteGIF

} /* namespase anonymous */

void MainWindow::save(const QString &fileName)
{
    m_title->recordButton()->setEnabled(false);
    m_title->settingsButton()->setEnabled(false);

    m_title->msg()->setText(tr("Writing GIF... Please wait."));

    QApplication::processEvents();

    m_busy = true;

    WriteGIF runnable(this, m_frames, m_delays, fileName);
    QThreadPool::globalInstance()->start(&runnable);

    while (!QThreadPool::globalInstance()->waitForDone(10)) {
        QApplication::processEvents();
    }

    m_busy = false;

    m_title->recordButton()->setEnabled(true);
    m_title->settingsButton()->setEnabled(true);
    m_title->msg()->setText({});

    m_frames.clear();
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if (m_busy) {
        const auto btn = QMessageBox::question(this,
                                               tr("GIF recorder is busy..."),
                                               tr("GIF recorder is busy.\nDo you want to terminate the application?"));

        if (btn == QMessageBox::Yes) {
            exit(-1);
        } else {
            e->ignore();
        }
    } else if (m_skipQuitEvent) {
        e->ignore();
    } else {
        e->accept();
    }
}

void MainWindow::drawRect(QPainter *p,
                          const QColor &c)
{
    p->setBrush(Qt::NoBrush);
    p->setPen(c);
    p->drawRect(m_rect);

    p->setBrush(c);

    if (m_current == Unknown) {
        // left
        p->drawPie(QRectF(QPointF(m_rect.x() - s_handleRadius, m_rect.y() + m_rect.height() / 2 - s_handleRadius),
                          QSizeF(s_handleRadius * 2, s_handleRadius * 2)),
                   90 * 16,
                   180 * 16);
        // right
        p->drawPie(QRectF(QPointF(m_rect.x() + m_rect.width() - s_handleRadius,
                                  m_rect.y() + m_rect.height() / 2 - s_handleRadius),
                          QSizeF(s_handleRadius * 2, s_handleRadius * 2)),
                   90 * 16,
                   -180 * 16);
        // top
        p->drawPie(QRectF(QPointF(m_rect.x() + m_rect.width() / 2 - s_handleRadius, m_rect.y() - s_handleRadius),
                          QSizeF(s_handleRadius * 2, s_handleRadius * 2)),
                   0,
                   180 * 16);
        // bottom
        p->drawPie(QRectF(QPointF(m_rect.x() + m_rect.width() / 2 - s_handleRadius,
                                  m_rect.y() + m_rect.height() - s_handleRadius),
                          QSizeF(s_handleRadius * 2, s_handleRadius * 2)),
                   0,
                   -180 * 16);
        // top-left
        p->drawPie(QRectF(QPointF(m_rect.x() - s_handleRadius, m_rect.y() - s_handleRadius),
                          QSizeF(s_handleRadius * 2, s_handleRadius * 2)),
                   0,
                   270 * 16);
        // bottom-right
        p->drawPie(
            QRectF(QPointF(m_rect.x() + m_rect.width() - s_handleRadius, m_rect.y() + m_rect.height() - s_handleRadius),
                   QSizeF(s_handleRadius * 2, s_handleRadius * 2)),
            90 * 16,
            -270 * 16);
        // bottom-left
        p->drawPie(QRectF(QPointF(m_rect.x() - s_handleRadius, m_rect.y() + m_rect.height() - s_handleRadius),
                          QSizeF(s_handleRadius * 2, s_handleRadius * 2)),
                   90 * 16,
                   270 * 16);
        // top-right
        p->drawPie(QRectF(QPointF(m_rect.x() + m_rect.width() - s_handleRadius, m_rect.y() - s_handleRadius),
                          QSizeF(s_handleRadius * 2, s_handleRadius * 2)),
                   180 * 16,
                   -270 * 16);
    }
}

void MainWindow::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    if (m_title->isMouseEnabled()) {
        QPainterPath path;
        path.addRect(rect());
        path.addRect(m_rect);
        path.setFillRule(Qt::OddEvenFill);

        p.setPen(Qt::NoPen);

        auto c = palette().color(QPalette::Window).darker(300);
        c.setAlpha(75);

        p.setBrush(c);
        p.drawPath(path);
    }

    drawRect(&p, m_color);
}

MainWindow::Orientation MainWindow::orientationUnder(const QPoint &p) const
{
    if (m_topLeft.translated(QPoint(m_rect.x() - s_handleRadius, m_rect.y() - s_handleRadius)).contains(p)) {
        return TopLeft;
    } else if (m_top.translated(QPoint(m_rect.x() + m_rect.width() / 2 - s_handleRadius, m_rect.y() - s_handleRadius))
                   .contains(p)) {
        return Top;
    } else if (m_topRight.translated(QPoint(m_rect.x() + m_rect.width() - s_handleRadius, m_rect.y() - s_handleRadius))
                   .contains(p)) {
        return TopRight;
    } else if (m_left.translated(QPoint(m_rect.x() - s_handleRadius, m_rect.y() + m_rect.height() / 2 - s_handleRadius))
                   .contains(p)) {
        return Left;
    } else if (m_right
                   .translated(QPoint(m_rect.x() + m_rect.width() - s_handleRadius,
                                      m_rect.y() + m_rect.height() / 2 - s_handleRadius))
                   .contains(p)) {
        return Right;
    } else if (m_bottomLeft
                   .translated(QPoint(m_rect.x() - s_handleRadius, m_rect.y() + m_rect.height() - s_handleRadius))
                   .contains(p)) {
        return BottomLeft;
    } else if (m_bottom
                   .translated(QPoint(m_rect.x() + m_rect.width() / 2 - s_handleRadius,
                                      m_rect.y() + m_rect.height() - s_handleRadius))
                   .contains(p)) {
        return Bottom;
    } else if (m_bottomRight
                   .translated(QPoint(m_rect.x() + m_rect.width() - s_handleRadius,
                                      m_rect.y() + m_rect.height() - s_handleRadius))
                   .contains(p)) {
        return BottomRight;
    } else if (m_rect.contains(p)) {
        return Move;
    } else {
        return Unknown;
    }
}

void MainWindow::mousePressEvent(QMouseEvent *e)
{
    if (m_title->isMouseEnabled()) {
        if (e->button() == Qt::LeftButton) {
            m_pos = e->globalPosition();
            m_current = orientationUnder(m_pos.toPoint());

            update();
        }

        e->accept();
    } else {
        e->ignore();
    }
}

void MainWindow::restoreCursor(Orientation o)
{
    if (m_cursor != Unknown) {
        QApplication::restoreOverrideCursor();
    }

    m_cursor = o;
}

void MainWindow::mouseMoveEvent(QMouseEvent *e)
{
    if (m_title->isMouseEnabled()) {
        if (m_current != Unknown) {
            auto delta = e->globalPosition() - m_pos;
            m_pos = e->globalPosition();

            switch (m_current) {
            case TopLeft: {
                m_rect.setTopLeft(m_rect.topLeft() + delta.toPoint());
            } break;

            case Top: {
                m_rect.setY(m_rect.y() + delta.toPoint().y());
            } break;

            case TopRight: {
                m_rect.setTopRight(m_rect.topRight() + delta.toPoint());
            } break;

            case Left: {
                m_rect.setX(m_rect.x() + delta.toPoint().x());
            } break;

            case Right: {
                m_rect.setWidth(m_rect.width() + delta.toPoint().x());
            } break;

            case BottomLeft: {
                m_rect.setBottomLeft(m_rect.bottomLeft() + delta.toPoint());
            } break;

            case Bottom: {
                m_rect.setHeight(m_rect.height() + delta.toPoint().y());
            } break;

            case BottomRight: {
                m_rect.setBottomRight(m_rect.bottomRight() + delta.toPoint());
            } break;

            case Move: {
                m_rect.moveCenter(m_rect.center() + delta.toPoint());
            } break;

            default:
                break;
            }

            update();
        } else {
            const auto handle = orientationUnder(e->globalPosition().toPoint());

            if (handle != m_cursor) {
                restoreCursor(handle);

                switch (handle) {
                case TopLeft:
                case BottomRight:
                    QApplication::setOverrideCursor(Qt::SizeFDiagCursor);
                    break;

                case Top:
                case Bottom:
                    QApplication::setOverrideCursor(Qt::SizeVerCursor);
                    break;

                case TopRight:
                case BottomLeft:
                    QApplication::setOverrideCursor(Qt::SizeBDiagCursor);
                    break;

                case Left:
                case Right:
                    QApplication::setOverrideCursor(Qt::SizeHorCursor);
                    break;

                case Move:
                    QApplication::setOverrideCursor(Qt::OpenHandCursor);
                    break;

                default:
                    break;
                }
            }
        }

        e->accept();
    } else {
        e->ignore();
    }
}

void MainWindow::makeAndSetMask()
{
    auto mask = QBitmap(size());
    mask.fill(Qt::color0);

    QPainter p(&mask);
    drawRect(&p, Qt::color1);
    auto r = m_title->rect();
    r.moveTopLeft(m_title->pos());
    p.drawRect(r);

    setMask(mask);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_title->isMouseEnabled()) {
        if (e->button() == Qt::LeftButton && m_current) {
            m_current = Unknown;
            m_rect = m_rect.normalized();

            update();
        }

        e->accept();
    } else {
        e->ignore();
    }
}

void MainWindow::onWritePercent(int percent)
{
    if (percent == 0) {
        m_title->progressBar()->show();
    }

    m_title->progressBar()->setValue(percent);

    if (percent == 100) {
        m_title->progressBar()->hide();
    }
}
