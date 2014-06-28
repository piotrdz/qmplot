/* dialogs.cpp - implements several dialog-derived classes that handle
                 the user interaction in several program functions.

 This file is part of QMPlot licensed under GPLv2.

 Copyright (C) Piotr Dziwinski 2009-2010
*/

#include "dialogs.h"
#include "function.h"

#include "../build/ui_colordialog.h"
#include "../build/ui_exportdialog.h"
#include "../build/ui_preferencesdialog.h"
#include "../build/ui_referencedialog.h"
#include "../build/ui_aboutdialog.h"
#include "../build/ui_warnunsaveddialog.h"

#include <QPainter>
#include <QImageWriter>
#include <QList>
#include <QFileDialog>
#include <QMessageBox>
#include <cmath>
using namespace std;


// -------- ColorDialog --------


ColorDialog::ColorDialog(QWidget *parent)
    : QDialog(parent), _ui(new Ui::ColorDialog)
{
  _ui->setupUi(this);
  setModal(true);
  _color = Qt::black;

  connect(_ui->redSlider, SIGNAL(valueChanged(int)),
          this, SLOT(redSliderChanged(int)));
  connect(_ui->greenSlider, SIGNAL(valueChanged(int)),
          this, SLOT(greenSliderChanged(int)));
  connect(_ui->blueSlider, SIGNAL(valueChanged(int)),
          this, SLOT(blueSliderChanged(int)));
  connect(_ui->hexEdit, SIGNAL(editingFinished()),
          this, SLOT(hexEditChanged()));
}

ColorDialog::~ColorDialog()
{
  delete _ui;
  _ui = NULL;
}

void ColorDialog::retranslateUi()
{
  _ui->retranslateUi(this);
}

void ColorDialog::setColor(const QColor &vColor)
{
  _color = vColor;

  // Disconnect signals to avoid recursion
  disconnect(_ui->redSlider, SIGNAL(valueChanged(int)),
             this, SLOT(redSliderChanged(int)));
  disconnect(_ui->greenSlider, SIGNAL(valueChanged(int)),
             this, SLOT(greenSliderChanged(int)));
  disconnect(_ui->blueSlider, SIGNAL(valueChanged(int)),
             this, SLOT(blueSliderChanged(int)));

  _ui->redSlider->setValue(_color.red());
  _ui->redValueLabel->setText(QString().setNum(_color.red()));
  _ui->greenSlider->setValue(_color.green());
  _ui->greenValueLabel->setText(QString().setNum(_color.green()));
  _ui->blueSlider->setValue(_color.blue());
  _ui->blueValueLabel->setText(QString().setNum(_color.blue()));

  _ui->colorWidget->setColor(_color);
  // Sets the hex representation of color (as in HTML, etc.), for instance #AABBCC
  _ui->hexEdit->setText(QString("#%1%2%3")
    .arg((int)_color.red(), (int)2, 16, QLatin1Char('0'))
    .arg((int)_color.green(), 2, 16, QLatin1Char('0'))
    .arg((int)_color.blue(), 2, 16, QLatin1Char('0')));

  connect(_ui->redSlider, SIGNAL(valueChanged(int)),
          this, SLOT(redSliderChanged(int)));
  connect(_ui->greenSlider, SIGNAL(valueChanged(int)),
          this, SLOT(greenSliderChanged(int)));
  connect(_ui->blueSlider, SIGNAL(valueChanged(int)),
          this, SLOT(blueSliderChanged(int)));
}

void ColorDialog::redSliderChanged(int value)
{
  _color.setRed(value);
  setColor(_color);
}

void ColorDialog::greenSliderChanged(int value)
{
  _color.setGreen(value);
  setColor(_color);
}

void ColorDialog::blueSliderChanged(int value)
{
  _color.setBlue(value);
  setColor(_color);
}

void ColorDialog::hexEditChanged()
{
  QString text = _ui->hexEdit->text();
  // Extract the red, green and blue values from hex representation
  // # R R G G B B
  // 0 1 2 3 4 5 6
  _color.setRed(text.mid(1, 2).toInt(0, 16));
  _color.setGreen(text.mid(3, 2).toInt(0, 16));
  _color.setBlue(text.mid(5, 2).toInt(0, 16));
  setColor(_color);
}


// -------- ExportDialog --------


ExportDialog::ExportDialog(QWidget *parent)
    : QDialog(parent), _ui(new Ui::ExportDialog)
{
  _ui->setupUi(this);

  // Default value


  // Set default data
  setData(_data);

  _fileDialog = new QFileDialog(this, tr("Select output file"));
  _fileDialog->setFileMode(QFileDialog::AnyFile);

  QStringList filters;
  // Fill the file filter with supported image formats
  QList<QByteArray> list = QImageWriter::supportedImageFormats();
  for (int i = 0; i < list.size(); ++i)
  {
    // E.g. "BMP files (*.bmp)"
    filters << tr("%1 files (%2)").arg(QString(list[i]).toUpper())
                 .arg(QString("*.") + QString(list[i]).toLower());
  }

  // Add the any files option
  filters << tr("Any files (*.*)");
  _fileDialog->setNameFilters(filters);

  connect(_ui->fileButton, SIGNAL(clicked()),
          _fileDialog, SLOT(show()));
  connect(_fileDialog, SIGNAL(accepted()),
          this, SLOT(fileDialogAccepted()));

  connect(_ui->unitScaleEdit, SIGNAL(valueEdited(double, bool)),
          this, SLOT(unitScaleEdited(double, bool)));
  connect(_ui->xMinEdit, SIGNAL(valueEdited(double, bool)),
          this, SLOT(xMinEdited(double, bool)));
  connect(_ui->xMaxEdit, SIGNAL(valueEdited(double, bool)),
          this, SLOT(xMaxEdited(double, bool)));
  connect(_ui->yMinEdit, SIGNAL(valueEdited(double, bool)),
          this, SLOT(yMinEdited(double, bool)));
  connect(_ui->yMaxEdit, SIGNAL(valueEdited(double, bool)),
          this, SLOT(yMaxEdited(double, bool)));

  connect(_ui->okButton, SIGNAL(clicked()),
          this, SLOT(okButtonClicked()));
  connect(_ui->cancelButton, SIGNAL(clicked()),
          this, SLOT(reject()));
}

ExportDialog::~ExportDialog()
{
  delete _ui;
  _ui = NULL;
}

void ExportDialog::retranslateUi()
{
  _ui->retranslateUi(this);
  setData(_data);
}

void ExportDialog::updateSize()
{
  if ( (!_ui->unitScaleEdit->valid()) ||
       (!_ui->xMinEdit->valid()) || (!_ui->xMaxEdit->valid()) ||
       (!_ui->yMinEdit->valid()) || (!_ui->yMaxEdit->valid()) )
  {
    _ui->sizeLabel->setText( tr("Image size: %1 x %2 [px]")
                                .arg(QString("!!")).arg(QString("!!")) );
  }

  _ui->sizeLabel->setText( tr("Image size: %1 x %2 [px]")
    .arg( static_cast<int>(floor(fabs(_data.xMax - _data.xMin) * _data.scale)) )
    .arg( static_cast<int>(floor(fabs(_data.yMax - _data.yMin) * _data.scale)) ) );
}

void ExportDialog::setData(const ExportData &vData)
{
  _data = vData;
  _data.fileName = tr("plot.png");
  _ui->unitScaleEdit->setValue(_data.scale);
  _ui->xMinEdit->setValue(_data.xMin);
  _ui->xMaxEdit->setValue(_data.xMax);
  _ui->yMinEdit->setValue(_data.yMin);
  _ui->yMaxEdit->setValue(_data.yMax);
  updateSize();
}

void ExportDialog::fileDialogAccepted()
{
  if (_fileDialog->selectedFiles().isEmpty()) return;

  _data.fileName = _fileDialog->selectedFiles().first();
  _ui->fileLabel->setText(_data.fileName);
}

void ExportDialog::unitScaleEdited(double value, bool valid)
{
  if (value <= 0.0)
    _ui->unitScaleEdit->setValid(false);
  else if (valid)
    _data.scale = value;

  updateSize();
}

void ExportDialog::xMinEdited(double value, bool valid)
{
  if (valid)
  {
    _data.xMin = value;

    _ui->xMaxEdit->revalidate();
    if (_ui->xMaxEdit->valid())
    {
      if (_ui->xMaxEdit->value() < value)
      {
        _ui->xMinEdit->setValid(false);
      }
    }
  }

  updateSize();
}

void ExportDialog::xMaxEdited(double value, bool valid)
{
  if (valid)
  {
    _data.xMax = value;

    _ui->xMinEdit->revalidate();
    if (_ui->xMinEdit->valid())
    {
      if (_ui->xMinEdit->value() > value)
      {
        _ui->xMaxEdit->setValid(false);
      }
    }
  }

  updateSize();
}

void ExportDialog::yMinEdited(double value, bool valid)
{
  if (valid)
  {
    _data.yMin = value;

    _ui->yMaxEdit->revalidate();
    if (_ui->yMaxEdit->valid())
    {
      if (_ui->yMaxEdit->value() < value)
      {
        _ui->yMinEdit->setValid(false);
      }
    }
  }

  updateSize();
}

void ExportDialog::yMaxEdited(double value, bool valid)
{
  if (valid)
  {
    _data.yMax = value;

    _ui->yMinEdit->revalidate();
    if (_ui->yMinEdit->valid())
    {
      if (_ui->yMinEdit->value() > value)
      {
        _ui->yMaxEdit->setValid(false);
      }
    }
  }

  updateSize();
}

void ExportDialog::okButtonClicked()
{
  if (!_ui->unitScaleEdit->valid())
  {
    QMessageBox::critical(this, tr("Error"),
                          tr("The unit scale is incorrect. It should be positive."),
                          QMessageBox::Ok);
  }
  else if ((!_ui->xMinEdit->valid()) || (!_ui->xMaxEdit->valid()) ||
      (!_ui->yMinEdit->valid()) || (!_ui->yMaxEdit->valid()))
  {
    QMessageBox::critical(this, tr("Error"),
                          tr("The coordinate settings are incorrect. "
                          "The minimum value should be less than maximum."),
                          QMessageBox::Ok);
  }
  else
  {
    accept();
  }
}


// -------- PreferencesDialog --------


PreferencesDialog::PreferencesDialog(QWidget *parent, const QStringList &translations)
    : QDialog(parent), _ui(new Ui::PreferencesDialog), _translations(translations)
{
  _ui->setupUi(this);

  // Set default data
  setData(_data);

  updateTranslations();

  connect(_ui->okButton, SIGNAL(clicked()),
          this, SLOT(okButtonClicked()));
  connect(_ui->cancelButton, SIGNAL(clicked()),
          this, SLOT(reject()));
}

PreferencesDialog::~PreferencesDialog()
{
  delete _ui;
  _ui = NULL;
}

void PreferencesDialog::retranslateUi()
{
  _ui->retranslateUi(this);
  updateTranslations();
}

void PreferencesDialog::updateTranslations()
{
  _ui->languageComboBox->clear();
  for (int i = 0; i < _translations.size(); ++i)
  {
    _ui->languageComboBox->addItem(_translations.at(i));
  }
}

void PreferencesDialog::setData(const Settings &vData)
{
  _data = vData;

  _ui->numberFormatComboBox->setCurrentIndex(static_cast<int>(_data.numberFormat));
  _ui->numberPrecisionSpinBox->setValue(_data.numberPrecision);
  _ui->languageComboBox->setCurrentIndex(_data.language);
  _ui->warnUnsavedNewCheckBox->setChecked(_data.warnUnsavedNew);
  _ui->warnUnsavedAtExitCheckBox->setChecked(_data.warnUnsavedAtExit);
}

void PreferencesDialog::okButtonClicked()
{
  _data.numberFormat = static_cast<NumberFormat>(_ui->numberFormatComboBox->currentIndex());
  _data.numberPrecision = _ui->numberPrecisionSpinBox->value();
  _data.language = _ui->languageComboBox->currentIndex();
  _data.warnUnsavedNew = _ui->warnUnsavedNewCheckBox->isChecked();
  _data.warnUnsavedAtExit = _ui->warnUnsavedAtExitCheckBox->isChecked();

  accept();
}


// -------- ReferenceDialog --------


ReferenceDialog::ReferenceDialog(QWidget *parent)
    : QDialog(parent), _ui(new Ui::ReferenceDialog)
{
  _ui->setupUi(this);
}

ReferenceDialog::~ReferenceDialog()
{
  delete _ui;
  _ui = NULL;
}

void ReferenceDialog::retranslateUi()
{
  _ui->retranslateUi(this);
}


// -------- AboutDialog --------


AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent), _ui(new Ui::AboutDialog)
{
  _ui->setupUi(this);
}

AboutDialog::~AboutDialog()
{
  delete _ui;
  _ui = NULL;
}

void AboutDialog::retranslateUi()
{
  _ui->retranslateUi(this);
}


// -------- WarnUnsavedDialog --------


WarnUnsavedDialog::WarnUnsavedDialog(QWidget *parent)
    : QDialog(parent), _ui(new Ui::WarnUnsavedDialog)
{
  _ui->setupUi(this);
}

WarnUnsavedDialog::~WarnUnsavedDialog()
{
  delete _ui;
  _ui = NULL;
}

bool WarnUnsavedDialog::checked()
{
  return _ui->checkBox->isChecked();
}
