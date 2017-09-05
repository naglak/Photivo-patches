/*******************************************************************************
**
** Photivo
**
** Copyright (C) 2009-2011 Michael Munzert <mail@mm-log.com>
** Copyright (C) 2011 Bernd Schoeler <brjohn@brother-john.net>
**
** This file is part of Photivo.
**
** Photivo is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License version 3
** as published by the Free Software Foundation.
**
** Photivo is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Photivo.  If not, see <http://www.gnu.org/licenses/>.
**
*******************************************************************************/

#ifndef PTDRAWLINEINTERACTION_H
#define PTDRAWLINEINTERACTION_H

//==============================================================================

#include "ptAbstractInteraction.h"

#include <QMouseEvent>
#include <QLine>
#include <QGraphicsLineItem>

//==============================================================================

class ptLineInteraction : public ptAbstractInteraction {
Q_OBJECT

public:
  explicit ptLineInteraction(QGraphicsView* View);
  ~ptLineInteraction();

  /*! Reimplemented from base class. */
  virtual Qt::KeyboardModifiers modifiers()    const { return Qt::NoModifier; }

  /*! Reimplemented from base class. */
  virtual ptMouseActions        mouseActions() const { return maDragDrop; }

  /*! Reimplemented from base class. */
  virtual Qt::MouseButtons      mouseButtons() const { return Qt::LeftButton; }

  double angle();

//------------------------------------------------------------------------------

private:
  QLineF* m_Line;
  short m_NowDragging;
  QGraphicsLineItem* m_LineItem;

  void Finalize(const ptStatus status);

//------------------------------------------------------------------------------

private slots:
  void keyAction(QKeyEvent* event);
  void mouseAction(QMouseEvent* event);


};
#endif // PTDRAWLINEINTERACTION_H
