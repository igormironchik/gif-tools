/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QAbstractButton>
#include <QBitmap>
#include <QElapsedTimer>
#include <QFrame>
#include <QLabel>
#include <QProgressBar>
#include <QTemporaryDir>
#include <QTimer>
#include <QToolButton>
#include <QWidget>

class CloseButton;
class MainWindow;

//
// Title
//

//! Title widget.
class TitleWidget : public QFrame
{
    Q_OBJECT

signals:
    void resizeRequested();

public:
    TitleWidget(MainWindow *mainWindow,
                QWidget *parent);
    ~TitleWidget() override = default;

    QToolButton *recordButton() const;
    QToolButton *settingsButton() const;
    QToolButton *transparentForMouseButton() const;
    CloseButton *closeButton() const;
    QLabel *msg() const;
    QProgressBar *progressBar() const;

    bool isMouseEnabled() const;

public slots:
    void disableMouse();
    void enableMouse();

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;

private:
    void handleMouseMove(QMouseEvent *e);

private:
    Q_DISABLE_COPY(TitleWidget)

    MainWindow *m_mainWindow;
    QToolButton *m_recordButton = nullptr;
    QToolButton *m_settingsButton = nullptr;
    QToolButton *m_transparentForMouseButton = nullptr;
    CloseButton *m_closeButton = nullptr;
    QLabel *m_msg = nullptr;
    QProgressBar *m_progress = nullptr;
    bool m_leftButtonPressed = false;
    bool m_mouseEnabled = true;
    QPointF m_pos = {0.0, 0.0};
}; // class TitleWidget

//
// CloseButton
//

//! Close button.
class CloseButton : public QAbstractButton
{
    Q_OBJECT

public:
    explicit CloseButton(QWidget *parent);
    ~CloseButton() override = default;

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *e) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    Q_DISABLE_COPY(CloseButton)

    bool m_hovered = false;
    QPixmap m_activePixmap;
    QPixmap m_inactivePixmap;
}; // class CloseButton

class EventMonitor;

//
// MainWindow
//

//! Main window.
class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(EventMonitor *eventMonitor);
    ~MainWindow() override = default;

    enum Orientation {
        Unknown = 0,
        Left,
        Right,
        Top,
        Bottom,
        TopLeft,
        BottomRight,
        BottomLeft,
        TopRight,
        Move
    };

public slots:
    void onWritePercent(int percent);
    void restoreCursor(Orientation o);

protected:
    void closeEvent(QCloseEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

private slots:
    void onSettings();
    void onRecord();
    void onTimer();
    void onMousePressed();
    void onMouseReleased();
    void onKeyPressed(const QString &key);
    void onKeyReleased(const QString &key);
    void onResizeRequested();
    void onTransparentForMouse(bool checked);

private:
    void makeFrame();
    void save(const QString &fileName);
    Orientation orientationUnder(const QPoint &p) const;
    void makeAndSetMask();
    void drawRect(QPainter *p,
                  const QColor &c);

private:
    Q_DISABLE_COPY(MainWindow)

    TitleWidget *m_title = nullptr;
    QTimer *m_timer = nullptr;
    QTimer *m_keysTimer = nullptr;
    int m_fps = 24;
    bool m_grabCursor = true;
    bool m_grabKeys = false;
    bool m_drawMouseClick = true;
    bool m_recording = false;
    bool m_busy = false;
    bool m_isMouseButtonPressed = false;
    bool m_skipQuitEvent = false;
    bool m_isMouseDisabledByUser = false;
    QTemporaryDir m_dir;
    QStringList m_frames;
    QString m_key;
    qsizetype m_counter = 0;
    QElapsedTimer m_elapsed;
    QVector<int> m_delays;
    QRect m_rect;
    Orientation m_current = Unknown;
    Orientation m_cursor = Unknown;
    QPointF m_pos;
    QRegion m_topLeft;
    QRegion m_top;
    QRegion m_topRight;
    QRegion m_left;
    QRegion m_right;
    QRegion m_bottomLeft;
    QRegion m_bottom;
    QRegion m_bottomRight;
    QColor m_color;
}; // class MainWindow
