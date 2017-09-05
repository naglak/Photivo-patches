/*******************************************************************************
**
** Photivo
**
** Copyright (C) 2008 Jos De Laender <jos.de_laender@telenet.be>
** Copyright (C) 2009,2010 Michael Munzert <mail@mm-log.com>
** Copyright (C) 2012 Bernd Schoeler <brjohn@brother-john.net>
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

#include "ptInput.h"
#include "ptInfo.h"
#include "filters/ptCfgItem.h"
#include "ptConstants.h"

#include <QMessageBox>
#include <QApplication>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QTimer>

#include <cassert>

//==============================================================================

ptInput::ptInput(const QWidget* MainWindow,
                 const QString  ObjectName,
                 const QString  ParentName,
                 const short    HasSlider,
                 const short    ColorSetting,
                 const short    HasDefaultValue,
                 const QVariant Default,
                 const QVariant Minimum,
                 const QVariant Maximum,
                 const QVariant Step,
                 const int      Decimals,
                 const QString  LabelText,
                 const QString  ToolTip,
                 const int      TimeOut)
: ptWidget(nullptr),
  m_SpinBox(nullptr)
{
  m_Type = Minimum.type();
  setObjectName(ObjectName);
  CheckTypes(Default, Minimum, Maximum, Step);

  m_HaveDefault = HasDefaultValue;
  m_HaveSlider = HasSlider;
  m_DefaultValue = Default;

  m_SettingsName = ObjectName;
  m_SettingsName.chop(5);

  m_Parent = MainWindow->findChild <QWidget*> (ParentName);

  if (!m_Parent) {
    fprintf(stderr,"(%s,%d) Could not find '%s'. Aborting\n",
           __FILE__,__LINE__,ParentName.toLocal8Bit().data());
    assert(m_Parent);
  }
  setParent(m_Parent);

  createGUI(Minimum, Maximum, Step, Decimals, ToolTip, LabelText, ColorSetting, TimeOut);
}

//==============================================================================

ptInput::ptInput(const ptCfgItem &ACfgItem, QWidget *AParent)
: ptWidget(AParent),
  m_SpinBox(nullptr)
{
  init(ACfgItem);
}

//==============================================================================

ptInput::ptInput(QWidget *AParent)
: ptWidget(AParent),
  m_SpinBox(nullptr)
{}

//==============================================================================

void ptInput::init(const ptCfgItem &ACfgItem) {
  GInfo->Assert(this->parent(), "Parent cannot be null.", AT);
  GInfo->Assert(!m_SpinBox, "Trying to initialize an already initalized input widget.", AT);
  if (!(ACfgItem.Type == ptCfgItem::SpinEdit || ACfgItem.Type == ptCfgItem::Slider ||
        ACfgItem.Type == ptCfgItem::HueSlider))
  {
    GInfo->Raise(QString("Wrong GUI type (%1). Must be a spin edit or slider.").arg(ACfgItem.Type), AT);
  }

  FIsNewSchool    = true;
  m_Type          = ACfgItem.Min.type();
  m_HaveDefault   = true;
  m_HaveSlider    = (ACfgItem.Type == ptCfgItem::Slider) || (ACfgItem.Type == ptCfgItem::HueSlider);
  m_SettingsName  = "";  // not used
  m_Parent        = this;
  m_DefaultValue  = ACfgItem.Default;

  this->setObjectName(ACfgItem.Id);
  CheckTypes(ACfgItem.Default, ACfgItem.Min, ACfgItem.Max, ACfgItem.StepSize);
  createGUI(ACfgItem.Min, ACfgItem.Max, ACfgItem.StepSize, ACfgItem.Decimals,
            ACfgItem.ToolTip, ACfgItem.Caption, (ACfgItem.Type == ptCfgItem::HueSlider),
            ptTimeout_Input);
}

//==============================================================================

void ptInput::createGUI(const QVariant &Minimum, const QVariant &Maximum, const QVariant &Step,
                        const int Decimals, const QString &ToolTip, const QString &LabelText,
                        const short ColorSetting, const int TimeOut)
{
  QHBoxLayout *Layout = new QHBoxLayout(m_Parent);
  m_Parent->setLayout(Layout);

  if (m_Type == QVariant::Int) {
    m_SpinBox = new QSpinBox(m_Parent);
    QSpinBox* IntSpinBox = qobject_cast <QSpinBox *> (m_SpinBox);
    IntSpinBox->setMinimum(Minimum.toInt());
    IntSpinBox->setMaximum(Maximum.toInt());
    IntSpinBox->setSingleStep(Step.toInt());
  } else if (m_Type == QVariant::Double) {
    m_SpinBox = new QDoubleSpinBox(m_Parent);
    QDoubleSpinBox* DoubleSpinBox = qobject_cast <QDoubleSpinBox *> (m_SpinBox);
    DoubleSpinBox->setMinimum(Minimum.toDouble());
    DoubleSpinBox->setMaximum(Maximum.toDouble());
    DoubleSpinBox->setSingleStep(Step.toDouble());
    DoubleSpinBox->setDecimals(Decimals);
  } else {
    assert (m_Type == QVariant::Int || m_Type == QVariant::Double);
  }

  m_SpinBox->setToolTip(ToolTip);
  m_SpinBox->setFixedHeight(24);
  m_SpinBox->setAlignment(Qt::AlignRight);
  m_SpinBox->installEventFilter(this);
  m_SpinBox->setFocusPolicy(Qt::ClickFocus);

  m_Slider = new ptSlider(m_Parent, LabelText, ToolTip, m_Type, Minimum, Maximum, m_DefaultValue,
                          Step, Decimals);
  m_Slider->setMinimumWidth(120);
//  m_Slider->setMaximumWidth(250);
  m_Slider->installEventFilter(this);
  m_Slider->setFocusPolicy(Qt::ClickFocus);

  m_Label = new QLabel(m_Parent);
  m_Label->setText(LabelText);
  m_Label->setToolTip(ToolTip);
  m_Label->setTextInteractionFlags(Qt::NoTextInteraction);
  m_Label->installEventFilter(this);

  Layout->addWidget(m_SpinBox);

  QVBoxLayout *colorLayout=NULL;
  if (ColorSetting == 1) {
//  this slider has a hue bar
    colorLayout=new QVBoxLayout;
    Layout->addLayout(colorLayout);
    colorLayout->addWidget(m_Slider);
    QWidget *wdg=new QWidget;
    wdg->setObjectName("HueWidget");
    colorLayout->addWidget(wdg);
    colorLayout->setContentsMargins(0,0,0,0);
    colorLayout->setMargin(0);
    colorLayout->setSpacing(0);
  } else {
    Layout->addWidget(m_Slider);
  }

  Layout->addWidget(m_Label);
  Layout->setContentsMargins(0,0,0,0);
  Layout->setMargin(0);

  if (m_HaveSlider) {
    m_SpinBox->hide();
    m_Label->hide();
    Layout->setAlignment(Qt::AlignLeft);
  } else {
    m_Slider->hide();
    Layout->addStretch(1);
    Layout->setSpacing(2);
  }

  // A timer for time filtering signals going outside.
  // Bad coding but fun ;-) (mike)
  if (this->objectName() == "CropExposureInput")
    m_TimeOut = 5;
  else
    m_TimeOut = TimeOut;

  m_KeyTimeOut = 2000;
  m_Timer = new QTimer(this);
  m_Timer->setSingleShot(1);

  connect(m_Timer,SIGNAL(timeout()),
          this,SLOT(OnValueChangedTimerExpired()));

  // Linking values together.
  if (m_Type == QVariant::Int) {
    connect(m_SpinBox,SIGNAL(valueChanged(int)),
            this,SLOT(OnSpinBoxChanged(int)));
  }
  if (m_Type == QVariant::Double) {
    connect(m_SpinBox,SIGNAL(valueChanged(double)),
            this,SLOT(OnSpinBoxChanged(double)));
  }
  connect(m_Slider,SIGNAL(valueChanged(QVariant)),
          this,SLOT(OnSliderChanged(QVariant)));
  connect(m_SpinBox,SIGNAL(editingFinished()),
          this,SLOT(EditingFinished()));

  // Set the default Value (and remember for later).
  SetValue(m_DefaultValue,1 /* block signals */);
  m_Emited = 1;
}

//==============================================================================

ptInput::~ptInput() {
  /* nothing to do here*/
}

//==============================================================================

void ptInput::setValue(const QVariant &AValue) {
  this->SetValue(AValue, 1);
}

//==============================================================================

void ptInput::SetValue(const QVariant Value,
                       const short    BlockSignal)
{
  // Hack for QT 4.6 update:
  // QVariant has now a new implicit constructor that takes a float. This
  // means that code that assigned a float to a variant would create a
  // variant with userType QMetaType::Float, instead of QVariant::Double.
  QVariant TempValue = Value;
  if (static_cast<QMetaType::Type>(TempValue.type()) == QMetaType::Float)
    TempValue.convert(QVariant::Double);

  // For the double->int implication , I don't want to use canConvert.
  if ( (TempValue.type() == QVariant::Double && m_Type == QVariant::Int)  ||
       (TempValue.type() == QVariant::Double && m_Type == QVariant::UInt) ||
       (TempValue.type() == QVariant::Int    && m_Type == QVariant::Double) ||
       (TempValue.type() != QVariant::Int  &&
        TempValue.type() != QVariant::UInt &&
        TempValue.type() != QVariant::Double) ) {
    printf("(%s,%d) this : %s Value.type() : %d m_Type : %d value : %s\n",
           __FILE__,__LINE__,
           this->objectName().toLocal8Bit().data(),
           TempValue.type(),
           m_Type,
           TempValue.toString().toLocal8Bit().data());
    assert(TempValue.type() == m_Type);
  }

  m_Value = TempValue;
  m_SpinBox->blockSignals(BlockSignal);
  m_Slider->blockSignals(BlockSignal);
  if (m_Type == QVariant::Int) {
     QSpinBox* IntSpinBox = qobject_cast <QSpinBox *> (m_SpinBox);
     IntSpinBox->setValue(Value.toInt());
     m_Slider->setValue(//(int)
       (/*0.5 +*/ 100.0*(Value.toInt() - IntSpinBox->minimum()) /
          (IntSpinBox->maximum() - IntSpinBox->minimum())) );
  } else if (m_Type == QVariant::Double) {
    QDoubleSpinBox* DoubleSpinBox = qobject_cast <QDoubleSpinBox *> (m_SpinBox);
    DoubleSpinBox->setValue(Value.toDouble());
    m_Slider->setValue(//(int)
      (/*0.5 +*/ 100.0*(Value.toDouble() - DoubleSpinBox->minimum()) /
        (DoubleSpinBox->maximum() - DoubleSpinBox->minimum())) );
  } else {
    assert(0);
  }
  m_SpinBox->blockSignals(0);
  m_Slider->blockSignals(0);
}

//==============================================================================

void ptInput::SetMinimum(const QVariant Value) {
  if (Value.type() != m_Type) {
    printf("(%s,%d) this : %s Value.type() : %d m_Type : %d\n",
           __FILE__,__LINE__,
           this->objectName().toLocal8Bit().data(),
           Value.type(),
           m_Type);
    assert(Value.type() == m_Type);
  }

  if (m_Type == QVariant::Int) {
    QSpinBox* IntSpinBox = qobject_cast <QSpinBox *> (m_SpinBox);
    IntSpinBox->setMinimum(Value.toInt());
  }
  if (m_Type == QVariant::Double) {
    QDoubleSpinBox* DoubleSpinBox = qobject_cast <QDoubleSpinBox *> (m_SpinBox);
    DoubleSpinBox->setMinimum(Value.toDouble());
  }
  m_Slider->SetMinimum(Value);
}

//==============================================================================

void ptInput::SetMaximum(const QVariant Value) {
  if (Value.type() != m_Type) {
    printf("(%s,%d) this : %s Value.type() : %d m_Type : %d\n",
           __FILE__,__LINE__,
           this->objectName().toLocal8Bit().data(),
           Value.type(),
           m_Type);
    assert(Value.type() == m_Type);
  }

  if (m_Type == QVariant::Int) {
    QSpinBox* IntSpinBox = qobject_cast <QSpinBox *> (m_SpinBox);
    IntSpinBox->setMaximum(Value.toInt());
  }
  if (m_Type == QVariant::Double) {
    QDoubleSpinBox* DoubleSpinBox = qobject_cast <QDoubleSpinBox *> (m_SpinBox);
    DoubleSpinBox->setMaximum(Value.toDouble());
  }
  m_Slider->SetMaximum(Value);
}

//==============================================================================

void ptInput::SetEnabled(const short Enabled) {
  m_SpinBox->setEnabled(Enabled);
  m_Slider->setEnabled(Enabled);
}

//==============================================================================

void ptInput::Show(const short Show) {
  m_Parent->setVisible(Show);
}

//==============================================================================

void ptInput::Reset() {
  m_SpinBox->clearFocus();
  m_Slider->clearFocus();
  SetValue(m_DefaultValue,0 /* Generate signal */);
}

//==============================================================================

// Set the slider. Int and Double variant.
void ptInput::OnSpinBoxChanged(int Value) {
  QSpinBox* IntSpinBox = qobject_cast <QSpinBox *> (m_SpinBox);
  m_Slider->blockSignals(true);
  m_Slider->setValue(//(int)
    (100.0*(Value - IntSpinBox->minimum()) /
       (IntSpinBox->maximum() - IntSpinBox->minimum()))  );
  m_Slider->blockSignals(false);
  m_Value = Value;
  OnValueChanged(Value);
}

//==============================================================================

void ptInput::OnSpinBoxChanged(double Value) {
  QDoubleSpinBox* DoubleSpinBox = qobject_cast <QDoubleSpinBox *> (m_SpinBox);
  m_Slider->blockSignals(true);
  m_Slider->setValue(//(int)
    (/*0.5 +*/ 100.0*(Value - DoubleSpinBox->minimum()) /
       (DoubleSpinBox->maximum() - DoubleSpinBox->minimum()))  );
  m_Slider->blockSignals(false);
  m_Value = Value;
  OnValueChanged(Value);
}

//==============================================================================

// Set the spinbox.
/*
void ptInput::OnSliderChanged(int Value) {
  if (m_Type == QVariant::Int) {
    QSpinBox* IntSpinBox = qobject_cast <QSpinBox *> (m_SpinBox);
    IntSpinBox->blockSignals(true);
    IntSpinBox->setValue(
      (int)(IntSpinBox->minimum() +
            Value/100.0 * (IntSpinBox->maximum() - IntSpinBox->minimum())));
    IntSpinBox->blockSignals(false);
    OnValueChanged(IntSpinBox->value());
  }
  if (m_Type == QVariant::Double) {
    QDoubleSpinBox* DoubleSpinBox = qobject_cast <QDoubleSpinBox *> (m_SpinBox);
    DoubleSpinBox->blockSignals(true);
    DoubleSpinBox->setValue(
      DoubleSpinBox->minimum() +
      Value/100.0 * (DoubleSpinBox->maximum() - DoubleSpinBox->minimum()));
    DoubleSpinBox->blockSignals(false);
    OnValueChanged(DoubleSpinBox->value());
  }
}
*/
void ptInput::OnSliderChanged(QVariant Value) {
  m_Value=Value;
  if (m_Type == QVariant::Int) {
    QSpinBox* IntSpinBox = qobject_cast <QSpinBox *> (m_SpinBox);
    IntSpinBox->blockSignals(true);
    IntSpinBox->setValue(Value.toInt());
    IntSpinBox->blockSignals(false);
    OnValueChanged(IntSpinBox->value());
  }
  if (m_Type == QVariant::Double) {
    QDoubleSpinBox* DoubleSpinBox = qobject_cast <QDoubleSpinBox *> (m_SpinBox);
    DoubleSpinBox->blockSignals(true);
    DoubleSpinBox->setValue(Value.toDouble());
    DoubleSpinBox->blockSignals(false);
    OnValueChanged(DoubleSpinBox->value());
  }
}

//==============================================================================

// Reset to the defaults.
void ptInput::OnButtonClicked() {
  m_SpinBox->clearFocus();
  SetValue(m_DefaultValue,0 /* Generate signal */);
}

//==============================================================================

void ptInput::EditingFinished() {
  if (m_Emited == 0) {
    OnValueChangedTimerExpired();
  }
}

//==============================================================================

void ptInput::OnValueChanged(int Value) {
  m_Emited = 0;
  m_Value = QVariant(Value);
  if (m_TimeOut) {
    if(!m_SpinBox->hasFocus()) {
      m_Timer->start(m_TimeOut);
    } else {
      // m_Timer->start(m_KeyTimeOut);
    }
  } else {
    OnValueChangedTimerExpired();
  }
}

//==============================================================================

void ptInput::OnValueChanged(double Value) {
  m_Emited = 0;
  m_Value = QVariant(Value);
  if (m_TimeOut) {
    if(!m_SpinBox->hasFocus()) {
      m_Timer->start(m_TimeOut);
    } else {
      // m_Timer->start(m_KeyTimeOut);
    }
  } else {
    OnValueChangedTimerExpired();
  }
}

//==============================================================================

void ptInput::OnValueChangedTimerExpired() {
  if (m_Emited == 0) {
    if (m_Type == QVariant::Int)
      printf("(%s,%d) emitting signal(%d)\n",__FILE__,__LINE__,m_Value.toInt());
    if (m_Type == QVariant::Double)
      printf("(%s,%d) emitting signal(%f)\n",__FILE__,__LINE__,m_Value.toDouble());

    if (FIsNewSchool){
      emit ptWidget::valueChanged(this->objectName(), m_Value);
    } else {
      emit this->valueChanged(m_Value);
    }
  }
  m_Emited = 1;
  m_SpinBox->clearFocus();
}

//==============================================================================

bool ptInput::eventFilter(QObject *obj, QEvent *event)
{
  if (event->type() == QEvent::ContextMenu) {
    if (m_HaveDefault && m_SpinBox->isEnabled() && !m_HaveSlider)
      OnButtonClicked();
    return true;

  } else if (0 && ((QMouseEvent*) event)->button()==Qt::RightButton) {
    if (m_HaveDefault)
      OnButtonClicked();
    return true;

  } else if (event->type() == QEvent::Wheel
             && ((QMouseEvent*)event)->modifiers()==Qt::AltModifier)
  {
    // send Alt+Wheel to parent
    if (FIsNewSchool)
      QApplication::sendEvent(this->parentWidget(), event);
    else
      QApplication::sendEvent(m_Parent, event);
    return true;

  } else {
    // pass the event on to the parent class
    return ptWidget::eventFilter(obj, event);
  }
}

//==============================================================================

void ptInput::CheckTypes(const QVariant &Default, const QVariant &Minimum, const QVariant &Maximum, const QVariant &Step) {
  // Check consistency of all types.
  if (Default.type() != m_Type ||
      Maximum.type() != m_Type ||
      Step.type()    != m_Type ) {
    printf("(%s,%d) this : %s\n"
           "Default.type() : %d\n"
           "Minimum.type() : %d\n"
           "Maximum.type() : %d\n"
           "Step.type() : %d\n"
           "m_Type : %d\n",
           __FILE__,__LINE__,
           objectName().toLocal8Bit().data(),
           Default.type(),
           Minimum.type(),
           Maximum.type(),
           Step.type(),
           m_Type);
    assert (Default.type() == m_Type);
    assert (Maximum.type() == m_Type);
    assert (Step.type()    == m_Type);
  }
}

//==============================================================================
