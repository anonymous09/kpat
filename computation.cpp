/***********************-*-C++-*-********

  computation.h  implements a patience card game

     Copyright (C) 1995  Paul Olav Tvete

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

//
// This one was discussed on the newsgroup rec.games.abstract
//
****************************************/

#include <qcombobox.h>
#include <krandomsequence.h>
#include "computation.h"
#include <klocale.h>
#include <kmainwindow.h>
#include <pile.h>
#include <deck.h>
#include <assert.h>

Computation::Computation( KMainWindow *parent, const char *name )
    :Dealer( parent, name)
{
    deck = new Deck(0, this);

    deck->hide();

    for (int i = 0; i < 4; i++) {
        play[i] = new Pile(1 + i, this);
        play[i]->move(110 + i * 100, 150);
        play[i]->setAddFlags(Pile::addSpread);
        play[i]->setRemoveFlags(Pile::several);

        target[i] = new Pile(5 + i, this);
        target[i]->move(110 + i * 100, 10);

        target[i]->setAddFlags(Pile::several);
        target[i]->setRemoveFlags(Pile::disallow);
        target[i]->setAddFun(&step);
    }

    pile = new Pile(13, this);
    pile->setAddFlags(Pile::disallow);
    pile->setRemoveFlags(Pile::autoTurnTop);
    pile->move(10, 10);

    deal();
}

void Computation::restart() {
    deck->collectAndShuffle();
    deal();
}

void Computation::deal() {
    while (!deck->isEmpty()) {
        Card *c = deck->nextCard();
        pile->add(c, !deck->isEmpty(), false);
    }
}

inline bool matches(const CardList &cl, Card *start, int offset)
{
    Card *before = start; // maybe 0 for ignore first card
    for (CardList::ConstIterator it = cl.begin(); it != cl.end(); ++it)
    {
        if (before && (*it)->value() % 13 != (before->value() + offset) % 13)
            return false;
        before = *it;
    }
    return true;
}

bool Computation::step( const Pile* c1, const CardList& cl) {
    assert(c1->index() >= 5 && c1->index() <= 8);

    int offset = c1->index() - 4;

    if (c1->isEmpty()) {
        Card::Values start = static_cast<Card::Values>(Card::Ace + (offset - 1));
        return cl.first()->value() == start && matches(cl, 0, offset);
    }

    return matches(cl, c1->top(), offset);
}

bool Computation::isGameWon() const
{
    if (!deck->isEmpty())
        return false;

    if (!pile->isEmpty())
        return false;

    for (int c = 0; c < 4; c++)
        if (!play[c]->isEmpty())
            return false;

    return true;
}

static class LocalDealerInfo2 : public DealerInfo
{
public:
    LocalDealerInfo2() : DealerInfo(I18N_NOOP("&Computation"), 6) {}
    virtual Dealer *createGame(KMainWindow *parent) { return new Computation(parent); }
} ldi2;

#include "computation.moc"
