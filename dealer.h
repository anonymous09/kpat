#ifndef _DEALER_H_
#define _DEALER_H_

#include <qvaluelist.h>
#include <qcanvas.h>
#include "card.h"
#include "pile.h"

class KMainWindow;
class Dealer;
class DealerInfo;

class DealerInfoList {
public:
    static DealerInfoList *self();
    void add(DealerInfo *);

    const QValueList<DealerInfo*> games() const { return list; }
private:
    QValueList<DealerInfo*> list;
    static DealerInfoList *_self;
};

class DealerInfo {
public:
    DealerInfo(const char *_name, int _index)
        : name(_name),
          gameindex(_index)
{
    DealerInfoList::self()->add(this);
}
    const char *name;
    uint gameindex;
    virtual Dealer *createGame(KMainWindow *parent) = 0;
};

class CardState;

typedef QValueList<CardState> CardStateList;

struct State
{
    CardStateList cards;
    QByteArray gameData;
};

/***************************************************************

  Dealer -- abstract base class of all varieties of patience

***************************************************************/
class Dealer: public QCanvasView
{
    Q_OBJECT

public:

    Dealer( KMainWindow* parent = 0, const char* name = 0 );
    virtual ~Dealer();

    void enlargeCanvas(QCanvasRectangle *c);
    void resetSize(const QSize &size);
    void setGameNumber(long gmn);
    long gameNumber() const;

    virtual bool isGameWon() const;

    void setViewSize(const QSize &size);
    virtual QSize sizeHint() const;

    void addPile(Pile *p);
    void removePile(Pile *p);

    virtual bool checkRemove( int checkIndex, const Pile *c1, const Card *c) const;
    virtual bool checkAdd   ( int checkIndex, const Pile *c1, const CardList& c2) const;

public slots:

    // restart is pure virtual, so we need something else
    virtual void startNew();
    void undo();
    virtual void takeState();
    virtual bool startAutoDrop();

signals:
    void undoPossible(bool poss);
    void gameWon();

protected:

    virtual void restart() = 0;

    virtual void contentsMousePressEvent(QMouseEvent* e);
    virtual void contentsMouseMoveEvent( QMouseEvent* );
    virtual void contentsMouseReleaseEvent( QMouseEvent* );
    virtual void contentsMouseDoubleClickEvent( QMouseEvent* );
    virtual void viewportResizeEvent ( QResizeEvent * );

    void unmarkAll();
    void mark(Card *c);
    Pile *findTarget(Card *c);
    virtual void cardClicked(Card *);
    virtual void pileClicked(Pile *);
    virtual void cardDblClicked(Card *);
    void won();

    KMainWindow *parent() const;

protected:
    PileList piles;

    State *getState();
    void setState(State *);

    // reimplement this to add game-specific information in the state structure
    virtual void getGameState( QDataStream & ) {}
    // reimplement this to use the game-specific information from the state structure
    virtual void setGameState( QDataStream & ) {}

    bool moved;
    CardList movingCards;
    QCanvasItemList marked;
    QPoint moving_start;
    Dealer( Dealer& );  // don't allow copies or assignments
    void operator = ( Dealer& );  // don't allow copies or assignments
    QCanvas myCanvas;
    QSize maxsize;
    QSize viewsize;
    QList<State> undoList;
    long gamenumber;


};

#endif
