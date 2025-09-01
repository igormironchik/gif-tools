/*
    SPDX-FileCopyrightText: 2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF editor include.
#include "mainwindow.hpp"
#include "about.hpp"
#include "busyindicator.hpp"
#include "frame.hpp"
#include "frameontape.hpp"
#include "tape.hpp"
#include "view.hpp"

// Qt include.
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenuBar>
#include <QMessageBox>
#include <QMetaMethod>
#include <QResizeEvent>
#include <QRunnable>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QThreadPool>
#include <QTimer>
#include <QToolBar>
#include <QVector>

// C++ include.
#include <algorithm>
#include <utility>
#include <vector>

// gif-widgets include.
#include "license_dialog.hpp"

namespace /* anonymous */
{

class ReadGIF final : public QRunnable
{
public:
    ReadGIF(QGifLib::Gif *container,
            const QString &fileName)
        : m_container(container)
        , m_fileName(fileName)
    {
        setAutoDelete(false);
    }

    void run() override
    {
        m_container->load(m_fileName);
    }

private:
    QGifLib::Gif *m_container;
    QString m_fileName;
}; // class ReadGIF

class CropGIF final : public QRunnable
{
public:
    CropGIF(BusyIndicator *receiver,
            QGifLib::Gif *container,
            const QRect &rect)
        : m_container(container)
        , m_rect(rect)
        , m_receiver(receiver)
    {
        setAutoDelete(false);
    }

    void run() override
    {
        const auto index = m_receiver->metaObject()->indexOfProperty("percent");
        auto property = m_receiver->metaObject()->property(index);

        int current = 0;
        const auto count = m_container->fileNames().size();

        property.write(m_receiver, 0);

        for (const auto &fileName : m_container->fileNames()) {
            QImage(fileName).copy(m_rect).save(fileName);
            ++current;
            property.write(m_receiver, qRound(((double)current / (double)count) * 100.0));
        }

        property.write(m_receiver, 100);
    }

private:
    QGifLib::Gif *m_container;
    QRect m_rect;
    BusyIndicator *m_receiver;
}; // class CropGIF

} /* namespace anonymous */

//
// MainWindowPrivate
//

class MainWindowPrivate
{
public:
    MainWindowPrivate(MainWindow *parent)
        : m_editMode(EditMode::Unknow)
        , m_busyFlag(false)
        , m_quitFlag(false)
        , m_playing(false)
        , m_stack(new QStackedWidget(parent))
        , m_busy(new BusyIndicator(m_stack))
        , m_view(new View(m_frames,
                          m_stack))
        , m_about(new About(parent))
        , m_crop(nullptr)
        , m_playStop(nullptr)
        , m_save(nullptr)
        , m_saveAs(nullptr)
        , m_open(nullptr)
        , m_applyEdit(nullptr)
        , m_cancelEdit(nullptr)
        , m_quit(nullptr)
        , m_editToolBar(nullptr)
        , m_q(parent)
    {
        m_busy->setRadius(75);
    }

    //! Edit mode.
    enum class EditMode {
        Unknow,
        Crop
    }; // enum class EditMode

    //! Clear view.
    void clearView();
    //! Enable file actions.
    void enableFileActions(bool on = true)
    {
        m_save->setEnabled(on);
        m_saveAs->setEnabled(on);
        m_open->setEnabled(on);

        m_applyEdit->setEnabled(!on);
        m_cancelEdit->setEnabled(!on);

        m_playStop->setEnabled(on);
    }
    //! Initialize tape.
    void initTape()
    {
        for (qsizetype i = 0, last = m_frames.count(); i < last; ++i) {
            m_view->tape()->addFrame({m_frames, i, false});

            QApplication::processEvents();
        };
    }
    //! Busy state.
    void busy()
    {
        m_busyFlag = true;

        m_stack->setCurrentWidget(m_busy);

        m_busy->setRunning(true);

        m_crop->setEnabled(false);
        m_save->setEnabled(false);
        m_saveAs->setEnabled(false);
        m_open->setEnabled(false);
        m_quit->setEnabled(false);

        m_editToolBar->hide();
    }
    //! Ready state.
    void ready()
    {
        m_busyFlag = false;

        m_stack->setCurrentWidget(m_view);

        m_busy->setRunning(false);

        m_crop->setEnabled(true);

        if (!m_currentGif.isEmpty()) {
            if (m_q->isWindowModified()) {
                m_save->setEnabled(true);
            }

            m_saveAs->setEnabled(true);
        }

        m_open->setEnabled(true);
        m_quit->setEnabled(true);

        m_editToolBar->show();
    }
    //! Wait for thread pool.
    void waitThreadPool()
    {
        while (!QThreadPool::globalInstance()->waitForDone(5)) {
            QApplication::processEvents();
        }
    }
    //! Set modified state.
    void setModified(bool on)
    {
        m_q->setWindowModified(on);

        if (on) {
            m_save->setEnabled(true);
        } else {
            m_save->setEnabled(false);
        }
    }

    //! \return Index of the next checked frame.
    int nextCheckedFrame(int current) const
    {
        for (int i = current + 1; i <= m_view->tape()->count(); ++i) {
            if (m_view->tape()->frame(i)->isChecked()) {
                return i;
            }
        }

        for (int i = 1; i < current; ++i) {
            if (m_view->tape()->frame(i)->isChecked()) {
                return i;
            }
        }

        return -1;
    }

    void openGif(const QString &fileName)
    {
        clearView();

        setModified(false);

        m_currentGif = fileName;

        ReadGIF read(&m_frames, fileName);
        QThreadPool::globalInstance()->start(&read);

        waitThreadPool();

        QFileInfo info(fileName);

        m_q->setWindowTitle(MainWindow::tr("GIF Editor - %1[*]").arg(info.fileName()));

        initTape();

        if (m_frames.count()) {
            m_view->tape()->setCurrentFrame(1);
            m_view->scrollTo(1);
        }

        m_crop->setEnabled(true);
        m_playStop->setEnabled(true);
        m_saveAs->setEnabled(true);
    }

    //! Current file name.
    QString m_currentGif;
    //! Frames.
    QGifLib::Gif m_frames;
    //! Edit mode.
    EditMode m_editMode;
    //! Busy flag.
    bool m_busyFlag;
    //! Quit flag.
    bool m_quitFlag;
    //! Play/stop flag.
    bool m_playing;
    //! Stacked widget.
    QStackedWidget *m_stack;
    //! Busy indicator.
    BusyIndicator *m_busy;
    //! View.
    View *m_view;
    //! Widget about.
    About *m_about;
    //! Crop action.
    QAction *m_crop;
    //! Play/stop action.
    QAction *m_playStop;
    //! Save action.
    QAction *m_save;
    //! Save as action.
    QAction *m_saveAs;
    //! Open action.
    QAction *m_open;
    //! Apply edit action.
    QAction *m_applyEdit;
    //! Cancel edit action.
    QAction *m_cancelEdit;
    //! Quit action.
    QAction *m_quit;
    //! Edit toolbar.
    QToolBar *m_editToolBar;
    //! Play timer.
    QTimer *m_playTimer;
    //! Parent.
    MainWindow *m_q;
}; // class MainWindowPrivate

void MainWindowPrivate::clearView()
{
    m_view->currentFrame()->clearImage();
    m_view->tape()->clear();
    m_frames.clean();
}

//
// MainWindow
//

MainWindow::MainWindow()
    : m_d(new MainWindowPrivate(this))
{
    setWindowTitle(tr("GIF Editor"));

    auto file = menuBar()->addMenu(tr("&File"));
    m_d->m_open = file->addAction(QIcon(QStringLiteral(":/img/document-open.png")),
                                  tr("Open"),
                                  tr("Ctrl+O"),
                                  this,
                                  &MainWindow::openGif);
    file->addSeparator();
    m_d->m_save = file->addAction(QIcon(QStringLiteral(":/img/document-save.png")),
                                  tr("Save"),
                                  tr("Ctrl+S"),
                                  this,
                                  &MainWindow::saveGif);
    m_d->m_saveAs = file->addAction(QIcon(QStringLiteral(":/img/document-save-as.png")),
                                    tr("Save As"),
                                    this,
                                    &MainWindow::saveGifAs);
    file->addSeparator();
    m_d->m_quit = file->addAction(QIcon(QStringLiteral(":/img/application-exit.png")),
                                  tr("Quit"),
                                  tr("Ctrl+Q"),
                                  this,
                                  &MainWindow::quit);

    m_d->m_save->setEnabled(false);
    m_d->m_saveAs->setEnabled(false);

    m_d->m_crop = new QAction(QIcon(QStringLiteral(":/img/transform-crop.png")), tr("Crop"), this);
    m_d->m_crop->setShortcut(tr("Ctrl+C"));
    m_d->m_crop->setShortcutContext(Qt::ApplicationShortcut);
    m_d->m_crop->setCheckable(true);
    m_d->m_crop->setChecked(false);
    m_d->m_crop->setEnabled(false);

    m_d->m_playStop = new QAction(QIcon(QStringLiteral(":/img/media-playback-start.png")), tr("Play"), this);
    m_d->m_playStop->setEnabled(false);

    m_d->m_applyEdit = new QAction(this);
    m_d->m_applyEdit->setShortcut(Qt::Key_Return);
    m_d->m_applyEdit->setShortcutContext(Qt::ApplicationShortcut);
    m_d->m_applyEdit->setEnabled(false);

    m_d->m_cancelEdit = new QAction(this);
    m_d->m_cancelEdit->setShortcut(Qt::Key_Escape);
    m_d->m_cancelEdit->setShortcutContext(Qt::ApplicationShortcut);
    m_d->m_cancelEdit->setEnabled(false);

    addAction(m_d->m_applyEdit);
    addAction(m_d->m_cancelEdit);

    m_d->m_playTimer = new QTimer(this);

    connect(m_d->m_crop, &QAction::triggered, this, &MainWindow::crop);
    connect(m_d->m_playStop, &QAction::triggered, this, &MainWindow::playStop);
    connect(m_d->m_applyEdit, &QAction::triggered, this, &MainWindow::applyEdit);
    connect(m_d->m_cancelEdit, &QAction::triggered, this, &MainWindow::cancelEdit);
    connect(m_d->m_playTimer, &QTimer::timeout, this, &MainWindow::showNextFrame);

    auto edit = menuBar()->addMenu(tr("&Edit"));
    edit->addAction(m_d->m_crop);

    m_d->m_editToolBar = new QToolBar(tr("Tools"), this);
    m_d->m_editToolBar->addAction(m_d->m_playStop);
    m_d->m_editToolBar->addSeparator();
    m_d->m_editToolBar->addAction(m_d->m_crop);

    addToolBar(Qt::LeftToolBarArea, m_d->m_editToolBar);

    m_d->m_editToolBar->hide();

    auto help = menuBar()->addMenu(tr("&Help"));
    help->addAction(QIcon(QStringLiteral(":/icon/icon_22x22.png")), tr("About"), this, &MainWindow::about);
    help->addAction(QIcon(QStringLiteral(":/img/Qt-logo-neon-transparent.png")),
                    tr("About Qt"),
                    this,
                    &MainWindow::aboutQt);
    help->addAction(QIcon(QStringLiteral(":/img/bookmarks-organize.png")), tr("Licenses"), this, &MainWindow::licenses);

    m_d->m_stack->addWidget(m_d->m_about);
    m_d->m_stack->addWidget(m_d->m_view);
    m_d->m_stack->addWidget(m_d->m_busy);

    setCentralWidget(m_d->m_stack);

    connect(m_d->m_view->tape(), &Tape::checkStateChanged, this, &MainWindow::frameChecked);
}

MainWindow::~MainWindow() noexcept
{
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if (m_d->m_busyFlag) {
        const auto btn = QMessageBox::question(this,
                                               tr("GIF editor is busy..."),
                                               tr("GIF editor is busy.\nDo you want to terminate the application?"));

        if (btn == QMessageBox::Yes) {
            exit(-1);
        } else {
            e->ignore();
        }
    } else {
        e->accept();
    }

    quit();
}

void MainWindow::openGif()
{
    static const auto pictureLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);

    const auto fileName =
        QFileDialog::getOpenFileName(this,
                                     tr("Open GIF..."),
                                     (!pictureLocations.isEmpty() ? pictureLocations.first() : QString()),
                                     tr("GIF (*.gif)"));

    if (!fileName.isEmpty()) {
        if (isWindowModified()) {
            const auto btn = QMessageBox::question(this,
                                                   tr("GIF was changed..."),
                                                   tr("\"%1\" was changed.\n"
                                                      "Do you want to save it?")
                                                       .arg(fileName));

            if (btn == QMessageBox::Yes) {
                saveGif();
            }
        }

        m_d->busy();

        m_d->openGif(fileName);

        m_d->ready();
    }
}

namespace /* anonymous */
{

class WriteGIF final : public QRunnable
{
public:
    WriteGIF(BusyIndicator *receiver,
             const QStringList &files,
             const QVector<int> &delays,
             const QString &fileName)
        : m_files(files)
        , m_delays(delays)
        , m_fileName(fileName)
        , m_receiver(receiver)
    {
        setAutoDelete(false);
    }

    void run() override
    {
        QGifLib::Gif gif;

        QObject::connect(&gif, &QGifLib::Gif::writeProgress, m_receiver, &BusyIndicator::setPercent);

        gif.write(m_fileName, m_files, m_delays, 0);
    }

private:
    const QStringList &m_files;
    const QVector<int> &m_delays;
    QString m_fileName;
    BusyIndicator *m_receiver;
}; // class WriteGIF

} /* namespace anonymous */

void MainWindow::saveGif()
{
    try {
        m_d->busy();

        QStringList toSave;
        QVector<int> delays;
        const auto allFiles = m_d->m_frames.fileNames();

        for (int i = 0; i < m_d->m_view->tape()->count(); ++i) {
            if (m_d->m_view->tape()->frame(i + 1)->isChecked()) {
                toSave.push_back(allFiles.at(i));
                delays.push_back(m_d->m_frames.delay(i));
            }
        }

        if (!toSave.empty()) {
            m_d->m_busy->setShowPercent(true);

            WriteGIF runnable(m_d->m_busy, toSave, delays, m_d->m_currentGif);
            QThreadPool::globalInstance()->start(&runnable);

            m_d->waitThreadPool();

            m_d->m_busy->setShowPercent(false);

            m_d->openGif(m_d->m_currentGif);
        } else {
            QMessageBox::information(this, tr("Can't save GIF..."), tr("Can't save GIF image with no frames."));
        }

        m_d->ready();
    } catch (const std::bad_alloc &) {
        m_d->ready();

        QMessageBox::critical(this, tr("Failed to save GIF..."), tr("Out of memory."));
    }
}

void MainWindow::saveGifAs()
{
    auto fileName = QFileDialog::getSaveFileName(this, tr("Choose file to save to..."), QString(), tr("GIF (*.gif)"));

    if (!fileName.isEmpty()) {
        if (!fileName.endsWith(QStringLiteral(".gif"), Qt::CaseInsensitive))
            fileName.append(QStringLiteral(".gif"));

        m_d->m_currentGif = fileName;

        QFileInfo info(fileName);

        setWindowTitle(tr("GIF Editor - %1[*]").arg(info.fileName()));

        saveGif();
    }
}

void MainWindow::quit()
{
    if (!m_d->m_busyFlag && !m_d->m_quitFlag) {
        if (isWindowModified()) {
            auto btn = QMessageBox::question(this,
                                             tr("GIF was changed..."),
                                             tr("GIF was changed. Do you want to save changes?"));

            if (btn == QMessageBox::Yes) {
                saveGif();
            }
        }

        m_d->m_quitFlag = true;

        QApplication::quit();
    }
}

void MainWindow::frameChecked(int,
                              bool)
{
    m_d->setModified(true);
}

void MainWindow::crop(bool on)
{
    if (on) {
        m_d->enableFileActions(false);

        m_d->m_editMode = MainWindowPrivate::EditMode::Crop;

        m_d->m_view->startCrop();
    } else {
        m_d->m_view->stopCrop();

        m_d->m_editMode = MainWindowPrivate::EditMode::Unknow;

        m_d->enableFileActions();
    }
}

void MainWindow::cancelEdit()
{
    switch (m_d->m_editMode) {
    case MainWindowPrivate::EditMode::Crop: {
        m_d->m_view->stopCrop();

        m_d->enableFileActions();

        m_d->m_crop->setChecked(false);

        m_d->m_editMode = MainWindowPrivate::EditMode::Unknow;

        QApplication::processEvents();
    } break;

    default:
        break;
    }
}

void MainWindow::applyEdit()
{
    switch (m_d->m_editMode) {
    case MainWindowPrivate::EditMode::Crop: {
        const auto rect = m_d->m_view->cropRect();

        if (!rect.isNull() && rect != m_d->m_view->currentFrame()->imageRect()) {
            m_d->busy();

            QVector<int> unchecked;

            for (int i = 1; i <= m_d->m_view->tape()->count(); ++i) {
                if (!m_d->m_view->tape()->frame(i)->isChecked()) {
                    unchecked.append(i);
                }
            }

            QApplication::processEvents();

            m_d->m_busy->setShowPercent(true);

            CropGIF crop(m_d->m_busy, &m_d->m_frames, rect);
            QThreadPool::globalInstance()->start(&crop);

            m_d->waitThreadPool();

            m_d->m_busy->setShowPercent(false);

            const auto current = m_d->m_view->tape()->currentFrame()->counter();
            m_d->m_view->tape()->clear();

            QApplication::processEvents();

            m_d->initTape();

            m_d->m_view->tape()->setCurrentFrame(current);

            for (const auto &i : std::as_const(unchecked)) {
                m_d->m_view->tape()->frame(i)->setChecked(false);
            }

            m_d->setModified(true);

            cancelEdit();

            m_d->ready();
        } else {
            cancelEdit();
        }
    } break;

    default:
        break;
    }
}

void MainWindow::about()
{
    QMessageBox::about(this,
                       tr("About GIF editor"),
                       tr("GIF editor.\n\n"
                          "Author - Igor Mironchik (igor.mironchik at gmail dot com).\n\n"
                          "Copyright (c) 2025 Igor Mironchik.\n\n"
                          "Licensed under GNU GPL 3.0."));
}

void MainWindow::aboutQt()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::licenses()
{
    LicenseDialog msg(this);
    msg.addLicense(
        QStringLiteral("The Oxygen Icon Theme"),
        QStringLiteral("<p><b>The Oxygen Icon Theme</b>\n\n</p>"
                       "<p>Copyright (C) 2007 Nuno Pinheiro &lt;nuno@oxygen-icons.org&gt;\n</p>"
                       "<p>Copyright (C) 2007 David Vignoni &lt;david@icon-king.com&gt;\n</p>"
                       "<p>Copyright (C) 2007 David Miller &lt;miller@oxygen-icons.org&gt;\n</p>"
                       "<p>Copyright (C) 2007 Johann Ollivier Lapeyre &lt;johann@oxygen-icons.org&gt;\n</p>"
                       "<p>Copyright (C) 2007 Kenneth Wimer &lt;kwwii@bootsplash.org&gt;\n</p>"
                       "<p>Copyright (C) 2007 Riccardo Iaconelli &lt;riccardo@oxygen-icons.org&gt;\n</p>"
                       "<p>\nand others\n</p>"
                       "\n"
                       "<p>This library is free software; you can redistribute it and/or "
                       "modify it under the terms of the GNU Lesser General Public "
                       "License as published by the Free Software Foundation; either "
                       "version 3 of the License, or (at your option) any later version.\n</p>"
                       "\n"
                       "<p>This library is distributed in the hope that it will be useful, "
                       "but WITHOUT ANY WARRANTY; without even the implied warranty of "
                       "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU "
                       "Lesser General Public License for more details.\n</p>"
                       "\n"
                       "<p>You should have received a copy of the GNU Lesser General Public "
                       "License along with this library. If not, see "
                       "<a href=\"http://www.gnu.org/licenses/\">&lt;http://www.gnu.org/licenses/&gt;</a>.\n</p>"
                       "\n"
                       "<p>Clarification:\n</p>"
                       "\n"
                       "<p>The GNU Lesser General Public License or LGPL is written for "
                       "software libraries in the first place. We expressly want the LGPL to "
                       "be valid for this artwork library too.\n</p>"
                       "\n"
                       "<p>KDE Oxygen theme icons is a special kind of software library, it is an "
                       "artwork library, it's elements can be used in a Graphical User Interface, or "
                       "GUI.\n</p>"
                       "\n"
                       "<p>Source code, for this library means:\n</p>"
                       "<p><ul> <li>where they exist, SVG;\n</li>"
                       " <li>otherwise, if applicable, the multi-layered formats xcf or psd, or "
                       "otherwise png.\n</li></ul></p>"
                       "\n"
                       "<p>The LGPL in some sections obliges you to make the files carry "
                       "notices. With images this is in some cases impossible or hardly useful.\n</p>"
                       "\n"
                       "<p>With this library a notice is placed at a prominent place in the directory "
                       "containing the elements. You may follow this practice.\n</p>"
                       "\n"
                       "<p>The exception in section 5 of the GNU Lesser General Public License covers "
                       "the use of elements of this art library in a GUI.\n</p>"
                       "\n"
                       "<p>kde-artists [at] kde.org\n</p>"
                       "\n"
                       "<p><b>GNU LESSER GENERAL PUBLIC LICENSE</b>\n</p>"
                       "<p>Version 3, 29 June 2007\n</p>"
                       "\n"
                       "<p>Copyright (C) 2007 Free Software Foundation, Inc. <a "
                       "href=\"http://fsf.org/\">&lt;http://fsf.org/&gt;</a> "
                       "Everyone is permitted to copy and distribute verbatim copies "
                       "of this license document, but changing it is not allowed.\n</p>"
                       "\n"
                       "\n"
                       "<p>This version of the GNU Lesser General Public License incorporates "
                       "the terms and conditions of version 3 of the GNU General Public "
                       "License, supplemented by the additional permissions listed below.\n</p>"
                       "\n"
                       "<p><b>0.</b> Additional Definitions.\n</p>"
                       "\n"
                       "<p>As used herein, \"this License\" refers to version 3 of the GNU Lesser "
                       "General Public License, and the \"GNU GPL\" refers to version 3 of the GNU "
                       "General Public License.\n</p>"
                       "\n"
                       "<p>\"The Library\" refers to a covered work governed by this License, "
                       "other than an Application or a Combined Work as defined below.\n</p>"
                       "\n"
                       "<p>An \"Application\" is any work that makes use of an interface provided "
                       "by the Library, but which is not otherwise based on the Library. "
                       "Defining a subclass of a class defined by the Library is deemed a mode "
                       "of using an interface provided by the Library.\n</p>"
                       "\n"
                       "<p>A \"Combined Work\" is a work produced by combining or linking an "
                       "Application with the Library.  The particular version of the Library "
                       "with which the Combined Work was made is also called the \"Linked "
                       "Version\".\n</p>"
                       "\n"
                       "<p>The \"Minimal Corresponding Source\" for a Combined Work means the "
                       "Corresponding Source for the Combined Work, excluding any source code "
                       "for portions of the Combined Work that, considered in isolation, are "
                       "based on the Application, and not on the Linked Version.\n</p>"
                       "\n"
                       "<p>The \"Corresponding Application Code\" for a Combined Work means the "
                       "object code and/or source code for the Application, including any data "
                       "and utility programs needed for reproducing the Combined Work from the "
                       "Application, but excluding the System Libraries of the Combined Work.\n</p>"
                       "\n"
                       "<p><b>1.</b> Exception to Section 3 of the GNU GPL.\n</p>"
                       "\n"
                       "<p>You may convey a covered work under sections 3 and 4 of this License "
                       "without being bound by section 3 of the GNU GPL.\n</p>"
                       "\n"
                       "<p><b>2.</b> Conveying Modified Versions.\n</p>"
                       "\n"
                       "<p>If you modify a copy of the Library, and, in your modifications, a "
                       "facility refers to a function or data to be supplied by an Application "
                       "that uses the facility (other than as an argument passed when the "
                       "facility is invoked), then you may convey a copy of the modified "
                       "version:\n</p>"
                       "\n"
                       "<p><b>a)</b> under this License, provided that you make a good faith effort to "
                       "ensure that, in the event an Application does not supply the "
                       "function or data, the facility still operates, and performs "
                       "whatever part of its purpose remains meaningful, or\n</p>"
                       "\n"
                       "<p><b>b)</b> under the GNU GPL, with none of the additional permissions of "
                       "this License applicable to that copy.\n</p>"
                       "\n"
                       "<p><b>3.</b> Object Code Incorporating Material from Library Header Files.\n</p>"
                       "\n"
                       "<p>The object code form of an Application may incorporate material from "
                       "a header file that is part of the Library.  You may convey such object "
                       "code under terms of your choice, provided that, if the incorporated "
                       "material is not limited to numerical parameters, data structure "
                       "layouts and accessors, or small macros, inline functions and templates "
                       "(ten or fewer lines in length), you do both of the following:\n</p>"
                       "\n"
                       "<p><b>a)</b> Give prominent notice with each copy of the object code that the "
                       "Library is used in it and that the Library and its use are "
                       "covered by this License.\n</p>"
                       "\n"
                       "<p><b>b)</b> Accompany the object code with a copy of the GNU GPL and this license "
                       "document.\n</p>"
                       "\n"
                       "<p><b>4.</b> Combined Works.\n</p>"
                       "\n"
                       "<p>You may convey a Combined Work under terms of your choice that, "
                       "taken together, effectively do not restrict modification of the "
                       "portions of the Library contained in the Combined Work and reverse "
                       "engineering for debugging such modifications, if you also do each of "
                       "the following:\n</p>"
                       "\n"
                       "<p><b>a)</b> Give prominent notice with each copy of the Combined Work that "
                       "the Library is used in it and that the Library and its use are "
                       "covered by this License.\n</p>"
                       "\n"
                       "<p><b>b)</b> Accompany the Combined Work with a copy of the GNU GPL and this license "
                       "document.\n</p>"
                       "\n"
                       "<p><b>c)</b> For a Combined Work that displays copyright notices during "
                       "execution, include the copyright notice for the Library among "
                       "these notices, as well as a reference directing the user to the "
                       "copies of the GNU GPL and this license document.\n</p>"
                       "\n"
                       "<p><b>d)</b> Do one of the following:\n</p>"
                       "\n"
                       "<p>    <b>0)</b> Convey the Minimal Corresponding Source under the terms of this "
                       "License, and the Corresponding Application Code in a form "
                       "suitable for, and under terms that permit, the user to "
                       "recombine or relink the Application with a modified version of "
                       "the Linked Version to produce a modified Combined Work, in the "
                       "manner specified by section 6 of the GNU GPL for conveying "
                       "Corresponding Source.\n</p>"
                       "\n"
                       "<p>    <b>1)</b> Use a suitable shared library mechanism for linking with the "
                       "Library.  A suitable mechanism is one that (a) uses at run time "
                       "a copy of the Library already present on the user's computer "
                       "system, and (b) will operate properly with a modified version "
                       "of the Library that is interface-compatible with the Linked "
                       "Version.\n</p>"
                       "\n"
                       "<p><b>e)</b> Provide Installation Information, but only if you would otherwise "
                       "be required to provide such information under section 6 of the "
                       "GNU GPL, and only to the extent that such information is "
                       "necessary to install and execute a modified version of the "
                       "Combined Work produced by recombining or relinking the "
                       "Application with a modified version of the Linked Version. (If "
                       "you use option 4d0, the Installation Information must accompany "
                       "the Minimal Corresponding Source and Corresponding Application "
                       "Code. If you use option 4d1, you must provide the Installation "
                       "Information in the manner specified by section 6 of the GNU GPL "
                       "for conveying Corresponding Source.)\n</p>"
                       "\n"
                       "<p><b>5.</b> Combined Libraries.\n</p>"
                       "\n"
                       "<p>You may place library facilities that are a work based on the "
                       "Library side by side in a single library together with other library "
                       "facilities that are not Applications and are not covered by this "
                       "License, and convey such a combined library under terms of your "
                       "choice, if you do both of the following:\n</p>"
                       "\n"
                       "<p><b>a)</b> Accompany the combined library with a copy of the same work based "
                       "on the Library, uncombined with any other library facilities, "
                       "conveyed under the terms of this License.\n</p>"
                       "\n"
                       "<p><b>b)</b> Give prominent notice with the combined library that part of it "
                       "is a work based on the Library, and explaining where to find the "
                       "accompanying uncombined form of the same work.\n</p>"
                       "\n"
                       "<p><b>6.</b> Revised Versions of the GNU Lesser General Public License.\n</p>"
                       "\n"
                       "<p>The Free Software Foundation may publish revised and/or new versions "
                       "of the GNU Lesser General Public License from time to time. Such new "
                       "versions will be similar in spirit to the present version, but may "
                       "differ in detail to address new problems or concerns.\n</p>"
                       "\n"
                       "<p>Each version is given a distinguishing version number. If the "
                       "Library as you received it specifies that a certain numbered version "
                       "of the GNU Lesser General Public License \"or any later version\" "
                       "applies to it, you have the option of following the terms and "
                       "conditions either of that published version or of any later version "
                       "published by the Free Software Foundation. If the Library as you "
                       "received it does not specify a version number of the GNU Lesser "
                       "General Public License, you may choose any version of the GNU Lesser "
                       "General Public License ever published by the Free Software Foundation.\n</p>"
                       "\n"
                       "<p>If the Library as you received it specifies that a proxy can decide "
                       "whether future versions of the GNU Lesser General Public License shall "
                       "apply, that proxy's public statement of acceptance of any version is "
                       "permanent authorization for you to choose that version for the "
                       "Library.</p>"));
    msg.addLicense(
        QStringLiteral("ImageMagick"),
        QStringLiteral(
            "<p><b>ImageMagick License</b>\n\n</p>"
            "<p>Terms and Conditions for Use, Reproduction, and Distribution\n</p>"
            "\n"
            "<p>The legally binding and authoritative terms and conditions for use, reproduction, "
            "and distribution of ImageMagick follow:\n</p>"
            "\n"
            "<p>Copyright (c) 1999-2021 ImageMagick Studio LLC, a non-profit organization dedicated "
            "to making software imaging solutions freely available.\n</p>"
            "\n"
            "<p><b>1.</b> Definitions.\n</p>"
            "\n"
            "<p>License shall mean the terms and conditions for use, reproduction, and distribution as "
            "defined by Sections 1 through 9 of this document.\n</p>"
            "\n"
            "<p>Licensor shall mean the copyright owner or entity authorized by the copyright owner "
            "that is granting the License.\n</p>"
            "\n"
            "<p>Legal Entity shall mean the union of the acting entity and all other entities that "
            "control, are controlled by, or are under common control with that entity. For the "
            "purposes of this definition, control means (i) the power, direct or indirect, to cause "
            "the direction or management of such entity, whether by contract or otherwise, or (ii) "
            "ownership of fifty percent (50%) or more of the outstanding shares, or (iii) beneficial "
            "ownership of such entity.\n</p>"
            "\n"
            "<p>You (or Your) shall mean an individual or Legal Entity exercising permissions granted "
            "by this License.\n</p>"
            "\n"
            "<p>Source form shall mean the preferred form for making modifications, including but not "
            "limited to software source code, documentation source, and configuration files.\n</p>"
            "\n"
            "<p>Object form shall mean any form resulting from mechanical transformation or translation "
            "of a Source form, including but not limited to compiled object code, generated "
            "documentation, and conversions to other media types.\n</p>"
            "\n"
            "<p>Work shall mean the work of authorship, whether in Source or Object form, made available "
            "under the License, as indicated by a copyright notice that is included in or attached to "
            "the work (an example is provided in the Appendix below).\n</p>"
            "\n"
            "<p>Derivative Works shall mean any work, whether in Source or Object form, that is based on "
            "(or derived from) the Work and for which the editorial revisions, annotations, "
            "elaborations, or other modifications represent, as a whole, an original work of "
            "authorship. For the purposes of this License, Derivative Works shall not include works "
            "that remain separable from, or merely link (or bind by name) to the interfaces of, the "
            "Work and Derivative Works thereof.\n</p>"
            "\n"
            "<p>Contribution shall mean any work of authorship, including the original version of the "
            "Work and any modifications or additions to that Work or Derivative Works thereof, that is "
            "intentionally submitted to Licensor for inclusion in the Work by the copyright owner or "
            "by an individual or Legal Entity authorized to submit on behalf of the copyright owner. "
            "For the purposes of this definition, \"submitted\" means any form of electronic, verbal, "
            "or written communication sent to the Licensor or its representatives, including but not "
            "limited to communication on electronic mailing lists, source code control systems, and "
            "issue tracking systems that are managed by, or on behalf of, the Licensor for the purpose "
            "of discussing and improving the Work, but excluding communication that is conspicuously "
            "marked or otherwise designated in writing by the copyright owner as Not a Contribution.\n</p>"
            "\n"
            "<p>Contributor shall mean Licensor and any individual or Legal Entity on behalf of whom a "
            "Contribution has been received by Licensor and subsequently incorporated within the Work.\n</p>"
            "\n"
            "<p><b>2.</b> Grant of Copyright License. Subject to the terms and conditions of this License, each "
            "Contributor hereby grants to You a perpetual, worldwide, non-exclusive, no-charge, "
            "royalty-free, irrevocable copyright license to reproduce, prepare Derivative Works of, "
            "publicly display, publicly perform, sublicense, and distribute the Work and such "
            "Derivative Works in Source or Object form.\n</p>"
            "\n"
            "<p><b>3.</b> Grant of Patent License. Subject to the terms and conditions of this License, each "
            "Contributor hereby grants to You a perpetual, worldwide, non-exclusive, no-charge, "
            "royalty-free, irrevocable (except as stated in this section) patent license to make, have "
            "made, use, offer to sell, sell, import, and otherwise transfer the Work, where such "
            "license applies only to those patent claims licensable by such Contributor that are "
            "necessarily infringed by their Contribution(s) alone or by combination of their "
            "Contribution(s) with the Work to which such Contribution(s) was submitted. If You "
            "institute patent litigation against any entity (including a cross-claim or counterclaim "
            "in a lawsuit) alleging that the Work or a Contribution incorporated within the Work "
            "constitutes direct or contributory patent infringement, then any patent licenses granted "
            "to You under this License for that Work shall terminate as of the date such litigation "
            "is filed.\n</p>"
            "\n"
            "<p><b>4.</b> Redistribution. You may reproduce and distribute copies of the Work or Derivative "
            "Works thereof in any medium, with or without modifications, and in Source or Object "
            "form, provided that You meet the following conditions:\n</p>"
            "\n"
            "<p> <b>a.</b> You must give any other recipients of the Work or Derivative Works a copy of this "
            "License; and\n</p>"
            "<p> <b>b.</b> You must cause any modified files to carry prominent notices stating that You changed "
            "the files; and\n</p>"
            "<p> <b>c.</b> You must retain, in the Source form of any Derivative Works that You distribute, all "
            "copyright, patent, trademark, and attribution notices from the Source form of the Work, "
            "excluding those notices that do not pertain to any part of the Derivative Works; and\n</p>"
            "<p> <b>d.</b> If the Work includes a \"NOTICE\" text file as part of its distribution, then any "
            "Derivative Works that You distribute must include a readable copy of the attribution "
            "notices contained within such NOTICE file, excluding those notices that do not pertain "
            "to any part of the Derivative Works, in at least one of the following places: within a "
            "NOTICE text file distributed as part of the Derivative Works; within the Source form or "
            "documentation, if provided along with the Derivative Works; or, within a display "
            "generated by the Derivative Works, if and wherever such third-party notices normally "
            "appear. The contents of the NOTICE file are for informational purposes only and do not "
            "modify the License. You may add Your own attribution notices within Derivative Works "
            "that You distribute, alongside or as an addendum to the NOTICE text from the Work, "
            "provided that such additional attribution notices cannot be construed as modifying the "
            "License.\n</p>"
            "\n"
            "<p>You may add Your own copyright statement to Your modifications and may provide additional "
            "or different license terms and conditions for use, reproduction, or distribution of Your "
            "modifications, or for any such Derivative Works as a whole, provided Your use, "
            "reproduction, and distribution of the Work otherwise complies with the conditions stated "
            "in this License.\n</p>"
            "\n"
            "<p><b>5.</b> Submission of Contributions. Unless You explicitly state otherwise, any Contribution "
            "intentionally submitted for inclusion in the Work by You to the Licensor shall be under "
            "the terms and conditions of this License, without any additional terms or conditions. "
            "Notwithstanding the above, nothing herein shall supersede or modify the terms of any "
            "separate license agreement you may have executed with Licensor regarding such Contributions.\n</p>"
            "\n"
            "<p><b>6.</b> Trademarks. This License does not grant permission to use the trade names, trademarks, "
            "service marks, or product names of the Licensor, except as required for reasonable and "
            "customary use in describing the origin of the Work and reproducing the content of the "
            "NOTICE file.\n</p>"
            "\n"
            "<p><b>7.</b> Disclaimer of Warranty. Unless required by applicable law or agreed to in writing, "
            "Licensor provides the Work (and each Contributor provides its Contributions) on an AS IS "
            "BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied, "
            "including, without limitation, any warranties or conditions of TITLE, NON-INFRINGEMENT, "
            "MERCHANTABILITY, or FITNESS FOR A PARTICULAR PURPOSE. You are solely responsible for "
            "determining the appropriateness of using or redistributing the Work and assume any risks "
            "associated with Your exercise of permissions under this License.\n</p>"
            "\n"
            "<p><b>8.</b> Limitation of Liability. In no event and under no legal theory, whether in tort "
            "(including negligence), contract, or otherwise, unless required by applicable law (such "
            "as deliberate and grossly negligent acts) or agreed to in writing, shall any Contributor "
            "be liable to You for damages, including any direct, indirect, special, incidental, or "
            "consequential damages of any character arising as a result of this License or out of the "
            "use or inability to use the Work (including but not limited to damages for loss of "
            "goodwill, work stoppage, computer failure or malfunction, or any and all other "
            "commercial damages or losses), even if such Contributor has been advised of the "
            "possibility of such damages.\n</p>"
            "\n"
            "<p><b>9.</b> Accepting Warranty or Additional Liability. While redistributing the Work or "
            "Derivative Works thereof, You may choose to offer, and charge a fee for, acceptance of "
            "support, warranty, indemnity, or other liability obligations and/or rights consistent "
            "with this License. However, in accepting such obligations, You may act only on Your own "
            "behalf and on Your sole responsibility, not on behalf of any other Contributor, and only "
            "if You agree to indemnify, defend, and hold each Contributor harmless for any liability "
            "incurred by, or claims asserted against, such Contributor by reason of your accepting "
            "any such warranty or additional liability.\n</p>"
            "\n"
            "<p>How to Apply the License to your Work\n</p>"
            "\n"
            "<p>To apply the ImageMagick License to your work, attach the following boilerplate notice, "
            "with the fields enclosed by brackets \"[]\" replaced with your own identifying "
            "information (don't include the brackets). The text should be enclosed in the appropriate "
            "comment syntax for the file format. We also recommend that a file or class name and "
            "description of purpose be included on the same \"printed page\" as the copyright notice "
            "for easier identification within third-party archives.\n</p>"
            "\n"
            "<p>    Copyright [yyyy] [name of copyright owner]\n</p>"
            "\n"
            "<p>    Licensed under the ImageMagick License (the \"License\"); you may not use\n"
            "    this file except in compliance with the License.  You may obtain a copy\n"
            "    of the License at\n</p>"
            "\n"
            "<p>    <a "
            "href=\"https://imagemagick.org/script/license.php\">https://imagemagick.org/script/license.php</a>\n</p>"
            "\n"
            "<p>    Unless required by applicable law or agreed to in writing, software\n"
            "    distributed under the License is distributed on an \"AS IS\" BASIS, WITHOUT\n"
            "    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the\n"
            "    License for the specific language governing permissions and limitations\n"
            "    under the License.</p>"));
    msg.addLicense(QStringLiteral("giflib"),
                   QStringLiteral("<p><b>giflib License</b></p>\n\n"
                                  "<p>The GIFLIB distribution is Copyright (c) 1997  Eric S. Raymond</p>\n\n"
                                  "<p>Permission is hereby granted, free of charge, to any person obtaining a copy "
                                  "of this software and associated documentation files (the \"Software\"), to deal "
                                  "in the Software without restriction, including without limitation the rights "
                                  "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell "
                                  "copies of the Software, and to permit persons to whom the Software is "
                                  "furnished to do so, subject to the following conditions:</p>\n\n"
                                  "<p>The above copyright notice and this permission notice shall be included in "
                                  "all copies or substantial portions of the Software.</p>\n\n"
                                  "<p>THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR "
                                  "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, "
                                  "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE "
                                  "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER "
                                  "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, "
                                  "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN "
                                  "THE SOFTWARE.</p>"));

    msg.exec();
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    static bool tapeHeightInit = false;

    if (!tapeHeightInit && m_d->m_stack->currentWidget() != m_d->m_view) {
        tapeHeightInit = true;

        m_d->m_view->resize(800, 600);

        QApplication::processEvents();

        m_d->m_view->tape()->setMinimumHeight(m_d->m_view->tape()->height());
    }

    e->accept();
}

void MainWindow::playStop()
{
    if (m_d->m_playing) {
        m_d->m_playTimer->stop();
        m_d->m_playStop->setText(tr("Play"));
        m_d->m_playStop->setIcon(QIcon(":/img/media-playback-start.png"));
    } else {
        m_d->m_playStop->setText(tr("Stop"));
        m_d->m_playStop->setIcon(QIcon(":/img/media-playback-stop.png"));
        const auto &img = m_d->m_view->tape()->currentFrame()->image();
        m_d->m_playTimer->start(static_cast<int>(img.m_gif.delay(img.m_pos)));
    }

    m_d->m_playing = !m_d->m_playing;
}

void MainWindow::showNextFrame()
{
    const auto next = m_d->nextCheckedFrame(m_d->m_view->tape()->currentFrame()->counter());

    if (next != -1) {
        const auto nextDelay = m_d->nextCheckedFrame(next);

        if (nextDelay != -1) {
            const auto &img = m_d->m_view->tape()->frame(nextDelay)->image();
            m_d->m_playTimer->start(m_d->m_frames.delay(img.m_pos));
        }

        m_d->m_view->tape()->setCurrentFrame(next);
        m_d->m_view->scrollTo(next);
    }
}
