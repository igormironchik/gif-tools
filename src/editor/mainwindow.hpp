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

signals:
    void openFileTriggered();
    void fileLoadedTriggered();
    void fileLoadingFailed();
    void saveFileTriggered();
    void fileSavingFailed();
    void applyEditTriggered();
    void cancelEditTriggered();
    void graphicsAppliedTriggered();

public:
    MainWindow();
    ~MainWindow() noexcept override;

public slots:
    //! Open GIF.
    void openFile(const QString &fileName,
                  bool afterShowEvent = false);

private slots:
    //! Hide pen width spin box.
    void hidePenWidthSpinBox();
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
    //! Change pen width.
    void penWidth(bool on);
    //! Draw rectangle.
    void drawRect(bool on);
    //! Draw arrow.
    void drawArrow(bool on);
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
    //! Rectangle selection is started.
    void onRectSelectionStarted();
    //! Apply text.
    void applyText();
    //! Show setting dlg.
    void onSettings();
    //! Change pen color.
    void penColor();
    //! Change brush color.
    void brushColor();
    //! GIF loaded.
    void gifLoaded();
    //! GIF saved.
    void gifSaved();
    //! GIF cropped.
    void gifCropped();
    //! Graphics applied.
    void graphicsApplied();
    //! Frame selected.
    void onFrameSelected(int idx);
    //! Frame changed.
    void onFrameChanged(int idx);

protected:
    void resizeEvent(QResizeEvent *e) override;
    void closeEvent(QCloseEvent *e) override;
    void showEvent(QShowEvent *e) override;

private:
    void initUi();
    void initStateMachine();

private:
    friend class MainWindowPrivate;
    friend class TipsState;
    friend class DrawTextState;
    friend class CropState;

    Q_DISABLE_COPY(MainWindow)

    QScopedPointer<MainWindowPrivate> m_d;
}; // class MainWindow

#endif // GIF_EDITOR_MAINWINDOW_HPP_INCLUDED
