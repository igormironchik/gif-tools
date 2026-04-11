/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// GIF editor include.
#include "about.hpp"
#include "busyindicator.hpp"
#include "tips.hpp"
#include "view.hpp"

// Qt include.
#include <QAction>
#include <QDir>
#include <QFutureWatcher>
#include <QLabel>
#include <QSpinBox>
#include <QStackedWidget>
#include <QToolBar>
#include <QToolButton>

class MainWindow;

//
// MainWindowPrivate
//

class MainWindowPrivate
{
public:
    MainWindowPrivate(MainWindow *parent);

    //! Edit mode.
    enum class EditMode {
        Unknow,
        Crop,
        Text,
        Rect,
        Arrow
    }; // enum class EditMode

    //! Clear view.
    void clearView();
    //! Enable/disable actions during editing.
    void enableActionsOnEdit(bool on = true);
    //! Initialize tape.
    void initTape();
    //! Set state of "Save" action.
    void setSaveAction();
    //! Enable actions.
    void enableActions();
    //! Disable actions on playing.
    void disableActionsOnPlaying();
    //! Busy state.
    void busy();
    //! Cancel tips & tricks page if it's.
    void cancelTips(bool restoreWidget);
    //! Ready state.
    void ready();
    //! Set modified state.
    void setModified(bool on);
    //! \return Index of the next checked frame.
    int nextCheckedFrame(int current) const;
    //! Open file.
    void openGif(const QString &fileName);
    //! Calculate timings.
    void calculateTimings();
    //! Set actions to initial state.
    void setActionsToInitialState();

    //! Current file name.
    QString m_currentGif;
    //! Total duration of the GIF.
    QString m_totalDuration;
    //! Frames.
    QGifLib::Gif m_frames;
    //! Timings.
    QVector<int> m_timings;
    //! Edit mode.
    EditMode m_editMode;
    //! Busy flag.
    bool m_busyFlag;
    //! Quit flag.
    bool m_quitFlag;
    //! Play/stop flag.
    bool m_playing;
    //! Was show evemt?
    bool m_shownAlready = false;

    //! State of the UI.
    struct State {
        bool m_isEditActionsEnabled = false;
        bool m_isEditToolBarShown = false;
        bool m_isTextToolBarShown = false;
        bool m_isDrawToolBarShow = false;
        bool m_isDrawArrowToolBarShow = false;

        //! Current widget on stack.
        QWidget *m_currentStackWidget = nullptr;
    };

    //! Current state of the UI. (Uses for tips only)
    State m_currentUiState;

    //! File name to open after show event.
    QString m_fileNameToOpenAfterShow;
    //! Future watcher.
    QFutureWatcher<void> m_watcher;
    //! Read GIF future watcher.
    QFutureWatcher<bool> m_readWatcher;
    //! Unchecked frames.
    QVector<qsizetype> m_unchecked;
    //! Stacked widget.
    QStackedWidget *m_stack = nullptr;
    //! Page with busy animation.
    QWidget *m_busyPage = nullptr;
    //! Busy indicator.
    BusyIndicator *m_busy = nullptr;
    //! Busy status label.
    QLabel *m_busyStatusLabel = nullptr;
    //! View.
    View *m_view = nullptr;
    //! Widget about.
    About *m_about = nullptr;
    //! Tips & tricks widget.
    Tips *m_tips = nullptr;
    //! Crop action.
    QAction *m_crop = nullptr;
    //! Insert text action.
    QAction *m_insertText = nullptr;
    //! Draw rect.
    QAction *m_drawRect = nullptr;
    //! Draw arrow.
    QAction *m_drawArrow = nullptr;
    //! Play/stop action.
    QAction *m_playStop = nullptr;
    //! Save action.
    QAction *m_save = nullptr;
    //! Save as action.
    QAction *m_saveAs = nullptr;
    //! Open action.
    QAction *m_open = nullptr;
    //! Apply edit action.
    QAction *m_applyEdit = nullptr;
    //! Cancel edit action.
    QAction *m_cancelEdit = nullptr;
    //! Cancel tips.
    QAction *m_cancelTips = nullptr;
    //! Quit action.
    QAction *m_quit = nullptr;
    //! Bold text action.
    QAction *m_boldText = nullptr;
    //! Italic text.
    QAction *m_italicText = nullptr;
    //! Font less.
    QAction *m_fontLess = nullptr;
    //! Font more.
    QAction *m_fontMore = nullptr;
    //! Text color.
    QAction *m_textColor = nullptr;
    //! Clear text format.
    QAction *m_clearFormat = nullptr;
    //! Show previous.
    QAction *m_finishText = nullptr;
    //! Pen color.
    QAction *m_penColor = nullptr;
    //! Brush color.
    QAction *m_brushColor = nullptr;
    //! Pen width.
    QAction *m_penWidth = nullptr;
    //! Tips & Tricks action.
    QAction *m_tipsAction = nullptr;
    //! Edit toolbar.
    QToolBar *m_editToolBar = nullptr;
    //! Text toolbar.
    QToolBar *m_textToolBar = nullptr;
    //! Draw toolbar.
    QToolBar *m_drawToolBar = nullptr;
    //! Draw arror toolbar.
    QToolBar *m_drawArrowToolBar = nullptr;
    //! Play timer.
    QTimer *m_playTimer = nullptr;
    //! Pen width box.
    QSpinBox *m_penWidthBox = nullptr;
    //! Pen width tool button on draw tool bar.
    QToolButton *m_penWidthBtnOnDrawToolBar = nullptr;
    //! Pen width tool button on draw arrow tool bar.
    QToolButton *m_penWidthBtnOnDrawArrowToolBar = nullptr;
    //! Status bar label.
    QLabel *m_status = nullptr;
    //! Edit menu.
    QMenu *m_editMenu = nullptr;
    //! Parent.
    MainWindow *m_q = nullptr;
}; // class MainWindowPrivate
