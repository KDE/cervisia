/*
 * Copyright (c) 2004 Jason Kivlighn <mizunoami44@users.sourceforge.net>
 * Copyright (c) 2005 Christian Loose <christian.loose@kdemail.net>
 *
 * based on work by Jason Kivlighn (krecipes/src/widgets/kretextedit.cpp)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "logmessageedit.h"
using Cervisia::LogMessageEdit;

#include <qtextstream.h>
#include <kaccel.h>


LogMessageEdit::LogMessageEdit(QWidget* parent)
    : KTextEdit(parent)
    , KCompletionBase()
    , m_completing(false)
    , m_completionStartPos(0)
{
    // create the completion object
    completionObject();

    // a mouse click stops the completion process
    connect( this, SIGNAL(clicked(int, int)), SLOT(stopCompletion()) );
}


void LogMessageEdit::setCompletedText(const QString& match)
{
    int para, index;
    getCursorPosition(&para, &index);

    QString paragraphText = text(para);
    int length = index - m_completionStartPos;
    QString word = match.right(match.length() - length);

    insert(word);

    setSelection(para, index, para, m_completionStartPos + match.length());
    setCursorPosition(para, index);

    m_completing = true;

    // disable spellchecker during completion process. Otherwise we lose the
    // text selection.
    setCheckSpellingEnabled(false);
}


void LogMessageEdit::setCompletedItems(const QStringList&)
{
}


void LogMessageEdit::keyPressEvent(QKeyEvent* event)
{
    bool noModifier = (event->state() == NoButton ||
                       event->state() == ShiftButton ||
                       event->state() == Keypad);

    if( noModifier )
    {
        QString keycode = event->text();
        if( !keycode.isEmpty() && keycode.unicode()->isPrint() )
        {
            KTextEdit::keyPressEvent(event);
            tryCompletion();
            event->accept();
            return;
        }
    }

    KeyBindingMap keys = getKeyBindings();

    // handle text completion key
    KShortcut shortcut = keys[TextCompletion];
    if( shortcut.isNull() )
        shortcut = KStdAccel::shortcut(KStdAccel::TextCompletion);

    KKey key(event);

    // accept the suggested completion?
    if( m_completing && shortcut.contains(key) )
    {
        int paraFrom, indexFrom, paraTo, indexTo;
        getSelection(&paraFrom, &indexFrom, &paraTo, &indexTo);

        removeSelection();
        setCursorPosition(paraTo, indexTo);

        m_completing = false;
        setCheckSpellingEnabled(true);

        return;
    }

    // handle previous match key
    shortcut = keys[PrevCompletionMatch];
    if( shortcut.isNull() )
        shortcut = KStdAccel::shortcut(KStdAccel::PrevCompletion);

    if( shortcut.contains(key) )
    {
        rotateMatches(PrevCompletionMatch);
        return;
    }

    // handle next match key
    shortcut = keys[NextCompletionMatch];
    if( shortcut.isNull() )
        shortcut = KStdAccel::shortcut(KStdAccel::NextCompletion);

    if( shortcut.contains(key) )
    {
        rotateMatches(NextCompletionMatch);
        return;
    }

    // any other key (except modifiers) will end the text completion
    if( event->key() != Qt::Key_Shift && event->key() != Qt::Key_Control &&
        event->key() != Qt::Key_Alt   && event->key() != Qt::Key_Meta )
    {
        m_completing = false;
        setCheckSpellingEnabled(true);
    }

    KTextEdit::keyPressEvent(event);
}


void LogMessageEdit::stopCompletion()
{
    m_completing = false;
    setCheckSpellingEnabled(true);
}


void LogMessageEdit::tryCompletion()
{
    int para, index;
    getCursorPosition(&para, &index);

    QString paragraphText = text(para);
    if( paragraphText.at(index).isSpace() )
    {
        if( !m_completing )
            m_completionStartPos = paragraphText.findRev(' ', index-1) + 1;

        int length = index - m_completionStartPos;
        QString word = paragraphText.mid(m_completionStartPos, length);

        QString match = compObj()->makeCompletion(word);
        if( !match.isNull() && match != word )
        {
            setCompletedText(match);
        }
        else
        {
            m_completing = false;
            setCheckSpellingEnabled(true);
        }
    }
}


void LogMessageEdit::rotateMatches(KeyBindingType type)
{
    KCompletion* completionObj = compObj();
    if( completionObj && m_completing &&
        (type == PrevCompletionMatch || type == NextCompletionMatch) )
    {
        QString match = (type == PrevCompletionMatch) ? completionObj->previousMatch()
                                                      : completionObj->nextMatch();

        int para, index;
        getCursorPosition(&para, &index);

        QString paragraphText = text(para);

        QString word = paragraphText.mid(m_completionStartPos, index - m_completionStartPos);

        if( match.isNull() || match == word )
            return;

        setCompletedText(match);
    }
}

#include "logmessageedit.moc"
