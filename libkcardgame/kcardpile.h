/*
 * Copyright (C) 1995 Paul Olav Tvete <paul@troll.no>
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

#ifndef KCARDPILE_H
#define KCARDPILE_H

#include "kcard.h"
class KCardScene;
#include "libkcardgame_export.h"
#include "../speeds.h"

class QPropertyAnimation;
#include <QtGui/QGraphicsPixmapItem>


class LIBKCARDGAME_EXPORT KCardPile : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
    Q_PROPERTY( qreal highlightedness READ highlightedness WRITE setHighlightedness )

public:
    explicit KCardPile( const QString & objectName = QString() );
    virtual ~KCardPile();

    enum { Type = QGraphicsItem::UserType + 2 };
    virtual int type() const;

    KCardScene * cardScene() const;

    QList<KCard*> cards() const;
    int count() const;
    bool isEmpty() const;
    int indexOf( const KCard * card ) const;
    KCard * at( int index ) const;
    KCard * top() const;
    QList<KCard*> topCardsDownTo( const KCard * card ) const;

    void setPilePos( QPointF pos );
    void setPilePos( qreal x, qreal y );
    QPointF pilePos() const;

    void setReservedSpace( QSizeF space );
    void setReservedSpace( qreal width, qreal height );
    QSizeF reservedSpace() const;

    void setMaximumSpace( QSizeF size );
    QSizeF maximumSpace() const;

    void setSpread( QSizeF spread );
    void setSpread( qreal width, qreal height );
    QSizeF spread() const;

    void setAutoTurnTop( bool autoTurnTop );
    bool autoTurnTop() const;

    virtual void setVisible(bool vis);

    void setHighlighted( bool highlighted );
    bool isHighlighted() const;

    void setGraphicVisible( bool visible );
    bool isGraphicVisible();

    void setGraphicSize( QSize size );

    virtual void animatedAdd( KCard * card, bool faceUp );
    virtual void add( KCard * card, int index = -1 );
    virtual void remove( KCard * card );
    void clear();

    virtual void layoutCards( int duration = DURATION_RELAYOUT );
    virtual void moveCardsBack( QList<KCard*> & cards, int duration = DURATION_MOVEBACK );
    virtual void moveCards( QList<KCard*> & cards, KCardPile * pile );

    void cardPressed( KCard * card );

public slots:
    virtual bool cardClicked( KCard * card );
    virtual bool cardDoubleClicked( KCard * card );
    virtual void relayoutCards();

signals:
    void clicked( KCard * card );
    void doubleClicked( KCard * card );
    void pressed( KCard * card );

protected:
    virtual QPointF cardOffset( const KCard * card ) const;

    virtual QPixmap normalPixmap( QSize size );
    virtual QPixmap highlightedPixmap( QSize size );

private:
    void updatePixmap( QSize size );

    void setHighlightedness( qreal highlightedness );
    qreal highlightedness() const;

    QList<KCard*> m_cards;

    bool m_autoTurnTop;
    bool m_highlighted;
    bool m_graphicVisible;

    QPointF m_pilePos;
    QSizeF m_reserved;
    QSizeF m_spread;
    QSizeF m_maximumSpace;

    qreal m_highlightedness;

    QPropertyAnimation * m_fadeAnimation;
    QTimer * m_relayoutTimer;
};

#endif