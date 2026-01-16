/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef GIF_EDITOR_RECTANGLE_HPP_INCLUDED
#define GIF_EDITOR_RECTANGLE_HPP_INCLUDED

// Qt include.
#include <QScopedPointer>
#include <QWidget>

class Frame;

//
// RectangleSelection
//

class RectangleSelection;

class RectangleSelectionPrivate
{
public:
    RectangleSelectionPrivate(RectangleSelection *parent,
                              Frame *toObserve);

    virtual ~RectangleSelectionPrivate() = default;

    enum class Handle {
        Unknown,
        TopLeft,
        Top,
        TopRight,
        Right,
        BottomRight,
        Bottom,
        BottomLeft,
        Left
    }; // enum class Handle

    //! Bound point to available space.
    QPoint boundToAvailable(const QPoint &p) const;
    //! Bound left top point to available space.
    QPoint boundLeftTopToAvailable(const QPoint &p) const;
    //! Check and override cursor if necessary.
    void checkAndOverrideCursor(Qt::CursorShape shape);
    //! Override cursor.
    void overrideCursor(const QPoint &pos);
    //! Resize crop.
    void resize(const QPoint &pos);
    //! \return Selected rect.
    QRectF selected(const QRect &full) const;
    //! Restore overriden cursor.
    void restoreOverridenCursor();
    //! \return Is handles should be outside selected rect.
    bool isHandleOutside() const;
    //! \return Top-left handle rect.
    QRect topLeftHandleRect() const;
    //! \return Top-right handle rect.
    QRect topRightHandleRect() const;
    //! \return Bottom-right handle rect.
    QRect bottomRightHandleRect() const;
    //! \return Bottom-left handle rect.
    QRect bottomLeftHandleRect() const;
    //! \return Y handle width.
    int yHandleWidth() const;
    //! \return X handle height.
    int xHandleHeight() const;
    //! \return Y handle x position.
    int yHandleXPos() const;
    //! \return X handle y position.
    int xHandleYPos() const;
    //! \return Top handle rect.
    QRect topHandleRect() const;
    //! \return Bottom handle rect.
    QRect bottomHandleRect() const;
    //! \return Left handle rect.
    QRect leftHandleRect() const;
    //! \return Right handle rect.
    QRect rightHandleRect() const;

    //! Selected rectangle.
    QRectF m_selected;
    //! Available rectangle.
    QRect m_available;
    //! Mouse pos.
    QPoint m_mousePos;
    //! Selecting started.
    bool m_started;
    //! Nothing selected yet.
    bool m_nothing;
    //! Clicked.
    bool m_clicked;
    //! Hover entered.
    bool m_hovered;
    //! Context menu is active.
    bool m_menu = false;
    //! Cursor overriden.
    bool m_cursorOverriden;
    //! Tmp cursor shape.
    Qt::CursorShape m_cursor;
    //! Current handle.
    Handle m_handle;
    //! Frame to observe resize event.
    Frame *m_frame;
    //! Parent.
    RectangleSelection *m_q;
}; // class RectangleSelectionPrivate

//! Rectangular selection.
class RectangleSelection : public QWidget
{
    Q_OBJECT

public:
    RectangleSelection(Frame *parent = nullptr);
    ~RectangleSelection() noexcept override;

    //! \return Selection rectangle.
    QRect selectionRect() const;

public slots:
    //! Start.
    void start();
    //! Stop.
    void stop();

private slots:
    //! Frame resized.
    void frameResized();

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void enterEvent(QEnterEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;

protected:
    QScopedPointer<RectangleSelectionPrivate> m_d;

private:
    Q_DISABLE_COPY(RectangleSelection)
}; // class RectangleSelection

#endif // GIF_EDITOR_RECTANGLE_HPP_INCLUDED
