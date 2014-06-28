/* function.cpp - implements the Function class and its derivatives,
                  which represent the types of functions
                  that can be plotted and FunctionDB class, which handles them.

 This file is part of QMPlot licensed under GPLv2.

 Copyright (C) Piotr Dziwinski 2009-2010
*/


#include "function.h"

#include <QtXml>
#include <cmath>
using namespace std;


// Convenience functions for reading and saving data in XML document

bool readDoubleProperty(const QDomElement &element, const QString &name, double &value)
{
  QDomElement propertyElement = element.firstChildElement(name);
  if (propertyElement.isNull()) return false;

  QString text = propertyElement.text();
  bool ok = true;
  double val = text.toDouble(&ok);
  if (!ok) return false;
  value = val;

  return true;
}

bool readIntProperty(const QDomElement &element, const QString &name, int &value)
{
  QDomElement propertyElement = element.firstChildElement(name);
  if (propertyElement.isNull()) return false;

  QString text = propertyElement.text();
  bool ok = true;
  int val = text.toInt(&ok);
  if (!ok) return false;
  value = val;

  return true;
}

bool readBoolProperty(const QDomElement &element, const QString &name, bool &value)
{
  QDomElement propertyElement = element.firstChildElement(name);
  if (propertyElement.isNull()) return false;

  QString text = propertyElement.text();
  if (text == QString("true"))
  {
    value = true;
  }
  else if (text == QString("false"))
  {
    value = false;
  }
  else
  {
    return false;
  }

  return true;
}

void saveDoubleProperty(QDomDocument &document, QDomElement &element,
                        const QString &name, double value)
{
  QDomElement propertyElement = document.createElement(name);
  element.appendChild(propertyElement);
  QDomText text = document.createTextNode(QString().setNum(value));
  propertyElement.appendChild(text);
}

void saveIntProperty(QDomDocument &document, QDomElement &element,
                     const QString &name, int value)
{
  QDomElement propertyElement = document.createElement(name);
  element.appendChild(propertyElement);
  QDomText text = document.createTextNode(QString().setNum(value));
  propertyElement.appendChild(text);
}

void saveBoolProperty(QDomDocument &document, QDomElement &element,
                      const QString &name, bool value)
{
  QDomElement propertyElement = document.createElement(name);
  element.appendChild(propertyElement);
  QDomText text = document.createTextNode(value ? QString("true") : QString("false"));
  propertyElement.appendChild(text);
}


// -------- Function --------


// List of standard function colors
static const QColor COLOR_WHEEL[] = {
  Qt::blue, Qt::green, Qt::red,
  Qt::cyan, Qt::magenta, Qt::gray,
  Qt::darkRed, Qt::darkBlue, Qt::darkGreen,
  Qt::darkCyan, Qt::darkMagenta, Qt::yellow };
static const int COLOR_WHEEL_SIZE = sizeof COLOR_WHEEL / sizeof *COLOR_WHEEL;
static int COLOR_WHEEL_INDEX = 0;

Function::Function(FunctionType vType) : _type(vType)
{
  _enabled = true;
  _name = QString("noname");
  _width = 1.0;
  _color = COLOR_WHEEL[COLOR_WHEEL_INDEX];
  if (++COLOR_WHEEL_INDEX >= COLOR_WHEEL_SIZE)
  {
    COLOR_WHEEL_INDEX = 0;
  }
}

bool Function::setName(const QString &vName)
{
  for (int i = 0; i < vName.size(); ++i)
  {
    QChar ch = vName[i];
    if (!(ch.isLetter() || (ch == '_'))) return false;
  }

  _name = vName;
  return true;
}

bool Function::readProperties(const QDomElement &element)
{
  QDomElement nameElement = element.firstChildElement("name");
  if (nameElement.isNull()) return false;
  if (!setName(nameElement.text())) return false;

  readDoubleProperty(element, "width", _width);

  QDomElement colorElement = element.firstChildElement("color");
  if (!colorElement.isNull())
  {
    int r, g, b;
    if (readIntProperty(colorElement, "r", r)) _color.setRed(r);
    if (readIntProperty(colorElement, "g", g)) _color.setGreen(g);
    if (readIntProperty(colorElement, "b", b)) _color.setBlue(b);
  }

  return true;
}

void Function::saveProperties(QDomDocument &document, QDomElement &element)
{
  QDomElement nameElement = document.createElement("name");
  element.appendChild(nameElement);
  QDomText nameText = document.createTextNode(_name);
  nameElement.appendChild(nameText);

  saveDoubleProperty(document, element, "width", _width);

  QDomElement colorElement = document.createElement("color");
  element.appendChild(colorElement);
  saveIntProperty(document, colorElement, "r", _color.red());
  saveIntProperty(document, colorElement, "g", _color.green());
  saveIntProperty(document, colorElement, "b", _color.blue());
}


// -------- CartesianFunction --------


CartesianFunction::CartesianFunction(const QString &vName, CartesianType vSubtype)
    : Function(FT_Cartesian)
{
  _subtype = vSubtype;
  _name = vName;
  _formula.setExpression("0");
  _minF = _maxF = false;
  _min = _max = 0.0;
}

void CartesianFunction::reparse()
{
  _formula.reparseExpression();
}

VerifyError CartesianFunction::check()
{
  vector<string> list = _formula.variablesInExpression();
  for (unsigned int i = 0; i < list.size(); ++i)
  {
    if ((_subtype == CT_XToY) && (list.at(i) != string("x")))
      return VE_UnresolvedVariable;

    else if ((_subtype == CT_YToX) && (list.at(i) != string("y")))
      return VE_UnresolvedVariable;
  }

  return VE_NoError;
}

bool CartesianFunction::readProperties(const QDomElement &element)
{
  if (!Function::readProperties(element)) return false;

  QDomElement formulaElement = element.firstChildElement("formula");
  if (formulaElement.isNull()) return false;

  QDomElement subtypeElement = element.firstChildElement("subtype");
  if (subtypeElement.isNull())
    _subtype = CT_XToY;
  else
  {
    if (subtypeElement.text() == "x_to_y")
      _subtype = CT_XToY;
    else if (subtypeElement.text() == "y_to_x")
      _subtype = CT_YToX;
    else
      _subtype = CT_XToY;
  }

  _formula.setExpression(formulaElement.text().toStdString());

  readBoolProperty(element, "min_flag", _minF);
  readDoubleProperty(element, "min", _min);
  readBoolProperty(element, "max_flag", _maxF);
  readDoubleProperty(element, "max", _max);

  return true;
}

void CartesianFunction::saveProperties(QDomDocument &document, QDomElement &element)
{
  Function::saveProperties(document, element);

  QDomElement formulaElement = document.createElement("formula");
  element.appendChild(formulaElement);
  QDomText formulaText =
      document.createTextNode(QString::fromStdString(_formula.expression()));
  formulaElement.appendChild(formulaText);

  QDomElement subtypeElement = document.createElement("subtype");
  element.appendChild(subtypeElement);
  QString sText;
  if (_subtype == CT_XToY)
    sText = "x_to_y";
  else if (_subtype == CT_YToX)
    sText = "y_to_x";
  else
    sText = "x_to_y";
  QDomText subtypeText = document.createTextNode(sText);
  subtypeElement.appendChild(subtypeText);

  saveBoolProperty(document, element, "min_flag", _minF);
  saveDoubleProperty(document, element, "min", _min);
  saveBoolProperty(document, element, "max_flag", _maxF);
  saveDoubleProperty(document, element, "max", _max);
}

void CartesianFunction::paint(QPainter &p, const FunctionPaintParams &fp)
{
  if (!_enabled) return;
  if (!_formula.status().ok()) return;

  QPen functionPen = QPen();
  functionPen.setJoinStyle(Qt::BevelJoin);
  functionPen.setWidthF(_width);
  functionPen.setColor(_color);
  p.setPen(functionPen);

  p.translate(fp.area.x(), fp.area.y());
  p.setClipRect(0, 0, fp.area.width(), fp.area.height());

  // We'll be drawing line segments, so we need these
  double lastVal = 0.0;
  bool hasLastVal = false;

  if (_subtype == CT_XToY)
  {
    double xVal = 0.0;
    _formula.setVariable("x", &xVal);

    // Computed value
    double val = 0.0;

    // Iterate through all x values in the area
    for (int x = 0; x < fp.area.width(); ++x)
    {
      xVal = fp.xMin + x / fp.scale;
      if ((_minF && (xVal < _min)) || (_maxF && (xVal > _max)))
        continue;

      ComputeResult result = _formula.computeValue(val);
      if (result.allOk())
      {
        // Convert val to pixel coordinates
        val = fp.area.height() - (val - fp.yMin) * fp.scale;

        if (fabs(val) > 32e3) continue;

        if (hasLastVal)
          p.drawLine(QPointF(x - 1.0, lastVal), QPointF(x, val));

        lastVal = val;
        hasLastVal = true;
      }
      else
      {
        hasLastVal = false;
      }
    }
    _formula.unsetVariable("x");
  }
  else // _subtype == CT_YToX
  {
    double yVal = 0.0;
    _formula.setVariable("y", &yVal);

    double val = 0.0;

    for (int y = 0; y < fp.area.height(); ++y)
    {
      yVal = fp.yMin + y / fp.scale;
      if ((_minF && (yVal < _min)) || (_maxF && (yVal > _max)))
        continue;

      ComputeResult result = _formula.computeValue(val);

      if (result.allOk())
      {
        val = (val - fp.xMin) * fp.scale;

        if (fabs(val) > 32e3) continue;

        if (hasLastVal)
          p.drawLine(QPointF(lastVal, fp.area.height() - y + 1.0),
                     QPointF(val, fp.area.height() - y));

        lastVal = val;
        hasLastVal = true;
      }
      else
      {
        hasLastVal = false;
      }
    }
    _formula.unsetVariable("y");
  }
}


// -------- ParametricFunction --------


ParametricFunction::ParametricFunction(const QString &vName) : Function(FT_Parametric)
{
  _name = vName;
  _xFormula.setExpression("sin t");
  _yFormula.setExpression("cos t");
  _minParam = 0.0;
  _maxParam = 2.0 * M_PI;
  _paramStep = 0.05;
}

void ParametricFunction::reparse()
{
  _xFormula.reparseExpression();
  _yFormula.reparseExpression();
}

VerifyError ParametricFunction::check()
{
  vector<string> list = _xFormula.variablesInExpression();
  for (unsigned int i = 0; i < list.size(); ++i)
  {
    if (list.at(i) != string("t"))
      return VE_UnresolvedVariable;
  }

  list = _yFormula.variablesInExpression();
  for (unsigned int i = 0; i < list.size(); ++i)
  {
    if (list.at(i) != string("t"))
      return VE_UnresolvedVariable;
  }

  return VE_NoError;
}

bool ParametricFunction::readProperties(const QDomElement &element)
{
  if (!Function::readProperties(element)) return false;

  QDomElement xFormulaElement = element.firstChildElement("x_formula");
  if (xFormulaElement.isNull()) return false;

  QDomElement yFormulaElement = element.firstChildElement("y_formula");
  if (yFormulaElement.isNull()) return false;

  _xFormula.setExpression(xFormulaElement.text().toStdString());
  _yFormula.setExpression(yFormulaElement.text().toStdString());

  readDoubleProperty(element, "min_param", _minParam);
  readDoubleProperty(element, "max_param", _maxParam);
  readDoubleProperty(element, "param_step", _paramStep);

  return true;
}

void ParametricFunction::saveProperties(QDomDocument &document, QDomElement &element)
{
  Function::saveProperties(document, element);

  QDomElement xFormulaElement = document.createElement("x_formula");
  element.appendChild(xFormulaElement);
  QDomText xFormulaText = document.createTextNode(
      QString::fromStdString(_xFormula.expression()));
  xFormulaElement.appendChild(xFormulaText);

  QDomElement yFormulaElement = document.createElement("y_formula");
  element.appendChild(yFormulaElement);
  QDomText yFormulaText = document.createTextNode(
      QString::fromStdString(_yFormula.expression()));
  yFormulaElement.appendChild(yFormulaText);

  saveDoubleProperty(document, element, "min_param", _minParam);
  saveDoubleProperty(document, element, "max_param", _maxParam);
  saveDoubleProperty(document, element, "param_step", _paramStep);
}

void ParametricFunction::paint(QPainter &p, const FunctionPaintParams &fp)
{
  if ((!_enabled) || (_minParam >= _maxParam)) return;
  if (!_xFormula.status().ok()) return;
  if (!_yFormula.status().ok()) return;

  QPen functionPen = QPen();
  functionPen.setJoinStyle(Qt::BevelJoin);
  functionPen.setWidthF(_width);
  functionPen.setColor(_color);
  p.setPen(functionPen);

  p.translate(fp.area.x(), fp.area.y());
  p.setClipRect(0, 0, fp.area.width(), fp.area.height());

  double tVal = 0.0;
  _xFormula.setVariable("t", &tVal);
  _yFormula.setVariable("t", &tVal);

  double xVal = 0.0, yVal = 0.0;
  double lastXVal = 0.0;
  double lastYVal = 0.0;
  bool hasLastVal = false;

  for (tVal = _minParam; tVal < _maxParam; tVal += _paramStep)
  {
    ComputeResult result1 = _xFormula.computeValue(xVal);
    ComputeResult result2 = _yFormula.computeValue(yVal);

    if ((result1.allOk()) && (result2.allOk()))
    {
      yVal = fp.area.height() - (yVal - fp.yMin) * fp.scale;
      xVal = (xVal - fp.xMin) * fp.scale;

      if (hasLastVal)
        p.drawLine(QPointF(lastXVal, lastYVal), QPointF(xVal, yVal));

      lastXVal = xVal;
      lastYVal = yVal;
      hasLastVal = true;
    }
    else
    {
      hasLastVal = false;
    }
  }

  _xFormula.unsetVariable("t");
  _yFormula.unsetVariable("t");
}


// -------- ImplicitFunction --------


ImplicitFunction::ImplicitFunction(const QString &vName) : Function(FT_Implicit)
{
  _name = vName;
  _formula.setExpression("sin x + cos y");
  _drawAccuracy = 40.0;
}

void ImplicitFunction::reparse()
{
  _formula.reparseExpression();
}

VerifyError ImplicitFunction::check()
{
  vector<string> list = _formula.variablesInExpression();
  if (list.size() == 0)
  {
    return VE_MissingVariable;
  }
  for (unsigned int i = 0; i < list.size(); ++i)
  {
    if ((list.at(i) != string("x")) && (list.at(i) != string("y")))
    {
      return VE_UnresolvedVariable;
    }
  }

  return VE_NoError;
}

bool ImplicitFunction::readProperties(const QDomElement &element)
{
  if (!Function::readProperties(element)) return false;

  QDomElement formulaElement = element.firstChildElement("formula");
  if (formulaElement.isNull()) return false;

  if (!_formula.setExpression(formulaElement.text().toStdString()))
    return false;

  readDoubleProperty(element, "draw_accuracy", _drawAccuracy);

  return true;
}

void ImplicitFunction::saveProperties(QDomDocument &document, QDomElement &element)
{
  Function::saveProperties(document, element);

  QDomElement formulaElement = document.createElement("formula");
  element.appendChild(formulaElement);
  QDomText formulaText = document.createTextNode(
      QString::fromStdString(_formula.expression()));
  formulaElement.appendChild(formulaText);

  saveDoubleProperty(document, element, "draw_accuracy", _drawAccuracy);
}

void ImplicitFunction::paint(QPainter &p, const FunctionPaintParams &fp)
{
  if (!_enabled) return;
  if (!_formula.status().ok()) return;

  QPen functionPen = QPen();
  functionPen.setJoinStyle(Qt::BevelJoin);
  functionPen.setWidthF(_width);
  functionPen.setColor(_color);
  p.setPen(functionPen);

  p.translate(fp.area.x(), fp.area.y());
  p.setClipRect(0, 0, fp.area.width(), fp.area.height());

  double xVal = 0.0;
  _formula.setVariable("x", &xVal);
  double yVal = 0.0;
  _formula.setVariable("y", &yVal);

  /* The minimum "resolving" value
     Values in range [-threshold, threshold] are treated like zero */
  double threshold = 0.5 / fp.scale;

  /* The plot is drawn by searching for roots in lines along Y axis
      in subsequent blocks of the length _drawAccuracy.
     The roots are found by calculating approximate differential of the function
      and jumping to the calculated values. */

  for (int x = 0; x < fp.area.width(); ++x)
  {
    xVal = fp.xMin + x / fp.scale;

    double y = 0.0;
    // Values below this are already plotted
    double doneY = 0.0;
    // Count of the number of jumps in current block
    int repeats = 0;
    while (y < fp.area.height())
    {
      yVal = fp.yMin + y / fp.scale;

      double val = 0.0;
      ComputeResult result = _formula.computeValue(val);
      if (!result.allOk())
      {
        y += 1.0;
        continue;
      }

      if (fabs(val) <= threshold)
      {
        yVal = fp.area.height() - (yVal - fp.yMin) * fp.scale;

        p.drawPoint(QPointF(x, yVal));

        repeats = 0;
        doneY = y + 1.0;
        y += _drawAccuracy;
        continue;
      }

      // Calculate differential for the current point

      double oldVal = val;
      double oldYVal = yVal;
      yVal += threshold;

      result = _formula.computeValue(val);

      if (!result.allOk())
      {
        y += 1.0;
        continue;
      }

      double diff = (val - oldVal) / threshold;
      double newYVal = oldYVal - oldVal / diff;
      double newY = (newYVal - fp.yMin) * fp.scale;

      // Don't go back below the done values
      if (newY < doneY)
      {
        repeats = 0;
        y += _drawAccuracy;
        doneY = y + 1.0;
        continue;
      }

      // Don't go back more than _drawAccuracy
      if (y - newY > _drawAccuracy)
      {
        repeats = 0;
        doneY = y + 1.0;
        y += _drawAccuracy;
        continue;
      }

      // Don't go forward more than _drawAccuracy
      if (newY - y > _drawAccuracy)
      {
        repeats = 0;
        y += _drawAccuracy;
        continue;
      }

      // Abandon the search in current block if there were more than 5 jumps
      if (++repeats > 5)
      {
        repeats = 0;
        y += _drawAccuracy;
        doneY = y + 1.0;
        continue;
      }

      y = newY;
    }
  }

  _formula.unsetVariable("x");
  _formula.unsetVariable("y");
}


// -------- FunctionDB --------


FunctionDB::FunctionDB()
{
  Q_ASSERT(_instance == NULL);
  _instance = this;
  _verifyError = VE_NoError;
  _recursionError = false;

  // Set callbacks for recursive functions
  TreeParser::setIsFunction(isFunction);
  TreeParser::setGetFunctionValue(getFunctionValue);
}

FunctionDB::~FunctionDB()
{
  _instance = NULL;
  QMap<QString, Function*>::iterator it;
  for (it = _functionsMap.begin(); it != _functionsMap.end(); ++it)
  {
    delete it.value();
    it.value() = NULL;
  }
  _functionsMap.clear();
}

Function* FunctionDB::function(const QString &name)
{
  Function *result = NULL;
  if (_functionsMap.contains(name))
    result = _functionsMap[name];

  return result;
}

QStringList FunctionDB::functionNameList() const
{
  QStringList result;
  QMap<QString, Function*>::const_iterator it;
  for (it = _functionsMap.begin(); it != _functionsMap.end(); ++it)
    result.append(it.key());

  return result;
}

QList<Function*> FunctionDB::functionList()
{
  QList<Function*> result;
  QMap<QString, Function*>::const_iterator it;
  for (it = _functionsMap.begin(); it != _functionsMap.end(); ++it)
    result.append(it.value());

  return result;
}

QString FunctionDB::genName()
{
  QString name;
  bool done = false;
  int index = 0;
  while (!done)
  {
    for (char ch = ((index == 0) ? 'f' : 'a'); ch <= 'z'; ++ch)
    {
      QString newName = name + QChar(ch);
      if (!_functionsMap.contains(newName))
      {
        name = newName;
        done = true;
        break;
      }
    }
    ++index;
  }
  return name;
}

Function* FunctionDB::addFunction(FunctionType type, const QString &pName)
{
  QString name = pName;
  if (name.isEmpty())
    name = genName();

  if (_functionsMap.contains(name))
    return NULL;

  Function *function;
  switch (type)
  {
    case FT_Cartesian:
    {
      function = new CartesianFunction(name);
      break;
    }
    case FT_Parametric:
    {
      function = new ParametricFunction(name);
      break;
    }
    case FT_Implicit:
    {
      function = new ImplicitFunction(name);
      break;
    }
  }
  _functionsMap.insert(name, function);
  return function;
}

bool FunctionDB::removeFunction(const QString &name)
{
  if (!_functionsMap.contains(name))
    return false;

  Function *function = _functionsMap.take(name);
  delete function;
  function = NULL;

  return true;
}

void FunctionDB::clear()
{
  _functionsMap.clear();
}

bool FunctionDB::changeName(const QString &oldName, const QString &newName)
{
  if ((!_functionsMap.contains(oldName)) || _functionsMap.contains(newName))
    return false;

  for (int i = 0; i < newName.size(); ++i)
  {
    QChar ch = newName[i];
    if (!(ch.isLetter() || (ch == '_'))) return false;
  }

  Function *function = _functionsMap.take(oldName);
  function->setName(newName);
  _functionsMap.insert(newName, function);
  function = NULL;

  return true;
}

void FunctionDB::reparseFunctions()
{
  QMap<QString, Function*>::iterator it;
  for (it = _functionsMap.begin(); it != _functionsMap.end(); ++it)
    it.value()->reparse();
}

void FunctionDB::disableFunctions()
{
  QMap<QString, Function*>::iterator it;
  for (it = _functionsMap.begin(); it != _functionsMap.end(); ++it)
    it.value()->enabled() = false;
}

bool FunctionDB::verifyFunction(const QString &name)
{
  reparseFunctions();

  Function *function = NULL;
  if (_functionsMap.contains(name))
  {
    function = _functionsMap[name];
  }
  else
  {
    _verifyError = VE_OtherError;
    return false;
  }

  _verifyError = function->check();
  function = NULL;

  return (_verifyError == VE_NoError);
}

bool FunctionDB::openFile(const QString &fileName)
{
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly))
  {
    qDebug("file.open() failed");
    return false;
  }

  QDomDocument document;
  QString errorString;
  int errorLine = 0, errorColumn = 0;
  if (!document.setContent(&file, &errorString, &errorLine, &errorColumn))
  {
    file.close();
    qDebug("document.setContents() failed: %s at line: %d, column: %d",
           errorString.toStdString().c_str(), errorLine, errorColumn);
    return false;
  }
  file.close();

  QDomElement documentElement = document.documentElement();
  if (documentElement.tagName() != "mplotdoc")
  {
    qDebug("Invalid opening tag");
    return false;
  }

  QDomNodeList functionList = documentElement.elementsByTagName("function");
  if (functionList.isEmpty())
  {
    qDebug("Function list empty");
    return false;
  }

  QVector<Function*> readFunctions;
  for (int i = 0; i < functionList.size(); ++i)
  {
    Function *function = NULL;
    QDomElement functionElement = functionList.at(i).toElement();
    if (functionElement.isNull()) continue;

    QDomElement typeElement = functionElement.firstChildElement("type");
    if (typeElement.isNull())
    {
      qDebug("No function type specified!");
      continue;
    }

    QString text = typeElement.text();
    if (text == "cartesian")
    {
      function = new CartesianFunction("f");
    }
    else if (text == "parametric")
    {
      function = new ParametricFunction("f");
    }
    else if (text == "implicit")
    {
      function = new ImplicitFunction("f");
    }
    else
    {
      qDebug("Invalid function type specified!");
      continue;
    }

    if (function == NULL)
    {
      qDebug("Function creation failed!");
      continue;
    }

    if (!function->readProperties(functionElement))
    {
      qDebug("Some required properties could not be found!");
      delete function;
      function = NULL;
      continue;
    }

    readFunctions.append(function);
    function = NULL;
  }

  _functionsMap.clear();
  for (int i = 0; i < readFunctions.size(); ++i)
  {
    _functionsMap.insert(readFunctions.at(i)->_name, readFunctions.at(i));
  }

  return true;
}

bool FunctionDB::saveFile(const QString &fileName)
{
  if (_functionsMap.empty())
  {
    return false;
  }

  QDomDocument document;
  QDomElement root = document.createElement("mplotdoc");
  document.appendChild(root);

  QMap<QString, Function*>::iterator it;
  for (it = _functionsMap.begin(); it != _functionsMap.end(); ++it)
  {
    QDomElement functionTag = document.createElement("function");
    root.appendChild(functionTag);

    QDomElement typeTag = document.createElement("type");
    functionTag.appendChild(typeTag);
    {
      QString string;
      switch (it.value()->type())
      {
        case FT_Cartesian:
        {
          string = "cartesian";
          break;
        }
        case FT_Parametric:
        {
          string = "parametric";
          break;
        }
        case FT_Implicit:
        {
          string = "implicit";
          break;
        }
      }
      QDomText text = document.createTextNode(string);
      typeTag.appendChild(text);
    }

    it.value()->saveProperties(document, functionTag);
  }

  QByteArray output = document.toByteArray();

  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly))
  {
    return false;
  }

  if (file.write(output) != output.size())
  {
    return false;
  }

  file.close();
  return true;
}

bool FunctionDB::isFunction(const std::string &n)
{
  FunctionDB *i = FunctionDB::instance();
  if (i == NULL) return false;

  QString name = QString::fromStdString(n);
  QString secondName;
  // Parametric functions can be split into _x and _y components
  if (name.endsWith("_x") || name.endsWith("_y"))
  {
    secondName = name.left(name.size() - 2);
  }

  if (i->_functionsMap.contains(name))
  {
    if (i->_functionsMap[name]->_type != FT_Cartesian)
    {
      return false;
    }
  }
  else if ((!secondName.isEmpty()) && (i->_functionsMap.contains(secondName)))
  {
    if (i->_functionsMap[secondName]->_type != FT_Parametric)
    {
      return false;
    }
  }
  else
  {
    return false;
  }

  return true;
}

bool FunctionDB::getFunctionValue(const std::string &n, double x, double &value)
{
  FunctionDB *i = FunctionDB::instance();
  if (i == NULL) return false;
  if (i->_recursionError) return false;

  QString name = QString::fromStdString(n);
  QString secondName;
  int type = -1;
  if (name.endsWith("_x"))
  {
    type = 1;
    secondName = name.left(name.size() - 2);
  }
  else if (name.endsWith("_y"))
  {
    type = 2;
    secondName = name.left(name.size() - 2);
  }

  if (i->_functionsMap.contains(name))
  {
    if (i->_functionsMap[name]->_type != FT_Cartesian)
      return false;

    CartesianFunction *cF = static_cast<CartesianFunction*>(i->_functionsMap[name]);

    if (!cF->_formula.status().ok())
      return false;

    if (cF->_subtype == CT_XToY)
    {
      // Recursion protection works, because we pass the replace = false flag to setVariable
      // This ensures that parser will reject this value of x, if it is already set.
      // And since variables are only set during computations, that means that the call
      // for function value has become recursive.
      if (!cF->_formula.setVariable("x", &x, false))
      {
        i->_recursionError = true;
        return false;
      }
    }
    else
    {
      if (!cF->_formula.setVariable("y", &x, false))
      {
        i->_recursionError = true;
        return false;
      }
    }

    ComputeResult result = cF->_formula.computeValue(value);

    if (cF->_subtype == CT_XToY)
      cF->_formula.unsetVariable("x");
    else
      cF->_formula.unsetVariable("y");

    cF = NULL;

    if (!result.allOk())
      return false;
  }
  else if ((!secondName.isEmpty()) && (i->_functionsMap.contains(secondName)))
  {
    if (i->_functionsMap[secondName]->_type != FT_Parametric)
      return false;

    ParametricFunction *pF = static_cast<ParametricFunction*>(i->_functionsMap[secondName]);

    ComputeResult result;

    if (type == 1)
    {
      if (!pF->_xFormula.status().ok())
        return false;

      if (!pF->_xFormula.setVariable("t", &x, false))
      {
        i->_recursionError = true;
        return false;
      }

      result = pF->_xFormula.computeValue(value);
    }
    else
    {
      if (!pF->_yFormula.status().ok())
        return false;

      if (!pF->_yFormula.setVariable("t", &x, false))
      {
        i->_recursionError = true;
        return false;
      }

      result = pF->_yFormula.computeValue(value);
    }

    if (type == 1)
    {
      pF->_xFormula.unsetVariable("t");
    }
    else
    {
      pF->_yFormula.unsetVariable("t");
    }

    pF = NULL;

    if (!result.allOk())
      return false;
  }
  else
  {
    return false;
  }

  return true;
}

// Initial value
FunctionDB* FunctionDB::_instance = NULL;
