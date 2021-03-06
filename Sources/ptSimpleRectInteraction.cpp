/*******************************************************************************
**
** Photivo
**
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

#include "ptSimpleRectInteraction.h"
#include "ptDefines.h"

#include <cassert>

//==============================================================================

ptSimpleRectInteraction::ptSimpleRectInteraction(QGraphicsView* View)
: ptAbstractInteraction(View),
  m_CtrlPressed(0),
  m_NowDragging(0)
{
  m_DragDelta = new QLine();
  m_Rect = new QRectF();
  m_RectItem = NULL;
}

//==============================================================================

ptSimpleRectInteraction::~ptSimpleRectInteraction() {
  delete m_Rect;
  delete m_RectItem;
  delete m_DragDelta;
}

//==============================================================================

void ptSimpleRectInteraction::Finalize(const ptStatus status) {
  if (m_RectItem != NULL) {
    FView->scene()->removeItem(m_RectItem);
    DelAndNull(m_RectItem);
  }

  m_NowDragging = 0;
  emit finished(status);
}

//==============================================================================

void ptSimpleRectInteraction::keyAction(QKeyEvent* event) {
  switch (event->type()) {
    case QEvent::KeyPress: {
      if (event->key() == Qt::Key_Escape) {
        event->accept();
        Finalize(stFailure);

      // initiate rect move
      } else if (event->key() == Qt::Key_Control && m_NowDragging) {
        event->accept();
        m_CtrlPressed++;
        FView->setCursor(Qt::SizeAllCursor);
        m_DragDelta->setP1(FView->mapFromGlobal(QCursor::pos()));
      }
      break;
    }


    case QEvent::KeyRelease: {
      // finish rect move
      if (event->key() == Qt::Key_Control) {
        event->accept();
        m_CtrlPressed--;
        if (m_CtrlPressed == 0) {
          FView->setCursor(Qt::ArrowCursor);
        }
      }
      break;
    }


    default:
      // nothing to do
      break;
  }
}

//==============================================================================

void ptSimpleRectInteraction::mouseAction(QMouseEvent* event) {
  switch (event->type()) {
    // left button press
    case QEvent::MouseButtonPress: {
      if (event->button() == Qt::LeftButton) {
        event->accept();
        assert(m_RectItem == NULL);

        // map viewport coords to scene coords
        QPointF pos(FView->mapToScene(event->pos()));
        m_Rect->setTopLeft(pos);
        m_Rect->setBottomRight(pos);

        QPen pen(QColor(150, 150, 150));
        m_RectItem = FView->scene()->addRect(*m_Rect, pen);
        FView->repaint();

        m_NowDragging = 1;
      }
      break;
    }


    // left button release
    case QEvent::MouseButtonRelease: {
      if (event->button() == Qt::LeftButton) {
        event->accept();
        Finalize(stSuccess);
      }
      break;
    }


    // mouse move
    case QEvent::MouseMove: {
      if (m_NowDragging) {
        event->accept();

        // move rectangle
        if (m_CtrlPressed > 0) {
          m_DragDelta->setP2(event->pos());
          qreal dx = m_DragDelta->dx() / FView->transform().m11();
          qreal dy = m_DragDelta->dy() / FView->transform().m11();
          m_Rect->moveTo(qBound(0.0, m_Rect->left() + dx, FView->scene()->sceneRect().width() - m_Rect->width()),
                         qBound(0.0, m_Rect->top() + dy, FView->scene()->sceneRect().height() - m_Rect->height()) );
          m_DragDelta->setP1(event->pos());
          m_RectItem->setRect(*m_Rect);
          FView->repaint();

        // change rectangle size
        } else {
          QPointF point = FView->mapToScene(event->pos());
          m_Rect->setRight(qBound(0.0, point.x(), FView->scene()->sceneRect().right()));
          m_Rect->setBottom(qBound(0.0, point.y(), FView->scene()->sceneRect().bottom()));
          m_RectItem->setRect(m_Rect->normalized());
          FView->repaint();
        }
      }
      break;
    }


    default: {
      assert(!"Wrong event type");
      break;
    }
  } //switch
}

//==============================================================================
