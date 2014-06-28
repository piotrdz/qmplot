/* dialogs.h - defines several dialog-derived classes that handle
               the user interaction in several program functions.

 This file is part of QMPlot licensed under GPLv2.

 Copyright (C) Piotr Dziwinski 2009-2010
*/


#ifndef _QMPLOT_DIALOGS_H
#define _QMPLOT_DIALOGS_H

#include "common.h"

#include <QColor>
#include <QStringList>
#include <QDialog>

// To avoid #includes
namespace Ui
{
  class ColorDialog;
  class ExportDialog;
  class PreferencesDialog;
  class ReferenceDialog;
  class AboutDialog;
  class WarnUnsavedDialog;
};
class QFileDialog;

//! \class ColorDialog A dialog that lets user choose a color
class ColorDialog : public QDialog
{
  Q_OBJECT

  public:
    ColorDialog(QWidget *parent);
    ~ColorDialog();

    inline const QColor& color() const
    { return _color; }
    void setColor(const QColor &vColor);

  public slots:
    void retranslateUi();

  private:
    Ui::ColorDialog *_ui;
    QColor _color;

  private slots:
    // Slots for UI changes
    void redSliderChanged(int value);
    void greenSliderChanged(int value);
    void blueSliderChanged(int value);
    void hexEditChanged();
};

//! \class ExportDialog A dialog that shows when exporting to a file
/** The dialog only gathers data (ExportDialogData). */
class ExportDialog : public QDialog
{
  Q_OBJECT

  public:
    ExportDialog(QWidget *parent);
    ~ExportDialog();

    void setData(const ExportData &vData);
    inline const ExportData& data() const
    { return _data; }

  public slots:
    void retranslateUi();

  private:
    Ui::ExportDialog *_ui;
    //! A dialog to select file for export
    QFileDialog *_fileDialog;
    ExportData _data;

    //! Function to update the label with image size
    void updateSize();

  private slots:
    // Slots for UI changes
    void fileDialogAccepted();
    void unitScaleEdited(double value, bool valid);
    void xMinEdited(double value, bool valid);
    void xMaxEdited(double value, bool valid);
    void yMinEdited(double value, bool valid);
    void yMaxEdited(double value, bool valid);
    void okButtonClicked();
};

//! \class PreferencesDialog A configuration dialog
/** Like export dialog, gathers data (Preferences).
QStringList translations is a reference to list of translated language names. */
class PreferencesDialog : public QDialog
{
  Q_OBJECT

  public:
    PreferencesDialog(QWidget *parent, const QStringList &translations);
    ~PreferencesDialog();

    void setData(const Settings &vData);
    const Settings& data()
    { return _data; }

  public slots:
    void retranslateUi();

  private:
    Ui::PreferencesDialog *_ui;
    const QStringList &_translations;
    Settings _data;

    // Fills language combo box
    void updateTranslations();

  private slots:
    void okButtonClicked();
};

//! \class ReferenceDialog A dialog that shows the reference of functions
/** Just implements the .ui file. */
class ReferenceDialog : public QDialog
{
  Q_OBJECT

  public:
    ReferenceDialog(QWidget *parent);
    ~ReferenceDialog();

  public slots:
    void retranslateUi();

  private:
    Ui::ReferenceDialog *_ui;
};

//! \class AboutDialog An about program dialog
/** Just implements the .ui file. */
class AboutDialog : public QDialog
{
  Q_OBJECT

  public:
    AboutDialog(QWidget *parent);
    ~AboutDialog();

  public slots:
    void retranslateUi();

  private:
    Ui::AboutDialog *_ui;
};

//! \class AboutDialog An about program dialog
/** Just implements the .ui file with the addition of getting the state of checkbox.
 Accepted means that the user clicked cancel button ! */
class WarnUnsavedDialog : public QDialog
{
  Q_OBJECT

  public:
    WarnUnsavedDialog(QWidget *parent);
    ~WarnUnsavedDialog();

    //! Returns whether the checkbox is checked
    bool checked();

  private:
    Ui::WarnUnsavedDialog *_ui;
};

#endif // _QMPLOT_DIALOGS_H
