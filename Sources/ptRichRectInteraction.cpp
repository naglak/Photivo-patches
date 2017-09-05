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

#include "ptRichRectInteraction.h"
#include "ptDefines.h"
#include "ptSettings.h"
#include "ptTheme.h"

extern ptSettings* Settings;
extern ptTheme* Theme;

//==============================================================================

#include <cassert>
#include <cmath>

//==============================================================================

ptRichRectInteraction::ptRichRectInteraction(QGraphicsView* View,
      const int x, const int y, const int width, const int height,
      const short FixedAspectRatio, const uint AspectRatioW,
      const uint AspectRatioH, const short Guidelines)
: ptAbstractInteraction(View),
  //constants
  EdgeThickness(40),
  TinyRectThreshold(80),
  //variables
  m_CtrlIsPressed(0),
  m_Guides(Guidelines),
  m_MovingEdge(meNone),
  m_NowDragging(0),
  m_ExitStatus(stUndetermined)
{
  assert(Settings != NULL);
  assert(Theme != NULL);

  // Setup brushes for the LightsOut rectangles
  m_LightsOutBrushes[0] = new QBrush();
  m_LightsOutBrushes[1] = new QBrush(QColor(20, 20, 20, 200));
  if (Settings->GetInt("BackgroundColor")) {
    m_LightsOutBrushes[2] = new QBrush(QColor(Settings->GetInt("BackgroundRed"),
                                              Settings->GetInt("BackgroundGreen"),
                                              Settings->GetInt("BackgroundBlue")));
  } else {
    m_LightsOutBrushes[2] = new QBrush(Theme->baseColor());
  }

  // initial rectangles for LightsOut
  for (int i = 0; i <= 3; i++) {
    m_LightsOutRects[i] = FView->scene()->addRect(
                              0,0,0,0,
                              QPen(Qt::NoPen),
                              *m_LightsOutBrushes[Settings->GetInt("LightsOut")]);
   }

  m_DragDelta = new QLine();
  m_Rect.setRect(x, y, width, height);
  m_RectItem = FView->scene()->addRect(m_Rect, QPen(QColor(255,255,255)));
  m_RectItem->setVisible(Settings->GetInt("LightsOut") != ptLightsOutMode_Black);

  m_Shadow = new QGraphicsDropShadowEffect;
  m_Shadow->setBlurRadius(1);
  m_Shadow->setOffset(1);
  m_Shadow->setColor(QColor(0,0,0));
  m_RectItem->setGraphicsEffect(m_Shadow);

  // Init guidelines items
  for (int i = 0; i <= 3; i++) {
    m_GuideItems[i] = new QGraphicsLineItem();
    m_GuideItems[i]->hide();
    m_GuideItems[i]->setPen(QPen(QColor(255,255,255)));
    FView->scene()->addItem(m_GuideItems[i]);
  }

  setAspectRatio(FixedAspectRatio, AspectRatioW, AspectRatioH, 0);
  UpdateScene();
}

//==============================================================================

ptRichRectInteraction::~ptRichRectInteraction() {
  for (int i = 0; i <= 3; i++) {
    FView->scene()->removeItem(m_GuideItems[i]);
    DelAndNull(m_GuideItems[i]);
  }

  DelAndNull(m_DragDelta);
  DelAndNull(m_RectItem);

  for (int i = 0; i <= 3; i++) {
    DelAndNull(m_LightsOutRects[i]);
  }
}
/* Resources managed by Qt parent mechanism (do not delete manually):
   m_Shadow
*/

//==============================================================================

void ptRichRectInteraction::setAspectRatio(const short FixedAspectRatio,
                                           uint AspectRatioW, uint AspectRatioH,
                                           const short ImmediateUpdate /*= 1*/)
{
  m_FixedAspectRatio = FixedAspectRatio;
  m_AspectRatioW = AspectRatioW;
  m_AspectRatioH = AspectRatioH;

  if (FixedAspectRatio) {
    assert((AspectRatioW != 0) && (AspectRatioH != 0));
    m_AspectRatio = (double) AspectRatioW / (double) AspectRatioH;
  }

  if (ImmediateUpdate) {
    m_MovingEdge = meNone;
    UpdateScene();
  }
}

//==============================================================================

void ptRichRectInteraction::setGuidelines(const short mode) {
  if (m_Guides != mode) {
    m_Guides = mode;
    RecalcGuides();
    FView->repaint();
  }
}

//==============================================================================

void ptRichRectInteraction::setLightsOut(short mode) {
  mode = qBound((short)0, mode, (short)2);
  Settings->SetValue("LightsOut", mode);
  m_RectItem->setVisible(mode != ptLightsOutMode_Black);
  for (int i = 0; i <= 3; i++) {
    m_LightsOutRects[i]->setBrush(*m_LightsOutBrushes[mode]);
  }
  UpdateScene();
}

//==============================================================================

void ptRichRectInteraction::mouseAction(QMouseEvent* event) {
  switch (event->type()) {
    case QEvent::MouseButtonPress:
      MousePressHandler(event);
      break;
    case QEvent::MouseButtonRelease:
      MouseReleaseHandler(event);
      break;
    case QEvent::MouseButtonDblClick:
      MouseDblClickHandler(event);
      break;
    case QEvent::MouseMove:
      MouseMoveHandler(event);
      break;
    default:
      // ignore other mouse events
      break;
  }
}

//==============================================================================

void ptRichRectInteraction::MousePressHandler(const QMouseEvent* event) {
  if (event->button() == Qt::LeftButton && event->modifiers() == Qt::NoModifier) {
    QPointF scPos = FView->mapToScene(event->pos());
    m_DragDelta->setPoints(event->pos(), event->pos());

    if (m_RectItem->contains(scPos)) {
      m_NowDragging = 1;

    // Start new rect when none is present or clicked outside current one.
    } else if (FView->scene()->sceneRect().contains(scPos)) {
      m_NowDragging = 1;
      m_MovingEdge = meNone;
      m_Rect.setRect(scPos.x(), scPos.y(), 1.0, 1.0);
      m_RectItem->setRect(m_Rect);
      m_CtrlIsPressed = 0;
      UpdateScene();
    }
  }
}

//==============================================================================

void ptRichRectInteraction::MouseReleaseHandler(const QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    m_NowDragging = 0;
  }
}

//==============================================================================

void ptRichRectInteraction::MouseDblClickHandler(const QMouseEvent* event) {
  if (m_RectItem->contains(FView->mapToScene(event->pos())) ) {
    m_ExitStatus = stSuccess;
    Finalize();
  }
}

//==============================================================================

void ptRichRectInteraction::MouseMoveHandler(const QMouseEvent* event) {
  if (m_NowDragging) {
    m_DragDelta->setP2(event->pos());   // GraphicsView scale
    qreal dx = m_DragDelta->dx() / FView->transform().m11();  // Transform delta to scene scale.
    qreal dy = m_DragDelta->dy() / FView->transform().m11();

    // Move current rectangle. The qBounds make sure it stops at image boundaries.
    //if (m_CtrlIsPressed > 0) {
    if (m_MovingEdge == meCenter) {
      m_Rect.moveTo(
          qBound(0.0, m_Rect.left() + dx,
                 FView->scene()->sceneRect().width() - m_Rect.width()),
          qBound(0.0, m_Rect.top() + dy,
                 FView->scene()->sceneRect().height() - m_Rect.height())
      );
      RecalcGuides();
      RecalcLightsOutRects();
      m_RectItem->setRect(m_Rect);

    } else {
      // initialize movement direction when rectangle was just started
      if (m_MovingEdge == meNone) {
        if ((dx >= 0) && (dy >= 0)) {
          m_MovingEdge = meBottomRight;
        } else if ((dx < 0) && (dy <= 0)) {
          m_MovingEdge = meBottomLeft;
        } else if ((dx > 0) && (dy < 0)) {
          m_MovingEdge = meTopLeft;
        } else if ((dx > 0) && (dy > 0)) {
          m_MovingEdge = meTopRight;
        }
      }
      UpdateScene();
    }

    m_DragDelta->setP1(m_DragDelta->p2());


  // No dragging in progress. Just update mouse cursor.
  } else {
    m_MovingEdge = MouseDragPos(event);
    if (event->modifiers() == Qt::NoModifier) {
      UpdateCursor();
    }
  }
}

//==============================================================================

void ptRichRectInteraction::keyAction(QKeyEvent* event) {
  if (event->type() == QEvent::KeyPress) {
    switch (event->key()) {
      case Qt::Key_Escape:    // cancel interaction
        event->accept();
        m_ExitStatus = stFailure;
        Finalize();
        break;

      case Qt::Key_Enter:   // confirm rectangle and end interaction
      case Qt::Key_Return:
        event->accept();
        m_ExitStatus = stSuccess;
        Finalize();
        break;

      case Qt::Key_Alt:   // toggle LightsOut mode
        event->accept();
        setLightsOut((Settings->GetInt("LightsOut") + 1) % 3);
        break;

      default:
        // nothing to do here
        break;
    }
  }
}

//==============================================================================

void ptRichRectInteraction::UpdateCursor() {
  switch (m_MovingEdge) {
    case meOutside:
      FView->setCursor(Qt::ArrowCursor);
      break;
    case meNone:
      FView->setCursor(Qt::CrossCursor);
      break;
    case meCenter:
      FView->setCursor(Qt::SizeAllCursor);
      break;
    case meTop:
    case meBottom:
      FView->setCursor(Qt::SizeVerCursor);
      break;
    case meLeft:
    case meRight:
      FView->setCursor(Qt::SizeHorCursor);
      break;
    case meTopLeft:
    case meBottomRight:
      FView->setCursor(Qt::SizeFDiagCursor);
      break;
    case meTopRight:
    case meBottomLeft:
      FView->setCursor(Qt::SizeBDiagCursor);
      break;
    default:
      assert(!"Unhandled m_MovingEdge!");
      break;
  }
}

//==============================================================================

void ptRichRectInteraction::stop(ptStatus exitStatus) {
  m_ExitStatus = exitStatus;
  Finalize();
}

//==============================================================================

void ptRichRectInteraction::Finalize() {
  FView->scene()->removeItem(m_RectItem);
  // Do not delete m_RectItem here! It's still needed to report the size and
  // position of the rectangle to the caller. Deletion happens in the destructor.
  FView->setCursor(Qt::ArrowCursor);
  emit finished(m_ExitStatus);
}

//==============================================================================

void ptRichRectInteraction::moveToCenter(const short horizontal, const short vertical) {
  QPointF NewCenter = m_Rect.center();

  if (horizontal) {
    NewCenter.setX(FView->scene()->width() / 2);
  }
  if (vertical) {
    NewCenter.setY(FView->scene()->height() / 2);
  }

  m_Rect.moveCenter(NewCenter);
  m_RectItem->setRect(m_Rect);
  UpdateScene();
}

//==============================================================================

void ptRichRectInteraction::flipAspectRatio() {
  if (m_FixedAspectRatio) {
    setAspectRatio(1, m_AspectRatioH, m_AspectRatioW, 0);
  }
  QPointF center = m_Rect.center();
  qreal newH = m_Rect.width();
  qreal newW = m_Rect.height();
  m_Rect.setWidth(newW);
  m_Rect.setHeight(newH);
  m_Rect.moveCenter(center);
  ClampToScene();
  UpdateScene();
}

//==============================================================================

// Make sure rectangle stays inside sceneRect. Does not change AR, even
// when fixed AR is not set.
void ptRichRectInteraction::ClampToScene() {
  QPointF center = m_Rect.center();
  qreal arw = m_Rect.width();
  qreal arh = m_Rect.height();

  qreal MaxWidth = 2 * qMin(center.x() - FView->scene()->sceneRect().left(),
                            FView->scene()->sceneRect().right() - center.x());
  qreal MaxHeight = 2* qMin(center.y() - FView->scene()->sceneRect().top(),
                            FView->scene()->sceneRect().bottom() - center.y());

  if (m_Rect.width() > MaxWidth) {
    m_Rect.setWidth(MaxWidth);
    m_Rect.setHeight(MaxWidth * arh / arw);
    MaxHeight = qMin(MaxHeight, m_Rect.height());
  }

  if (m_Rect.height() > MaxHeight) {
    m_Rect.setHeight(MaxHeight);
    m_Rect.setWidth(MaxHeight * arw / arh);
  }

  m_Rect.moveCenter(center);
  m_RectItem->setRect(m_Rect);
}

//==============================================================================

// Make sure rectangle has the proper AR. Wenn adjusted the opposite
// corner/edge to the one that was moved remains fixed.
// When there was no movement (e.g. changed AR values in MainWindow)
// the center of the rectangle remains fixed.
void ptRichRectInteraction::EnforceAspectRatio(qreal dx /*= 0*/, qreal dy /*= 0*/) {
  dx = qAbs(dx);
  dy = qAbs(dy);
  qreal ImageRight = FView->scene()->sceneRect().right();
  qreal ImageBottom = FView->scene()->sceneRect().bottom();
  qreal NewWidth = m_Rect.height() * m_AspectRatio;
  qreal NewHeight = m_Rect.width() / m_AspectRatio;
  qreal EdgeCenter = 0.0;

  switch (m_MovingEdge){
    case meTopLeft:
      if (dx > dy) {  // primarily horizontal mouse movement: new width takes precedence
        m_Rect.setTop(m_Rect.top() +
                                  m_Rect.height() - NewHeight);
        if (m_Rect.top() < 0) {
          m_Rect.setTop(0);
          m_Rect.setLeft(m_Rect.right() - m_Rect.height() * m_AspectRatio);
        }
      } else {  // primarily vertical mouse movement: new height takes precedence
        m_Rect.setLeft(m_Rect.left() +
                                   m_Rect.width() - NewWidth);
        if (m_Rect.left() < 0) {
          m_Rect.setLeft(0);
          m_Rect.setTop(m_Rect.bottom() - m_Rect.width() / m_AspectRatio);
        }
      }
      break;

    case meTop:
      EdgeCenter = m_Rect.left() + m_Rect.width() / 2;
      m_Rect.setWidth(NewWidth);
      m_Rect.moveLeft(EdgeCenter - NewWidth / 2);
      if (m_Rect.right() > ImageRight) {
        m_Rect.setRight(ImageRight);
        m_Rect.setTop(m_Rect.bottom() - m_Rect.width() / m_AspectRatio);
      }
      if (m_Rect.left() < 0) {
        m_Rect.setLeft(0);
        m_Rect.setTop(m_Rect.bottom() - m_Rect.width() / m_AspectRatio);
      }
      break;

    case meTopRight:
      if (dx > dy) {
        m_Rect.setTop(m_Rect.top() + m_Rect.height() - NewHeight);
        if (m_Rect.top() < 0) {
          m_Rect.setTop(0);
          m_Rect.setRight(m_Rect.left() + m_Rect.height() * m_AspectRatio);
        }
      } else {
        m_Rect.setWidth(NewWidth);
        if (m_Rect.right() > ImageRight) {
          m_Rect.setRight(ImageRight);
          m_Rect.setTop(m_Rect.bottom() - m_Rect.width() / m_AspectRatio);
        }
      }
      break;

    case meRight:
      EdgeCenter = m_Rect.top() + m_Rect.height() / 2;
      m_Rect.setHeight(NewHeight);
      m_Rect.moveTop(EdgeCenter - NewHeight / 2);
      if (m_Rect.bottom() > ImageBottom) {
        m_Rect.setBottom(ImageBottom);
        m_Rect.setRight(m_Rect.left() + m_Rect.height() * m_AspectRatio);
      }
      if (m_Rect.top() < 0) {
        m_Rect.setTop(0);
        m_Rect.setRight(m_Rect.left() + m_Rect.height() * m_AspectRatio);
      }
      break;

    case meBottomRight:
      if (dx > dy) {
        m_Rect.setBottom(m_Rect.bottom() + NewHeight - m_Rect.height());
        if (m_Rect.bottom() > ImageBottom) {
          m_Rect.setBottom(ImageBottom);
          m_Rect.setRight(m_Rect.left() + m_Rect.height() * m_AspectRatio);
        }
      } else {
        m_Rect.setWidth(NewWidth);
        if (m_Rect.right() > ImageRight) {
          m_Rect.setRight(ImageRight);
          m_Rect.setTop(m_Rect.bottom() - m_Rect.width() / m_AspectRatio);
        }
      }
      break;

    case meBottom:
      EdgeCenter = m_Rect.left() + m_Rect.width() / 2;
      m_Rect.setWidth(NewWidth);
      m_Rect.moveLeft(EdgeCenter - NewWidth / 2);
      if (m_Rect.right() > ImageRight) {
        m_Rect.setRight(ImageRight);
        m_Rect.setBottom(m_Rect.top() + m_Rect.width() / m_AspectRatio);
      }
      if (m_Rect.left() < 0) {
        m_Rect.setLeft(0);
        m_Rect.setBottom(m_Rect.top() + m_Rect.width() / m_AspectRatio);
      }
      break;

    case meBottomLeft:
      if (dx > dy) {
        m_Rect.setBottom(m_Rect.bottom() + NewHeight - m_Rect.height());
        if (m_Rect.bottom() > ImageBottom) {
          m_Rect.setBottom(ImageBottom);
          m_Rect.setLeft(m_Rect.right() - m_Rect.height() * m_AspectRatio);
        }
      } else {
        m_Rect.setLeft(m_Rect.left() + m_Rect.width() - NewWidth);
        if (m_Rect.left() < 0) {
          m_Rect.setLeft(0);
          m_Rect.setTop(m_Rect.bottom() - m_Rect.width() / m_AspectRatio);
        }
      }
      break;

    case meLeft:
      EdgeCenter = m_Rect.top() + m_Rect.height() / 2;
      m_Rect.setHeight(NewHeight);
      m_Rect.moveTop(EdgeCenter - NewHeight / 2);
      if (m_Rect.bottom() > ImageBottom) {
        m_Rect.setBottom(ImageBottom);
        m_Rect.setLeft(m_Rect.right() - m_Rect.height() * m_AspectRatio);
      }
      if (m_Rect.top() < 0) {
        m_Rect.setTop(0);
        m_Rect.setLeft(m_Rect.right() - m_Rect.height() * m_AspectRatio);
      }
      break;

    case meNone: {
      // Calculate rect with new AR and about the same area as the old one according to:
      // OldWidth * OldHeight = x * NewARW * x * NewARH
      // x = sqrt((OldWidth * OldHeight) / (NewARW * NewARH))
      // Center point stays the same.
      QPointF center = m_Rect.center();
      float x = sqrt((m_Rect.width() * m_Rect.height()) / (m_AspectRatioW * m_AspectRatioH));
      m_Rect.setWidth(x * m_AspectRatioW);
      m_Rect.setHeight(x * m_AspectRatioH);
      m_Rect.moveCenter(center);
      ClampToScene();
      break;
    }

    default:
      // nothing to do
      break;
  }

  m_RectItem->setRect(m_Rect);
}

//==============================================================================

// Returns the area of the crop/selection rectangle the mouse cursor hovers over.
// The mouse position inside the crop rectangle determines which action is performed
// on drag. There are nine areas: 55511111111666
//                                444        222
//                                77733333333888
// - Dragging the corners changes both adjacent edges.
// - Dragging the edges changes only that edge. Dragging in the middle area moves
//   the rectangle without changing its size.
// - Dragging beyond actual image borders is not possible.
// - Edge areas are usually EdgeThickness pixels thick. On hovering mouse cursor
//   changes shape to indicate the move/resize mode.
// - For rectangle edges of TinyRectThreshold pixels or shorter only the corner modes
//   apply, one for each half of the edge. This avoids too tiny interaction areas.
ptMovingEdge ptRichRectInteraction::MouseDragPos(const QMouseEvent* event) {
  QPointF pos(FView->mapToScene(event->pos()));

  // Catch mouse outside current crop rect
  if (!m_RectItem->contains(pos)) {
    if (FView->scene()->sceneRect().contains(pos)) {
      return meNone;
    } else {
      return meOutside;
    }
  }

  ptMovingEdge HoverOver = meNone;
  int TBthick = 0;    // top/bottom
  int LRthick = 0;    // left/right

  // Determine edge area thickness
  if (m_Rect.height() <= (int)(TinyRectThreshold / FView->transform().m11())) {
    TBthick = (int)(m_Rect.height() / 2);
  } else {
    TBthick = (int)(EdgeThickness / FView->transform().m11());
  }
  if (m_Rect.width() <= (int)(TinyRectThreshold / FView->transform().m11())) {
    LRthick = (int)(m_Rect.width() / 2);
  } else {
    LRthick = (int)(EdgeThickness / FView->transform().m11());
  }

  // Determine in which area the mouse is
  if (m_Rect.bottom() - pos.y() <= TBthick) {
    HoverOver = meBottom;
  } else if (pos.y() - m_Rect.top() <= TBthick) {
    HoverOver = meTop;
  } else {
    HoverOver = meCenter;
  }

  if (m_Rect.right() - pos.x() <= LRthick) {
    if (HoverOver == meBottom) {
      HoverOver = meBottomRight;
    } else if (HoverOver == meTop) {
      HoverOver = meTopRight;
    } else {
      HoverOver = meRight;
    }

  } else if (pos.x() - m_Rect.left() <= LRthick) {
    if (HoverOver == meBottom) {
      HoverOver = meBottomLeft;
    } else if (HoverOver == meTop) {
      HoverOver = meTopLeft;
    } else {
      HoverOver = meLeft;
    }
  }

  return HoverOver;
}

//==============================================================================

// Calc new corner point, change moving edge/corner when rect is
// dragged into another "quadrant", and update rectangle.
void ptRichRectInteraction::RecalcRect() {
  QRectF NewRect = m_Rect;
  qreal ImageRight = FView->scene()->sceneRect().right();
  qreal ImageBottom = FView->scene()->sceneRect().bottom();
  qreal dx = m_DragDelta->dx() / FView->transform().m11();
  qreal dy = m_DragDelta->dy() / FView->transform().m11();
  qreal dxCeil = dx;
  qreal dyCeil = dy;
  qreal dxFloor = dx;
  qreal dyFloor = dy;

  switch (m_MovingEdge) {
    case meTopLeft:
      // Calc preliminary new position of moved corner point. Don't allow it to move
      // beyond the actual image.
      NewRect.setLeft(qBound(0.0, m_Rect.left() + dxFloor, ImageRight));
      NewRect.setTop(qBound(0.0, m_Rect.top() + dyFloor, ImageBottom));
      // Determine if moving corner changed
      if ((NewRect.left() > m_Rect.right()) && (NewRect.top() <= m_Rect.bottom())) {
        m_MovingEdge = meTopRight;
      } else if ((NewRect.left() > m_Rect.right()) && (NewRect.top() >= m_Rect.bottom())) {
        m_MovingEdge = meBottomRight;
      } else if ((NewRect.left() <= m_Rect.right()) && (NewRect.top() > m_Rect.bottom())) {
        m_MovingEdge = meBottomLeft;
      }
      // Update crop rect
      m_Rect = NewRect.normalized();
      break;

    case meTop:
      dx = 0;
      NewRect.setTop(qBound(0.0, m_Rect.top() + dyFloor, ImageBottom));
      if (NewRect.top() > m_Rect.bottom()) {
        m_MovingEdge = meBottom;
      }
      m_Rect = NewRect.normalized();
      break;

    case meTopRight:
      NewRect.setRight(qBound(0.0, m_Rect.right() + dxCeil, ImageRight));
      NewRect.setTop(qBound(0.0, m_Rect.top() + dyFloor, ImageBottom));
      if ((NewRect.right() < m_Rect.left()) && (NewRect.top() <= m_Rect.bottom())) {
        m_MovingEdge = meTopLeft;
      } else if ((NewRect.right() < m_Rect.left()) && (NewRect.top() > m_Rect.bottom())) {
        m_MovingEdge = meBottomLeft;
      } else if ((NewRect.right() >= m_Rect.left()) && (NewRect.top() > m_Rect.bottom())) {
        m_MovingEdge = meBottomRight;
      }
      m_Rect = NewRect.normalized();
      break;

    case meRight:
      dy = 0;
      NewRect.setRight(qBound(0.0, m_Rect.right() + dxCeil, ImageRight));
      if ((NewRect.right() < m_Rect.left())) {
        m_MovingEdge = meLeft;
      }
      m_Rect = NewRect.normalized();
      break;

    case meBottomRight:
      NewRect.setRight(qBound(0.0, m_Rect.right() + dxCeil, ImageRight));
      NewRect.setBottom(qBound(0.0, m_Rect.bottom() + dyCeil, ImageBottom));
      if ((NewRect.right() < m_Rect.left()) && (NewRect.bottom() >= m_Rect.top())) {
        m_MovingEdge = meBottomLeft;
      } else if ((NewRect.right() < m_Rect.left()) && (NewRect.bottom() < m_Rect.top())) {
        m_MovingEdge = meTopLeft;
      } else if ((NewRect.right() >= m_Rect.left()) && (NewRect.bottom() < m_Rect.top())) {
        m_MovingEdge = meTopRight;
      }
      m_Rect = NewRect.normalized();
      break;

    case meBottom:
      dx = 0;
      NewRect.setBottom(qBound(0.0, m_Rect.bottom() + dy, ImageBottom));
      if (NewRect.bottom() < m_Rect.top()) {
        m_MovingEdge = meTop;
      }
      m_Rect = NewRect.normalized();
      break;

    case meBottomLeft:
      NewRect.setLeft(qBound(0.0, m_Rect.left() + dx, ImageRight));
      NewRect.setBottom(qBound(0.0, m_Rect.bottom() + dy, ImageBottom));
      if ((NewRect.left() > m_Rect.right()) && (NewRect.bottom() >= m_Rect.top())) {
        m_MovingEdge = meBottomRight;
      } else if ((NewRect.left() > m_Rect.right()) && (NewRect.bottom() < m_Rect.top())) {
        m_MovingEdge = meTopRight;
      } else if ((NewRect.left() <= m_Rect.right()) && (NewRect.bottom() < m_Rect.top())) {
        m_MovingEdge = meTopLeft;
      }
      m_Rect = NewRect.normalized();
      break;

    case meLeft:
      dy = 0;
      NewRect.setLeft(qBound(0.0, m_Rect.left() + dx, ImageRight));
      if (NewRect.left() > m_Rect.right()) {
        m_MovingEdge = meRight;
      }
      m_Rect = NewRect.normalized();
      break;

    default:
      // nothing to do here
      break;
  }

  if (m_FixedAspectRatio) {
    EnforceAspectRatio(dx, dy);    // also updates m_RectItem
  } else {
    m_RectItem->setRect(m_Rect);
  }
}

//==============================================================================

void ptRichRectInteraction::RecalcGuides() {
  // Draw no guides when LightsOut is on "black" setting. In that case calc
  // lines but set them all to invisible.
  bool ShowLines = Settings->GetInt("LightsOut") != ptLightsOutMode_Black;

  switch (m_Guides) {
    case ptGuidelines_None: {
      for (int i = 0; i <= 3; i++) {
        m_GuideItems[i]->hide();
      }
      break;
    }


    case ptGuidelines_RuleThirds: {
      qreal HeightThird = m_Rect.height() / 3.0;
      qreal WidthThird = m_Rect.width() / 3.0;

      m_GuideItems[0]->setLine(m_Rect.left() + WidthThird,
                               m_Rect.top(),
                               m_Rect.left() + WidthThird,
                               m_Rect.bottom());
      m_GuideItems[1]->setLine(m_Rect.right() - WidthThird,
                               m_Rect.top(),
                               m_Rect.right() - WidthThird,
                               m_Rect.bottom());
      m_GuideItems[2]->setLine(m_Rect.left(),
                               m_Rect.top() + HeightThird,
                               m_Rect.right(),
                               m_Rect.top() + HeightThird);
      m_GuideItems[3]->setLine(m_Rect.left(),
                               m_Rect.bottom() - HeightThird,
                               m_Rect.right(),
                               m_Rect.bottom() - HeightThird);

      for (int i = 0; i <= 3; i++) {
        m_GuideItems[i]->setVisible(ShowLines);
      }
      break;
    }


    case ptGuidelines_GoldenRatio: {
      qreal ShortWidth = m_Rect.width() * 5.0/13.0;
      qreal ShortHeight = m_Rect.height() * 5.0/13.0;

      m_GuideItems[0]->setLine(m_Rect.left() + ShortWidth,
                               m_Rect.top(),
                               m_Rect.left() + ShortWidth,
                               m_Rect.bottom());
      m_GuideItems[1]->setLine(m_Rect.right() - ShortWidth,
                               m_Rect.top(),
                               m_Rect.right() - ShortWidth,
                               m_Rect.bottom());
      m_GuideItems[2]->setLine(m_Rect.left(),
                               m_Rect.top() + ShortHeight,
                               m_Rect.right(),
                               m_Rect.top() + ShortHeight);
      m_GuideItems[3]->setLine(m_Rect.left(),
                               m_Rect.bottom() - ShortHeight,
                               m_Rect.right(),
                               m_Rect.bottom() - ShortHeight);

      for (int i = 0; i <= 3; i++) {
        m_GuideItems[i]->setVisible(ShowLines);
      }
      break;
    }


    case ptGuidelines_Diagonals: {
      qreal length = m_Rect.width() > m_Rect.height()
                     ? m_Rect.height()
                     : m_Rect.width();

      m_GuideItems[0]->setLine(m_Rect.left(),
                               m_Rect.top(),
                               m_Rect.left() + length,
                               m_Rect.top() + length);
      m_GuideItems[1]->setLine(m_Rect.right(),
                               m_Rect.bottom(),
                               m_Rect.right() - length + 1,
                               m_Rect.bottom() - length + 1);
      m_GuideItems[2]->setLine(m_Rect.left(),
                               m_Rect.bottom(),
                               m_Rect.left() + length,
                               m_Rect.bottom() - length + 1);
      m_GuideItems[3]->setLine(m_Rect.right(),
                               m_Rect.top(),
                               m_Rect.right() - length + 1,
                               m_Rect.top() + length);

      for (int i = 0; i <= 3; i++) {
        m_GuideItems[i]->setVisible(ShowLines);
      }
      break;
    }


    case ptGuidelines_Centerlines: {
      m_GuideItems[0]->setLine(m_Rect.center().x(),
                               m_Rect.top(),
                               m_Rect.center().x(),
                               m_Rect.bottom());
      m_GuideItems[1]->setLine(m_Rect.left(),
                               m_Rect.center().y(),
                               m_Rect.right(),
                               m_Rect.center().y());

      m_GuideItems[0]->setVisible(ShowLines);
      m_GuideItems[1]->setVisible(ShowLines);
      m_GuideItems[2]->hide();
      m_GuideItems[3]->hide();
    }


    default:
      // nothing to do here
      break;
  }
}

//==============================================================================

// Array index order: top, right, bottom, left
// Up to four rectangles are drawn according to the following figure.
// tttttttttttttttttt
// lll            rrr
// lll            rrr
// bbbbbbbbbbbbbbbbbb
void ptRichRectInteraction::RecalcLightsOutRects() {
  m_LightsOutRects[0]->setRect(    // top
      FView->scene()->sceneRect().left(),
      FView->scene()->sceneRect().top(),
      FView->scene()->sceneRect().width(),
      m_Rect.top() - FView->scene()->sceneRect().top()
  );

  m_LightsOutRects[1]->setRect(    // right
      m_Rect.right(),
      m_Rect.top(),
      FView->scene()->sceneRect().right() - m_Rect.right(),
      m_Rect.height()
  );

  m_LightsOutRects[2]->setRect(    // bottom
      FView->scene()->sceneRect().left(),
      m_Rect.bottom(),
      FView->scene()->sceneRect().width(),
      FView->scene()->sceneRect().bottom() - m_Rect.bottom()
  );

  m_LightsOutRects[3]->setRect(    // left
      FView->scene()->sceneRect().left(),
      m_Rect.top(),
      m_Rect.left() - FView->scene()->sceneRect().left(),
      m_Rect.height()
  );
}

//==============================================================================

void ptRichRectInteraction::UpdateScene() {
  FView->blockSignals(1);
  RecalcRect();
  RecalcGuides();
  RecalcLightsOutRects();
  FView->blockSignals(0);
  //FView->repaint();
}

//==============================================================================
