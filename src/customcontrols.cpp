/* customcontrols.cpp - implements several derived classes that are used in the UI.

 This file is part of QMPlot licensed under GPLv2.

 Copyright (C) Piotr Dziwinski 2009-2010
*/

#include "customcontrols.h"
#include "dialogs.h"

#include <QPaintEvent>
#include <QPainter>
#include <QTimer>
#include <QFocusEvent>


// -------- ExpressionLineEdit --------


ExpressionLineEdit::ExpressionLineEdit(QWidget *parent) : QLineEdit(parent)
{
  _editedFirst = false;
  reset();
  _valid = true;
  setToolTip(QString("= 0"));

  connect(this, SIGNAL(textEdited(const QString &)),
          this, SLOT(tEdited(const QString &)));
  connect(this, SIGNAL(textChanged(const QString &)),
          this, SLOT(tChanged(const QString &)));
  connect(this, SIGNAL(editingFinished()),
          this, SLOT(eFinished()));
}

void ExpressionLineEdit::reset()
{
  setText(QString("0"));
}

void ExpressionLineEdit::setValid(bool vValid)
{
  _valid = vValid;
  update();
}

void ExpressionLineEdit::revalidate()
{
  tChanged(text());
}

double ExpressionLineEdit::value() const
{
  double value = 0.0;
  ComputeResult result = _parser.computeValue(value);

  if (result.allOk())
    return value;

  return 0.0;
}

void ExpressionLineEdit::setValue(double vValue)
{
  setText(QString().setNum(vValue));
}

void ExpressionLineEdit::tEdited(const QString &text)
{
  _editedFirst = true;
  updateExpression(text);

  emit valueEdited(value(), _valid);
}

void ExpressionLineEdit::tChanged(const QString &text)
{
  if (_editedFirst)
    _editedFirst = false;
  else
    updateExpression(text);

  emit valueChanged(value(), _valid);
}

void ExpressionLineEdit::eFinished()
{
  if (text().isEmpty())
    setText("0");

  emit editingFinished(value(), _valid);
}

void ExpressionLineEdit::updateExpression(const QString &text)
{
  bool result = _parser.setExpression(text.toStdString());
  _valid = result;
  if (_valid)
  {
    double value = 0.0;
    ComputeResult result = _parser.computeValue(value);
    _valid = result.allOk();
    if (_valid)
      setToolTip(QString("= %1").arg(value));
    else
      setToolTip(tr("Invalid expression"));
  }

  if (!_valid)
  {
    QPalette p = palette();
    p.setColor(QPalette::Text, Qt::red);
    setPalette(p);
  }
  else
  {
    setPalette((static_cast<QWidget*>(parent()))->palette());
  }
  update();
}


// -------- SignalTextEdit --------

SignalTextEdit::SignalTextEdit(QWidget *parent) : QTextEdit(parent)
{
  _timer = new QTimer(this);
  _timer->setInterval(1000);
  _timer->setSingleShot(true);
  connect(_timer, SIGNAL(timeout()),
          this, SIGNAL(editingFinished()));
  connect(this, SIGNAL(textChanged()),
          this, SLOT(tChanged()));
}

SignalTextEdit::~SignalTextEdit()
{
  delete _timer;
  _timer = NULL;
}

void SignalTextEdit::enableSignal()
{
  connect(this, SIGNAL(textChanged()),
          this, SLOT(tChanged()));
}

void SignalTextEdit::disableSignal()
{
  disconnect(this, SIGNAL(textChanged()),
             this, SLOT(tChanged()));
}

void SignalTextEdit::tChanged()
{
  if (_timer->isActive())
    _timer->stop();

  _timer->start();
}

void SignalTextEdit::focusOutEvent(QFocusEvent *e)
{
  QTextEdit::focusOutEvent(e);

  if (_timer->isActive())
    _timer->stop();

  emit editingFinished();
}


// -------- ColorButton --------


ColorButton::ColorButton(QWidget *parent) : QPushButton(parent)
{
  _color = Qt::black;
  _dialog = new ColorDialog(this);
  _dialog->setColor(_color);

  connect(this, SIGNAL(clicked()),
          this, SLOT(displayDialog()));
  // Only connect to accepted(), when rejected(), do nothing
  connect(_dialog, SIGNAL(accepted()),
          this, SLOT(dialogAccepted()));
}

ColorButton::~ColorButton()
{
  delete _dialog;
  _dialog = NULL;
}

void ColorButton::retranslateUi()
{
  _dialog->retranslateUi();
}

void ColorButton::setColor(const QColor &vColor)
{
  _color = vColor;
  update();
  emit colorChanged(_color);
}

void ColorButton::paintEvent(QPaintEvent *e)
{
  QPushButton::paintEvent(e);

  QPainter p(this);

  p.setPen(QPen(Qt::black));
  p.setBrush(QBrush(_color));

  p.drawRect(width() / 4, height() / 4, width() / 2, height() / 2);

  p.end();
}

void ColorButton::displayDialog()
{
  _dialog->setColor(_color);
  _dialog->show();
}

void ColorButton::dialogAccepted()
{
  setColor(_dialog->color());
  emit colorEdited(_color);
}


// -------- ColorWidget --------


ColorWidget::ColorWidget(QWidget *parent) : QWidget(parent)
{
  _color = Qt::black;
}

void ColorWidget::setColor(const QColor &vColor)
{
  _color = vColor;
  update();
}

void ColorWidget::paintEvent(QPaintEvent *e)
{
  e->accept();

  QPainter p(this);

  p.setPen(QPen(Qt::black));
  p.setBrush(QBrush(_color));

  p.drawRect(0, 0, width(), height());

  p.end();
}
