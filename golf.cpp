/*
 * Copyright (C) 2001-2009 Stephan Kulow <coolo@kde.org>
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

#include "golf.h"

#include "dealerinfo.h"
#include "speeds.h"
#include "patsolve/golfsolver.h"

#include "Shuffle"

#include <KLocale>


void Golf::initialize()
{
    const qreal dist_x = 1.11;
    const qreal smallNeg = -1e-6;

    static_cast<KStandardCardDeck*>( deck() )->setDeckContents();

    talon = new PatPile(0, "talon");
    talon->setPileRole(PatPile::Stock);
    talon->setPilePos(0, smallNeg);
    talon->setSpread(0, 0);
    connect(talon, SIGNAL(clicked(KCard*)), SLOT(newCards()));
    addPile(talon);

    waste = new PatPile(8, "waste");
    waste->setPileRole(PatPile::Foundation);
    waste->setPilePos(1.1, smallNeg);
    waste->setSpread(0.12, 0);
    waste->setReservedSpace( 0, 0, 1 + 5 * dist_x, 1 );
    addPile(waste);

    for( int r = 0; r < 7; r++ ) {
        stack[r]=new PatPile(1+r, QString("stack%1").arg(r));
        stack[r]->setPileRole(PatPile::Tableau);
        stack[r]->setPilePos(r*dist_x,0);
        // Manual tweak of the pile z values to make some animations better.
        stack[r]->setZValue((7-r)/100.0);
        stack[r]->setReservedSpace( 0, 0, 1, 2.3 );
        addPile(stack[r]);
    }

    setActions(DealerScene::Hint | DealerScene::Demo | DealerScene::Draw);
    setSolver( new GolfSolver( this ) );

    connect( this, SIGNAL(cardClicked(KCard*)), this, SLOT(handleCardClick(KCard*)) );
}


bool Golf::checkAdd(const PatPile * pile, const QList<KCard*> & oldCards, const QList<KCard*> & newCards) const
{
    return pile->pileRole() == PatPile::Foundation
           && ( getRank( newCards.first() ) == getRank( oldCards.last() ) + 1
                || getRank( newCards.first() ) == getRank( oldCards.last() ) - 1 );
}


bool Golf::checkRemove(const PatPile * pile, const QList<KCard*> & cards) const
{
    return pile->pileRole() == PatPile::Tableau
           && cards.first() == pile->top();
}


void Golf::restart()
{
    QList<KCard*> cards = shuffled( deck()->cards(), gameNumber() );

    for(int i=0;i<5;i++)
        for(int r=0;r<7;r++)
            addCardForDeal( stack[r], cards.takeLast(), true, stack[6]->pos() );

    while ( !cards.isEmpty() )
    {
        KCard * c = cards.takeFirst();
        c->setPos( talon->pos() );
        c->setFaceUp( false );
        talon->add( c );
    }

    startDealAnimation();

    flipCardToPile(talon->top(), waste, DURATION_MOVE);

    emit newCardsPossible( true );
}


KCard *Golf::newCards()
{
    if (talon->isEmpty())
         return 0;

    if ( waste->top() && deck()->hasAnimatedCards() )
        return waste->top();

    clearHighlightedItems();

    flipCardToPile(talon->top(), waste, DURATION_MOVE);

    if ( talon->isEmpty() )
        emit newCardsPossible( false );

    return waste->top();
}


void Golf::handleCardClick( KCard * card )
{
    PatPile * pile = dynamic_cast<PatPile*>( card->pile() );

    if ( pile
         && pile->pileRole() == PatPile::Tableau
         && card == pile->top() )
    {
        KCardPile * p = findTarget( card );
        if ( p )
            moveCardToPile( card, p, DURATION_MOVE );
    }
}


bool Golf::drop()
{
    return false;
}


void Golf::setGameState( const QString & )
{
    emit newCardsPossible( !talon->isEmpty() );
}


static class GolfDealerInfo : public DealerInfo
{
public:
    GolfDealerInfo()
      : DealerInfo(I18N_NOOP("Golf"), GolfId)
    {}

    virtual DealerScene *createGame() const
    {
        return new Golf();
    }
} golfDealerInfo;


#include "golf.moc"
