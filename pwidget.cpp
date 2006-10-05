/*
   patience -- main program
     Copyright (C) 1995  Paul Olav Tvete

   Permission to use, copy, modify, and distribute this software and its
   documentation for any purpose and without fee is hereby granted,
   provided that the above copyright notice appear in all copies and that
   both that copyright notice and this permission notice appear in
   supporting documentation.

   This file is provided AS IS with no warranties of any kind.  The author
   shall have no liability with respect to the infringement of copyrights,
   trade secrets or any patents by this file or any part thereof.  In no
   event will the author be liable for any lost revenue or profits or
   other special, indirect and consequential damages.


   Heavily modified by Mario Weilguni <mweilguni@sime.com>
*/

#include <stdio.h>

#include <QRegExp>
#include <QTimer>
#include <qimage.h>
//Added by qt3to4:
#include <QPixmap>
#include <QTextStream>
#include <QList>
#include <QShowEvent>
#include <QDomDocument>

#include <kapplication.h>
#include <klocale.h>
#include <kaction.h>
#include <ktoggleaction.h>
#include <kstdaction.h>
#include <kdebug.h>
#include <kcarddialog.h>
#include <krandom.h>
#include <kinputdialog.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>
#include <ktemporaryfile.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <kacceleratormanager.h>
#include <kmenubar.h>
#include <ktoolinvocation.h>
#include <kglobal.h>
#include <kicon.h>

#include "pwidget.h"
#include "version.h"
#include "dealer.h"
#include "cardmaps.h"
#include "speeds.h"
#include "gamestatsimpl.h"


static pWidget *current_pwidget = 0;


void saveGame(int) {
    current_pwidget->saveGame();
}

pWidget::pWidget()
    : KMainWindow(0), dill(0)
{
    setObjectName( "pwidget" );
    current_pwidget = this;
    // KCrash::setEmergencySaveFunction(::saveGame);
    KStdAction::quit(kapp, SLOT(quit()), actionCollection(), "game_exit");

    undo = KStdAction::undo(this, SLOT(undoMove()),
                     actionCollection(), "undo_move");
    undo->setEnabled(false);
    (void)KStdAction::openNew(this, SLOT(newGame()),
                              actionCollection(), "new_game");
    (void)KStdAction::open(this, SLOT(openGame()),
                           actionCollection(), "open");
    recent = KStdAction::openRecent(this, SLOT(openGame(const KUrl&)),
                                    actionCollection(), "open_recent");
    recent->loadEntries(KGlobal::config());
    (void)KStdAction::saveAs(this, SLOT(saveGame()),
                           actionCollection(), "save");
    KAction *a = new KAction(i18n("&Choose Game..."), actionCollection(), "choose_game");
    connect( a, SIGNAL( triggered( bool ) ), SLOT( chooseGame() ) );

    a = new KAction(i18n("Restart &Game"), actionCollection(), "restart_game");
    a->setIcon(KIcon("reload"));
    connect( a, SIGNAL( triggered( bool ) ), SLOT( restart() ) );

    (void)KStdAction::help(this, SLOT(helpGame()), actionCollection(), "help_game");
    games = new KSelectAction(i18n("&Game Type"), actionCollection(), "game_type");
    connect( games, SIGNAL( triggered( int ) ), SLOT( newGameType() ) );

    QStringList list;
    QList<DealerInfo*>::ConstIterator it;
    int max_type = 0;

    for (it = DealerInfoList::self()->games().begin();
         it != DealerInfoList::self()->games().end(); ++it)
    {
        // while we develop, it may happen that some lower
        // indices do not exist
        int index = (*it)->gameindex;
        for (int i = 0; i <= index; i++)
            if (list.count() <= i)
                list.append("unknown");
        list[index] = i18n((*it)->name);
        if (max_type < index)
            max_type = index;
    }
    games->setItems(list);

    KGlobal::dirs()->addResourceType("wallpaper", KStandardDirs::kde_default("data") + "kpat/backgrounds/");
    KGlobal::dirs()->addResourceType("wallpaper", KStandardDirs::kde_default("data") + "ksnake/backgrounds/");
    wallpapers = new KSelectAction(i18n("&Change Background"), actionCollection(), "wallpaper");
    connect( wallpapers, SIGNAL( triggered( int ) ), SLOT(changeWallpaper() ) );

    list.clear();
    wallpaperlist.clear();
    QStringList wallpaperlist2 = KGlobal::dirs()->findAllResources("wallpaper", QString::null,
                                                                   false, true, list);
    QStringList list2;
    for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
	QString file = *it;
	int rindex = file.lastIndexOf('.');
	if (rindex != -1) {
	  QString ext = file.mid(rindex + 1).toLower();
	  if (ext == "jpeg" || ext == "png" || ext == "jpg") {
	     list2.append(file.left(rindex));
             wallpaperlist.append( file );
          }
	}
    }

    wallpapers->setItems(list2);
    wallpapers->setCurrentItem(list2.indexOf("No-Ones-Laughing-3"));

    (void)new cardMap();
    changeWallpaper();

/*    backs = new KAction(i18n("&Switch Cards..."), actionCollection(), "backside");
      connect( backs, SIGNAL( triggered(bool ) ), SLOT( changeBackside() ) );*/
    stats = new KAction(i18n("&Statistics"), actionCollection(),"game_stats");
    connect( stats, SIGNAL( triggered( bool ) ), SLOT(showStats()) );

    dropaction = new KToggleAction(i18n("&Enable Autodrop"),
                                   actionCollection(), "enable_autodrop");
    connect( dropaction, SIGNAL( triggered( bool ) ), SLOT(enableAutoDrop()) );
    dropaction->setCheckedState(KGuiItem(i18n("Disable Autodrop")));

    KConfigGroup cg(KGlobal::config(), settings_group );

    QString bgpath = cg.readPathEntry("Background");
    kDebug(11111) << "bgpath '" << bgpath << "'" << endl;
    if (bgpath.isEmpty())
        bgpath = KStandardDirs::locate("wallpaper", "No-Ones-Laughing-3.jpg");
    background = QPixmap(bgpath);
    if ( background.isNull() ) {
        background = QPixmap( KStandardDirs::locate("wallpaper", "No-Ones-Laughing-3.jpg") );
        cg.writePathEntry( "Background", QString::null );
    }

    bool autodrop = cg.readEntry("Autodrop", true);
    dropaction->setChecked(autodrop);

    int game = cg.readEntry("DefaultGame", 0);
    if (game > max_type)
        game = max_type;
    games->setCurrentItem(game);

    statusBar()->insertPermanentItem( "", 1, 0 );

    createGUI(QString::null/*, false*/);
    KAcceleratorManager::manage(menuBar());

    newGameType();
    adjustSize();
    setAutoSaveSettings();
}

pWidget::~pWidget()
{
    delete dill;
}

void pWidget::undoMove() {
    if( dill )
        dill->undo();
}

void pWidget::helpGame()
{
    if (!dill)
        return;
    KToolInvocation::invokeHelp(dill->anchorName());
}

void pWidget::undoPossible(bool poss)
{
    undo->setEnabled(poss);
}

void pWidget::changeWallpaper()
{
    if (wallpapers->currentItem() < 0 || wallpapers->currentItem() >= wallpaperlist.count())
	return;

    QString bgpath=KStandardDirs::locate("wallpaper", wallpaperlist[wallpapers->currentItem()]);
    if (bgpath.isEmpty())
        return;
    background = QPixmap(bgpath);
    if (background.isNull()) {
        KMessageBox::sorry(this, i18n("<qt>Could not load wallpaper<br/>%1</qt>", bgpath));
        return;
    }

    QImage bg = background.toImage().convertToFormat(QImage::Format_Indexed8);
    if (bg.isNull() || !bg.numColors())
        return;

    KConfigGroup cg(KGlobal::config(), settings_group);

    cg.writePathEntry("Background", bgpath);

    if (dill) {
        dill->setBackgroundPixmap(background, midcolor);
        dill->scene()->update();
    }
}

void pWidget::enableAutoDrop()
{
    bool drop = dropaction->isChecked();
    KConfigGroup cg(KGlobal::config(), settings_group );
    cg.writeEntry( "Autodrop", drop);
    dill->setAutoDropEnabled(drop);
}

void pWidget::newGame()
{
    // Check if the user is already running a game, and if she is,
    // then ask if she wants to abort it.
    if (!dill->dscene()->isGameWon() && !dill->dscene()->isGameLost()
	&& KMessageBox::warningContinueCancel(0,
                                     i18n("You are already running an unfinished game.  "
                                          "If you abort the old game to start a new one, "
                                          "the old game will be registered as a loss in "
                                          "the statistics file.\n"
                                          "What do you want to do?"),
                                     i18n("Abort Current Game?"),
                                     KGuiItem(i18n("Abort Old Game")),
                                     "careaboutstats" )  == KMessageBox::Cancel)
        return;

    dill->setGameNumber(KRandom::random());
    setGameCaption();
    restart();
}


void pWidget::restart()
{
    statusBar()->clearMessage();
    dill->startNew();
}

void pWidget::setGameCaption()
{
    QString name = games->currentText();
    QString newname;
    QString gamenum;
    gamenum.setNum( dill->gameNumber() );
    for (int i = 0; i < name.length(); i++)
        if (name.at(i) != QChar('&'))
            newname += name.at(i);

    setCaption( newname + " - " + gamenum );
}

void pWidget::newGameType()
{
    delete dill;
    dill = 0;
    slotUpdateMoves();

    int id = games->currentItem();
    if ( id < 0 )
        id = 0;
    kDebug() << "newGameType " << id << endl;
    for (QList<DealerInfo*>::ConstIterator it = DealerInfoList::self()->games().begin();
	it != DealerInfoList::self()->games().end(); ++it) {
        if ((*it)->gameindex == id) {
            dill = new Dealer( this );
            dill->setScene( (*it)->createGame() );
            QString name = (*it)->name;
            name = name.replace(QRegExp("[&']"), "");
            name = name.replace(QRegExp("[ ]"), "_").toLower();
            dill->setAnchorName("game_" + name);
            connect(dill, SIGNAL(saveGame()), SLOT(saveGame()));
            connect(dill->dscene(), SIGNAL(gameInfo(const QString&)),
                    SLOT(slotGameInfo(const QString &)));
            connect(dill, SIGNAL(updateMoves()),
                    SLOT(slotUpdateMoves()));
            dill->setGameId(id);
            dill->setupActions();
            dill->setBackgroundPixmap(background, midcolor);
            dill->startNew();
            break;
        }
    }

    if (!dill) {
        kError() << "unimplemented game type " << id << endl;
        dill = new Dealer( this );
        dill->setScene( DealerInfoList::self()->games().first()->createGame() );
    }

    connect(dill, SIGNAL(undoPossible(bool)), SLOT(undoPossible(bool)));
    connect(dill, SIGNAL(gameWon(bool)), SLOT(gameWon(bool)));
    connect(dill, SIGNAL(gameLost()), SLOT(gameLost()));

    dill->setAutoDropEnabled(dropaction->isChecked());

    // it's a bit tricky - we have to do this here as the
    // base class constructor runs before the derived class's
    dill->takeState();

    setGameCaption();

    KConfigGroup cg(KGlobal::config(), settings_group);
    cg.writeEntry("DefaultGame", id);

    QSize min(700,400);
    min = min.expandedTo(dill->minimumCardSize());
    dill->setMinimumSize(min);
    dill->resize(min);
    updateGeometry();
    setCentralWidget(dill);
    dill->show();
}

void pWidget::showEvent(QShowEvent *e)
{
    if (dill)
        dill->setMinimumSize(QSize(0,0));
    KMainWindow::showEvent(e);
}

void pWidget::slotGameInfo(const QString &text)
{
    statusBar()->showMessage(text, 3000);
}

void pWidget::slotUpdateMoves()
{
    int moves = 0;
    if ( dill ) moves = dill->getMoves();
    statusBar()->changeItem( i18np("1 move", "%n moves", moves), 1 );
}

void pWidget::chooseGame()
{
    bool ok;
    long number = KInputDialog::getText(i18n("Game Number"),
                                        i18n("Enter a game number (FreeCell deals are the same as in the FreeCell FAQ):"),
                                        QString::number(dill->gameNumber()), 0, this).toLong(&ok);
    if (ok) {
        dill->setGameNumber(number);
        setGameCaption();
        restart();
    }
}

void pWidget::gameWon(bool withhelp)
{
    QString congrats;
    if (withhelp)
        congrats = i18n("Congratulations! We have won!");
    else
        congrats = i18n("Congratulations! You have won!");
#if TEST_SOLVER == 0
    KMessageBox::information(this, congrats, i18n("Congratulations!"));
#endif
    QTimer::singleShot(0, this, SLOT(newGame()));
#if TEST_SOLVER == 1
    dill->demo();
#endif
}

void pWidget::gameLost()
{
    QString   dontAskAgainName = "gameLostDontAskAgain";

    KConfigGroup cg(KGlobal::config(), QLatin1String("Notification Messages"));
    QString dontAsk = cg.readEntry(dontAskAgainName,QString()).toLower();

    // If we are ordered never to ask again and to continue the game,
    // then do so.
    if (dontAsk == "no")
	return;
    // If it says yes, we ask anyway. Just starting a new game would
    // be incredibly annoying.
    if (dontAsk == "yes")
	dontAskAgainName.clear();

    if (KMessageBox::questionYesNo(this, i18n("You could not win this game, "
                                              "but there is always a second try.\nStart a new game?"),
                                   i18n("Could Not Win!"),
                                   KGuiItem(i18n("New Game")),
				   KStdGuiItem::cont(),
				   dontAskAgainName) == KMessageBox::Yes) {

        QTimer::singleShot(0, this, SLOT(newGame()));
    }
}

void pWidget::openGame(const KUrl &url)
{
    QString tmpFile;
    if( KIO::NetAccess::download( url, tmpFile, this ) )
    {
        QFile of(tmpFile);
        of.open(QIODevice::ReadOnly);
        QDomDocument doc;
        QString error;
        if (!doc.setContent(&of, &error))
        {
            KMessageBox::sorry(this, error);
            return;
        }
        int id = doc.documentElement().attribute("id").toInt();

        if (id != games->currentItem()) {
            games->setCurrentItem(id);
            newGameType();
            if (!dill) {
                KMessageBox::error(this, i18n("The saved game is of unknown type!"));
                games->setCurrentItem(0);
                newGameType();
            }
        }
        dill->openGame(doc);
        setGameCaption();
        KIO::NetAccess::removeTempFile( tmpFile );
        recent->addUrl(url);
        recent->saveEntries(KGlobal::config());
    }
}

void pWidget::openGame()
{
    KUrl url = KFileDialog::getOpenUrl();
    openGame(url);
}

void pWidget::saveGame()
{
    KUrl url = KFileDialog::getSaveUrl();
    KTemporaryFile file;
    file.open();
    QDomDocument doc("kpat");
    dill->saveGame(doc);
    QTextStream stream (&file);
    stream << doc.toString();
    stream.flush();
    KIO::NetAccess::upload(file.fileName(), url, this);
    recent->addUrl(url);
    recent->saveEntries(KGlobal::config());
}

void pWidget::showStats()
{
	GameStatsImpl* dlg = new GameStatsImpl(this);
	if (dill)
		dlg->showGameType(dill->gameId());
	dlg->exec();
}

#include "pwidget.moc"

