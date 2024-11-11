/*!
    SPDX-FileCopyrightText: 2018-2024 Igor Mironchik <igor.mironchik@gmail.com>
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

class MainWindow;

//
// ResizeHandle
//

//! Resize handle.
class ResizeHandle : public QFrame
{
    Q_OBJECT

public:
    enum Orientation { Horizontal = 0, Vertical, TopLeftBotomRight, BottomLeftTopRight };

    ResizeHandle(Orientation o, bool withMove, QWidget *parent, MainWindow *obj);
    ~ResizeHandle() override = default;

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;

private:
    void handleMouseMove(QMouseEvent *e);

private:
    Q_DISABLE_COPY(ResizeHandle)

    MainWindow *m_obj = nullptr;
    Orientation m_orient = Orientation::Horizontal;
    bool m_leftButtonPressed = false;
    bool m_withMove = false;
    QPointF m_pos = {0.0, 0.0};
}; // class ResizeHandle

//
// Title
//

//! Title widget.
class TitleWidget : public QFrame
{
    Q_OBJECT

public:
    TitleWidget(QWidget *parent, MainWindow *obj);
    ~TitleWidget() override = default;

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;

private:
    void handleMouseMove(QMouseEvent *e);

private:
    Q_DISABLE_COPY(TitleWidget)

    MainWindow *m_obj = nullptr;
    bool m_leftButtonPressed = false;
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

public slots:
    void onWritePercent(int percent);

protected:
    void resizeEvent(QResizeEvent *e) override;
    void closeEvent(QCloseEvent *e) override;

private slots:
    void onSettings();
    void onRecord();
    void onTimer();
    void onMousePressed();
    void onMouseReleased();

private:
    void makeFrame();
    void save(const QString &fileName);

private:
    Q_DISABLE_COPY(MainWindow)

    TitleWidget *m_title = nullptr;
    QWidget *m_recordArea = nullptr;
    QWidget *m_c = nullptr;
    QToolButton *m_recordButton = nullptr;
    QToolButton *m_settingsButton = nullptr;
    CloseButton *m_closeButton = nullptr;
    QTimer *m_timer = nullptr;
    QLabel *m_msg = nullptr;
    QProgressBar *m_progress = nullptr;
    QBitmap m_mask;
    int m_fps = 24;
    bool m_grabCursor = true;
    bool m_drawMouseClick = true;
    bool m_recording = false;
    bool m_busy = false;
    bool m_isMouseButtonPressed = false;
    bool m_skipQuitEvent = false;
    QTemporaryDir m_dir;
    QStringList m_frames;
    qsizetype m_counter = 0;
    QElapsedTimer m_elapsed;
    QVector<int> m_delays;
}; // class MainWindow
