/*******************************************************************************
**
** Photivo
**
** Copyright (C) 2012-2015 Bernd Schoeler <brjohn@brother-john.net>
** Copyright (C) 2012 Michael Munzert <mail@mm-log.com>
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

#ifndef PTCFGITEM_H
#define PTCFGITEM_H

#include <QColor>
#include <QList>
#include <QVariant>
#include <memory>

class ptStorable;
class ptCurve;

class ptCfgItem;
using TCfgItemList = QList<ptCfgItem>;

//------------------------------------------------------------------------------
class ptCfgItem {
public:
  static const int CFirstCustomType = 100;

  /*! \brief The \c TType enum contains the available types of items. */
  enum TType {
    // Widgets stored in ptFilterConfig’s default store
    ColorSelectButton = 0,          //!< ptColorSelectButton
    Check,                          //!< ptCheck
    Combo,                          //!< ptChoice
    SpinEdit,                       //!< ptInput: Simple input field for numbers.
    Slider,                         //!< ptInput: Input of numbers via a slider.
    HueSlider,                      //!< ptInput: Slider with an added hue bar.
    Generic,                        //!< generic value without associated GUI element
    // Widgets stored in in the custom store
    CurveWin = CFirstCustomType,    //!< ptCurveWindow
    CustomType                      //!< any user defined type implementing ptStorable
  };

  struct TComboEntry {
    QString   caption;
    int       value;
    QString   storableId;
  };

  using TComboEntryList = QList<TComboEntry>;


public:
  struct TColorSelectButton {
    QString       Id;
    TType         Type;
    QColor        Default;
    bool          UseCommonDispatch;
    bool          Storable;
    QString       ToolTip;
  };

  struct TCheck {
    QString       Id;
    TType         Type;
    bool          Default;
    bool          UseCommonDispatch;
    bool          Storable;
    QString       Caption;
    QString       ToolTip;
  };

  struct TCombo {
    QString             Id;
    TType               Type;
    int                 Default;
    TComboEntryList     EntryList;
    bool                UseCommonDispatch;
    bool                Storable;
    QString             Caption;
    QString             ToolTip;
  };

  struct TInput {
    QString       Id;
    TType         Type;
    QVariant      Default;
    QVariant      Min;
    QVariant      Max;
    QVariant      StepSize;
    int           Decimals;
    bool          UseCommonDispatch;
    bool          Storable;
    QString       Caption;
    QString       ToolTip;
  };

  struct TCurve {
    QString                   Id;
    TType                     Type;
    std::shared_ptr<ptCurve>  Curve;
    QString                   Caption;
  };

  struct TCustom {
    QString                   Id;
    TType                     Type;
    ptStorable*               Object;
  };

  struct TGeneric {
    QString  Id;
    QVariant Default;
    bool     Storable;
  };


public:
  /* NOTE: ptCfgItem basically does same thing that ptSettings does. The class is needed for nice
    static init in the derived filter constructors. There’s no way to get that with simple structs
    and still keep all UI items in one single list. */
  /*! \group Constructors.
      One for each type of GUI item. */
  ///@{
  ptCfgItem(const TColorSelectButton& AValues);
  ptCfgItem(const TCheck&  AValues);
  ptCfgItem(const TCombo&  AValues);
  ptCfgItem(TCombo&&       AValues);
  ptCfgItem(const TInput&  AValues);
  ptCfgItem(const TCurve&  AValues);
  ptCfgItem(const TCustom& AValues);
  ptCfgItem(const TGeneric& AValue);
  ///@}

  /*! Performs a type and range check of `AValue` according to the requirements of this `ptCfgItem`
      object and returns a `QVariant` with valid type and value. When validation is not possible
      raises a `ptInfo` exception. */
  QVariant validate(const QVariant &AValue) const;

  /*! \group Members
      Simple members for easy access */
  ///@{
  // used by all item types
  QString Id;                         /*!< Internal ID. Used as key to identify this control in
                                           settings. Never shows up in GUI. */
  TType   Type;                       //!< Type of input control.
  bool    UseCommonDispatch = false;  //!< Defines if control uses the automatic signals/slots mechanism.
  bool    Storable = false;           /*!< Defines if the control is saved to the preset file.
                                           Only applies to controls in the default store. */
  QString Caption;                    //!< Caption text that appears in the GUI.
  QString ToolTip;                    //!< Text for the GUI popup tooltip.

  // specific to all custom types
  ptStorable   *AssocObject = nullptr;

  // used by TColorSelectButton, TCheck, TCombo, TInput, TCurve (via the constructor)
  QVariant      Default;

  // specific to TCombo
  QList<TComboEntry>   EntryList;

  // specific to TInput
  QVariant      Min;
  QVariant      Max;
  QVariant      StepSize;
  int           Decimals = -1;

  // specific to TCurve
  std::shared_ptr<ptCurve> Curve;
  ///@}


private:
  void  init();
  void  ensureVariantType(QVariant &AValue) const;
  void  setVariantType();

  QMetaType::Type  FIntendedType;

};

#endif // PTCFGIITEM_H
