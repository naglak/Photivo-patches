/*******************************************************************************
**
** Photivo
**
** Copyright (C) 2011 Sergey Salnikov <salsergey@gmail.com>
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

#ifndef PTSLIDER_H
#define PTSLIDER_H

#include <QSlider>
#include <QSpinBox>
#include <QVariant>

class ptSlider : public QSlider
{
  Q_OBJECT
public:
  explicit ptSlider(QWidget *parent         = 0,
                    const QString  Label    = "ptSlider",
                    const QString  ToolTip  = "ptSlider",
                    const QVariant::Type Type = QVariant::Int,
                    const QVariant Minimum  = 0,
                    const QVariant Maximum  = 100,
                    const QVariant Default  = 0,
                    const QVariant Step     = 1,
                    const int      Decimals = 0);
  void init(const QString  Label             = "ptSlider",
            const QString  ToolTip  = "ptSlider",
            const QVariant::Type Type = QVariant::Int,
            const QVariant Minimum  = 0,
            const QVariant Maximum  = 100,
            const QVariant Default  = 0,
            const QVariant Step     = 1,
            const int      Decimals = 0);

  QVariant Minimum() const { return m_Minimum; }
  QVariant Maximum() const { return m_Maximum; }
  void SetMinimum(const QVariant min);
  void SetMaximum(const QVariant max);

  void enableEditor(bool e, bool save = true);
  void hoverMoveEvent(QHoverEvent *ev);
  void initValueRect();

protected:
  bool event(QEvent *event);
  bool eventFilter(QObject *obj, QEvent *ev);
  void keyPressEvent(QKeyEvent *ev);
  void mouseMoveEvent(QMouseEvent *ev);
  void mousePressEvent(QMouseEvent *ev);
  void mouseReleaseEvent(QMouseEvent *ev);
  void paintEvent(QPaintEvent *ev);
  void resizeEvent(QResizeEvent *ev);
  void wheelEvent(QWheelEvent *ev);

signals:
  void editingFinished();
  void valueChanged(QVariant value);

public slots:
  void setValue(int val);
  void setValue(double val);

private slots:
  void setValue(QVariant val);

private:
  QString m_Label, m_ToolTip;
  QVariant::Type m_Type;
  QVariant m_Minimum, m_Maximum, m_Value, m_Step, m_Default;
  short m_Decimals;
  bool m_IsSliderMoving, m_IsEditingEnabled;
  QRect m_ValueRect;
  QAbstractSpinBox *m_EditBox;
  short m_HandleWidth;
};

#endif // PTSLIDER_H
