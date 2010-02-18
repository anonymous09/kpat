/*
 * Copyright (C) 2000-2009 Stephan Kulow <coolo@kde.org>
 * Copyright (C) 2010 Parker Coates <parker.coates@kdemail.net>
 *
 * License of original code:
 * -------------------------------------------------------------------------
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 *
 *   This file is provided AS IS with no warranties of any kind.  The author
 *   shall have no liability with respect to the infringement of copyrights,
 *   trade secrets or any patents by this file or any part thereof.  In no
 *   event will the author be liable for any lost revenue or profits or
 *   other special, indirect and consequential damages.
 * -------------------------------------------------------------------------
 *
 * License of modifications/additions made after 2009-01-01:
 * -------------------------------------------------------------------------
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of 
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * -------------------------------------------------------------------------
 */

#include "clock.h"

#include "dealerinfo.h"
#include "patsolve/clocksolver.h"

#include "libkcardgame/shuffle.h"

#include <KLocale>


Clock::Clock( )
    : DealerScene( )
{
    setSceneAlignment( AlignHCenter | AlignVCenter );

    setupDeck(new KStandardCardDeck());

    const qreal dist_x = 1.11;
    const qreal ys[12] = {   0./96,  15./96,  52./96, 158./96, 264./96, 301./96, 316./96, 301./96, 264./96, 158./96,  52./96,  15./96};
    const qreal xs[12] = { 200./72, 280./72, 360./72, 400./72, 360./72, 280./72, 200./72, 120./72, 40./72, 0./72, 40./72, 120./72};

    for (int i=0; i<12; i++) {
        target[i] = new PatPile(i+1, QString("target%1").arg(i));
        target[i]->setPileRole(PatPile::Foundation);
        target[i]->setFoundation(true);
        target[i]->setPilePos(4 * dist_x + 0.4 + xs[i], 0.2 + ys[i]);
        target[i]->setSpread(0, 0);
        addPile(target[i]);
    }

    for (int i=0; i<8; i++) {
        store[i] = new PatPile(14+i, QString("store%1").arg(i));
        store[i]->setPileRole(PatPile::Tableau);
        store[i]->setPilePos(dist_x*(i%4), 2.5 * (i/4));
        store[i]->setReservedSpace( QSizeF( 1.0, 1.8 ) );
        addPile(store[i]);
    }

    setActions(DealerScene::Hint | DealerScene::Demo);
    setSolver( new ClockSolver( this ) );
}

void Clock::restart()
{
    foreach( KCardPile * p, piles() )
        p->clear();
    deal();
}

bool Clock::checkAdd(const PatPile * pile, const QList<KCard*> & oldCards, const QList<KCard*> & newCards) const
{
    if ( pile->pileRole() == PatPile::Tableau )
    {
        return oldCards.isEmpty()
               || getRank( newCards.first() ) == getRank( oldCards.last() ) - 1;
    }
    else
    {
        return getSuit( oldCards.last() ) == getSuit( newCards.first() )
               && ( getRank( newCards.first() ) == getRank( oldCards.last() ) + 1
                    || ( getRank( oldCards.last() ) == KStandardCardDeck::King
                         && getRank( newCards.first() ) == KStandardCardDeck::Ace ) );
    }
}


bool Clock::checkRemove(const PatPile* pile, const QList<KCard*> & cards) const
{
    return pile->pileRole() == PatPile::Tableau
           && cards.first() == pile->top();
}

void Clock::deal() {
    static const KStandardCardDeck::Suit suits[12] = { KStandardCardDeck::Diamonds, KStandardCardDeck::Spades, KStandardCardDeck::Hearts, KStandardCardDeck::Clubs,
                                                       KStandardCardDeck::Diamonds, KStandardCardDeck::Spades, KStandardCardDeck::Hearts, KStandardCardDeck::Clubs,
                                                       KStandardCardDeck::Diamonds, KStandardCardDeck::Spades, KStandardCardDeck::Hearts, KStandardCardDeck::Clubs };
    static const KStandardCardDeck::Rank ranks[12] = { KStandardCardDeck::Nine, KStandardCardDeck::Ten, KStandardCardDeck::Jack, KStandardCardDeck::Queen,
                                                       KStandardCardDeck::King, KStandardCardDeck::Two, KStandardCardDeck::Three, KStandardCardDeck::Four,
                                                       KStandardCardDeck::Five, KStandardCardDeck::Six, KStandardCardDeck::Seven, KStandardCardDeck::Eight };

    const QPointF center = ( target[0]->pos() + target[6]->pos() ) / 2;

    QList<KCard*> cards = shuffled( deck()->cards(), gameNumber() );

    int j = 0;
    while (!cards.isEmpty())
    {
        KCard *c =cards.takeLast();
        for (int i = 0; i < 12; i++)
        {
            if (getRank(c) == ranks[i] && getSuit(c) == suits[i])
            {
                QPointF initPos = (2 * center + target[(i + 2) % 12]->pos()) / 3;
                addCardForDeal( target[i], c, true, initPos );
                c = 0;
                break;
            }
        }
        if (c)
        {
            addCardForDeal( store[j], c, true, store[ j < 4 ? 0 : 4 ]->pos() );
            j = (j + 1) % 8;
        }
    }

    startDealAnimation();
}



static class ClockDealerInfo : public DealerInfo
{
public:
    ClockDealerInfo()
      : DealerInfo(I18N_NOOP("Grandfather's Clock"), GrandfathersClockId)
    {}

    virtual DealerScene *createGame() const
    {
        return new Clock();
    }
} clockDealerInfo;


#include "clock.moc"
