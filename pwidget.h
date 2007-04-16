/* -*- C++ -*-
 *
 * patience -- main program
 *   Copyright (C) 1995  Paul Olav Tvete
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 *
 *
 * Heavily modified by Mario Weilguni <mweilguni@sime.com>
 *
 */

#ifndef __PWIDGET__H__
#define __PWIDGET__H__

#include <kxmlguiwindow.h>
//Added by qt3to4:
#include <QPixmap>
#include <QLabel>
#include <QShowEvent>
#include <krecentfilesaction.h>
class PatienceView;
class KToggleAction;
class KSelectAction;
class KRecentFilesAction;
class QAction;
class QLabel;
class cardMap;

class pWidget: public KXmlGuiWindow {
    Q_OBJECT

public:
    pWidget();
    ~pWidget();

public slots:
    void undoMove();
    void newGameType();
    void slotNewGameType();
    void restart();

    void openGame();
    void openGame(const KUrl &url);
    void saveGame();

    void newGame();
    void chooseGame();
    void undoPossible(bool poss);
    void gameLost();
    void slotGameInfo(const QString &);
    void slotUpdateMoves();
    void helpGame();
    void enableAutoDrop();
    void showStats();

    void slotGameSolverWon();
    void slotGameSolverLost();
    void slotGameSolverUnknown();
    void slotPickRandom();

private:
    void setGameCaption();
    virtual void showEvent(QShowEvent *e);

private:
    // Members

    PatienceView   *dill;

    KSelectAction  *games;
    QAction        *undo;
    KToggleAction  *dropaction;
    QAction        *stats;

    QPixmap         background;
    KRecentFilesAction  *recent;
    cardMap        *m_cards; // possibly move to PatienceView
};

#endif
