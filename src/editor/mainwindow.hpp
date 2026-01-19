/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef GIF_EDITOR_MAINWINDOW_HPP_INCLUDED
#define GIF_EDITOR_MAINWINDOW_HPP_INCLUDED

// Qt include.
#include <QMainWindow>
#include <QScopedPointer>

//
// MainWindow
//

class MainWindowPrivate;

//! Main window.
class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow() noexcept override;

protected:
    void closeEvent(QCloseEvent *e) override;

private slots:
    //! Open GIF.
    void openGif();
    //! Save GIF.
    void saveGif();
    //! Save GIF as.
    void saveGifAs();
    //! Quit.
    void quit();
    //! Frame checked/unchecked.
    void frameChecked(int idx,
                      bool on);
    //! Crop.
    void crop(bool on);
    //! Insert text.
    void insertText(bool on);
    //! Cancel edit.
    void cancelEdit();
    //! Apply edit.
    void applyEdit();
    //! About dialog
    void about();
    //! About Qt dialog.
    void aboutQt();
    //! Licenses.
    void licenses();
    //! Play/stop.
    void playStop();
    //! Show next frame.
    void showNextFrame();
    //! Switch to text edit mode.
    void onSwitchToTextEditMode();
    //! Switch to text selection rect mode.
    void onSwitchToTextSelectionRectMode();

protected:
    void resizeEvent(QResizeEvent *e) override;

private:
    Q_DISABLE_COPY(MainWindow)

    QScopedPointer<MainWindowPrivate> m_d;
}; // class MainWindow

#endif // GIF_EDITOR_MAINWINDOW_HPP_INCLUDED
