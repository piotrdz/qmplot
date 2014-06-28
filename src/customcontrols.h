/* customcontrols.h - defines several derived classes that are used in the UI.

 This file is part of QMPlot licensed under GPLv2.

 Copyright (C) Piotr Dziwinski 2009-2010
*/

#ifndef _QMPLOT_CUSTOMCONTROLS_H
#define _QMPLOT_CUSTOMCONTROLS_H

#include "treeparser.h"

#include <QWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>

// Can't be bothered to include the headers
class ColorDialog;
class QPaintEvent;
class QTimer;
class QFocusEvent;

/** \class ExpressionLineEdit A QLineEdit which allows to enter any computable parser expression
The edit contains a TreeParser object, which is updated every time the text changes. If the user
enters an invalid (was badly parsed, or contained unresolved functions/variables) expression,
the text is set to red.
An invalid expression's value is zero.
The valid flag can also be set despite the expression being computable using setValid()
Default expression (and text) is "0".
*/
class ExpressionLineEdit : public QLineEdit
{
  Q_OBJECT

  public:
    ExpressionLineEdit(QWidget *parent);

    //! Resets the text and expression to 0
    void reset();

    //! Returns true if the expression is valid
    inline bool valid() const
    { return _valid; }

    //! Revalidate expression, setting it again to the same value
    void revalidate();

    //! Returns the value of expression if it can be computed, otherwise 0
    double value() const;

  public slots:
    //! Allows for custom setting of the valid flag
    void setValid(bool vValid);

    //! Set the expression to the given value
    void setValue(double vValue);

  signals:
    //! Emitted whenever the value is changed, including programmatically
    /** \a value is the value set, \a valid states whether it is valid */
    void valueChanged(double value, bool valid);

    //! Emitted only when the value is changed by the user
    /** \a value is the value set, \a valid states whether it is valid */
    void valueEdited(double value, bool valid);

    //! Emitted when the signal editingFinished() from QLineEdit is emitted
    /** \a value is the value set, \a valid states whether it is valid */
    void editingFinished(double value, bool valid);

  private:
    //! The parser object
    TreeParser _parser;
    //! True if the expression is computable
    bool _valid;
    //! Used to signal when textEdited() was called to avoid calling updateExpresion twice
    bool _editedFirst;

  private slots:
    //! Connected to textEdited() and textChanged()
    void tEdited(const QString &text);
    void tChanged(const QString &text);
    //! Connected to editingFinished()
    void eFinished();
    //! Updates the parser
    void updateExpression(const QString &text);
};

//! \class SignalTextEdit A QTextEdit that provides editingFinished() signal
/** The editingFinished() signal is sent on focus out event or after 1000 ms after last text change. */
class SignalTextEdit : public QTextEdit
{
  Q_OBJECT

  public:
    SignalTextEdit(QWidget *parent = NULL);
    ~SignalTextEdit();

    //! Enable (connect) the signal on text changes
    void enableSignal();
    //! Disable (disconnect) the signal on text changes
    void disableSignal();

  signals:
    void editingFinished();

  private:
    QTimer *_timer;

  private slots:
    void tChanged();

  protected:
    void focusOutEvent(QFocusEvent *e);
};

/** \class ColorButton A QPushButton which allows user to choose a color
The button looks like a regular button but with rectangle of the current color drawn on top of it.
When clicked, it displays ColorDialog and when done, sets the color to the chosen one.
Default color is black.
*/
class ColorButton : public QPushButton
{
  Q_OBJECT

  public:
    ColorButton(QWidget *parent);
    ~ColorButton();

    inline const QColor& color() const
    { return _color; }

  public slots:
    // For retranslating ColorDialog
    void retranslateUi();
    void setColor(const QColor &vColor);

  signals:
    //! Emitted whenever the color is changed, including programmatically
    void colorChanged(const QColor &color);

    //! Emitted when the user changes the color, but not when changed programmatically
    void colorEdited(const QColor &color);

  private:
    //! The current color
    QColor _color;
    //! Color dialog object associated with the color button
    ColorDialog *_dialog;

  protected:
    //! Reimplemented for drawing color rectangle on the button
    void paintEvent(QPaintEvent *);

  private slots:
    //! When OK button is clicked
    void displayDialog();
    //! When the color dialog is accepted
    void dialogAccepted();
};

/** \class ColorWidget A simple "dumb" widget which is a rectangle filled with given color
The default color is black.
*/
class ColorWidget : public QWidget
{
  public:
    ColorWidget(QWidget *parent);

    inline const QColor& color() const
    { return _color; }
    void setColor(const QColor &vColor);

  protected:
    void paintEvent(QPaintEvent *);

  private:
    QColor _color;
};


#endif // _QMPLOT_CUSTOMCONTROLS_H
