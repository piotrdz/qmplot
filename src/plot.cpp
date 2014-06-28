/* plot.h - defines the PlotArea class, which draws the function plots,
            and renders pixmaps for export.

 This file is part of QMPlot licensed under GPLv2.

 Copyright (C) Piotr Dziwinski 2009-2010
*/

#include "plot.h"
#include "function.h"

#include <QPaintEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QPen>
#include <cmath>

using namespace std;

/* Minumum value of unit, because lower values will probably be
  useless due to accuracy loss */
const double MIN_UNIT = 1e-12;
// Minimum pixel scale, lower values don't make much sense
const double MIN_SCALE = 1e-6;
// Useful shorthand
const double SQRT_3 = sqrt(3.0);

PlotArea::PlotArea(QWidget *parent) : QWidget(parent)
{
  _scale = 40.0;
  _tX = _tY = 0.0;
  _axisUnit = 1.0;
  _manualAxisUnitF = false;
  _manualAxisUnit = 1.0;
  _baseTx = _baseTy = 0.0;
  _drawFlag = true;
  _axisFont = new QFont("Arial", 10, QFont::Bold);
  _fontMetrics = new QFontMetrics(*_axisFont);
  setFocusPolicy(Qt::WheelFocus);
}

PlotArea::~PlotArea()
{
  delete _axisFont;
  _axisFont = NULL;
  delete _fontMetrics;
  _fontMetrics = NULL;
}

void PlotArea::reset()
{
  _scale = 40.0;
  _tX = _tY = 0.0;
  _axisUnit = 1.0;
  _manualAxisUnitF = false;
  _manualAxisUnit = 1.0;
  _baseTx = _baseTy = 0.0;
  emit translateXChanged(_tX);
  emit translateYChanged(_tY);
  emit unitScaleChanged(_scale);
  emit axisUnitChanged(_axisUnit);
}

bool PlotArea::setUnitScale(double vScale)
{
  if (vScale <= MIN_SCALE) return false;
  _scale = vScale;
  update();
  return true;
}

void PlotArea::setTranslateX(double vTx)
{
  _tX = vTx;
  update();
}

void PlotArea::setTranslateY(double vTy)
{
  _tY = vTy;
  update();
}

void PlotArea::setManualAxisUnitF(bool vOn)
{
  _manualAxisUnitF = vOn;
  updateAxisUnit();
  update();
}

bool PlotArea::setManualAxisUnit(double vValue)
{
  if (vValue < MIN_UNIT) return false;
  _manualAxisUnit = vValue;
  updateAxisUnit();
  update();
  return true;
}

void PlotArea::mousePressEvent(QMouseEvent *e)
{
  if (e->button() == Qt::LeftButton)
  {
    _mouseBasePos = e->pos();
    _baseTx = _tX;
    _baseTy = _tY;
    _drawFlag = false;
    setCursor(QCursor(Qt::SizeAllCursor));
  }
}

void PlotArea::keyPressEvent(QKeyEvent *e)
{
  bool changed = false;
  switch (e->key())
  {
    case Qt::Key_Left:
    {
      _tX = _tX - width() / (10.0 * _scale);
      changed = true;
      break;
    }
    case Qt::Key_Right:
    {
      _tX = _tX + width() / (10.0 * _scale);
      changed = true;
      break;
    }
    case Qt::Key_Up:
    {
      _tY = _tY + height() / (10.0 * _scale);
      changed = true;
      break;
    }
    case Qt::Key_Down:
    {
      _tY = _tY - height() / (10.0 * _scale);
      changed = true;
      break;
    }
    case Qt::Key_Plus:
    {
      _scale = _scale * 1.1;
      emit unitScaleChanged(_scale);
      updateAxisUnit();
      update();
      break;
    }
    case Qt::Key_Minus:
    {
      _scale = _scale * 0.9;
      emit unitScaleChanged(_scale);
      updateAxisUnit();
      update();
      break;
    }
    default: { }
  }
  if (changed)
  {
    emit translateXChanged(_tX);
    emit translateYChanged(_tY);
    updateAxisUnit();
    update();
  }
}

void PlotArea::mouseMoveEvent(QMouseEvent *e)
{
  if (e->buttons() & Qt::LeftButton)
  {
    _tX = _baseTx - (e->pos().x() - _mouseBasePos.x()) / _scale;
    _tY = _baseTy + (e->pos().y() - _mouseBasePos.y()) / _scale;
    emit translateXChanged(_tX);
    emit translateYChanged(_tY);
    update();
  }
}

void PlotArea::mouseReleaseEvent(QMouseEvent *e)
{
  if (e->button() == Qt::LeftButton)
  {
    _drawFlag = true;
    updateAxisUnit();
    update();
    setCursor(QCursor(Qt::ArrowCursor));
  }
}

void PlotArea::wheelEvent(QWheelEvent *e)
{
  _scale = _scale * (1.0 + 0.1 * (e->delta() / 120.0));
  emit unitScaleChanged(_scale);
  updateAxisUnit();
  update();
}

void PlotArea::resizeEvent(QResizeEvent *)
{
  updateAxisUnit();
  update();
}

// A bit complex algorithm to find the appropriate units
void PlotArea::updateAxisUnit()
{
  if (_manualAxisUnitF)
  {
    _axisUnit = _manualAxisUnit;
    return;
  }
  _axisUnit = pow(10.0, ceil(-log10(_scale * 0.5)) + 1.0);

  double w = width() / _scale;
  double xMin = _tX - w * 0.5;
  double xMax = _tX + w * 0.5;
  int ndiv = 0;
  int prevAction = 0;
  while (true)
  {
    double testX = abs(floor(max(abs(xMin), abs(xMax))) / _axisUnit) * _axisUnit;
    int tw1 = _fontMetrics->width(QString().setNum(testX, 'g'));
    int tw2 = _fontMetrics->width(QString().setNum(testX + _axisUnit, 'g'));
    // Discovered through trial and error
    if ((_axisUnit * _scale - 0.25 * (tw1 + tw2) < 15.0) && (prevAction != 2))
    {
      prevAction = 1;
      switch (ndiv)
      {
        case 0:
        {
          _axisUnit *= 2.0;
          ndiv = 1;
          break;
        }
        case 1:
        {
          _axisUnit *= (2.5/2.0);
          ndiv = 2;
          break;
        }
        case 2:
        {
          _axisUnit *= (5.0/2.5);
          ndiv = 3;
          break;
        }
        case 3:
        {
          _axisUnit *= (10.0/5.0);
          ndiv = 0;
          break;
        }
        default: {}
      }
    }
    else if ((_axisUnit * _scale - 0.25 * (tw1 + tw2) > (5.0 * ((tw1 + tw2) / 2.0))) && (prevAction != 1))
    {
      prevAction = 2;
      switch (ndiv)
      {
        case 0:
        {
          _axisUnit /= 2.0;
          ndiv = 1;
          break;
        }
        case 1:
        {
          _axisUnit /= (2.5/2.0);
          ndiv = 2;
          break;
        }
        case 2:
        {
          _axisUnit /= (5.0/2.5);
          ndiv = 3;
          break;
        }
        case 3:
        {
          _axisUnit /= (10.0/5.0);
          ndiv = 0;
          break;
        }
        default: {}
      }
    }
    else
    {
      break;
    }
  }

  emit axisUnitChanged(_axisUnit);
}

void PlotArea::exportPlot(ExportData &data)
{
  Q_ASSERT(data.pixmap == NULL);

  double oldScale = _scale;
  double oldTx = _tX;
  double oldTy = _tY;

  _scale = data.scale;
  _tX = (data.xMax + data.xMin) / 2.0;
  _tY = (data.yMax + data.yMin) / 2.0;

  data.pixmap = new QPixmap((int)(floor((data.xMax - data.xMin) * data.scale)),
                            (int)(floor((data.yMax - data.yMin) * data.scale)));

  QPainter p(data.pixmap);
  paint(p, data.pixmap->width(), data.pixmap->height());

  _scale = oldScale;
  _tX = oldTx;
  _tY = oldTy;
}

void PlotArea::paintEvent(QPaintEvent *e)
{
  e->accept();

  QPainter p(this);
  paint(p, width(), height());
}

void PlotArea::paint(QPainter &p, int width, int height)
{
  p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
  p.fillRect(QRect(0, 0, width, height), Qt::white);

  QPen axisPen = p.pen();

  QPen gridPen = p.pen();
  gridPen.setColor(Qt::lightGray);
  gridPen.setWidthF(0.5);

  QPen borderFontPen = p.pen();
  borderFontPen.setColor(QColor("#707070"));

  p.setFont(*_axisFont);
  p.setBrush(QBrush(Qt::black));

  double w = width / _scale;
  double h = height / _scale;

  double xMin = _tX - w * 0.5;
  double xMax = _tX + w * 0.5;
  double yMin = _tY - h * 0.5;
  double yMax = _tY + h * 0.5;

  double xAxis = width * 0.5 - _tX * _scale;
  double yAxis = height * 0.5 + _tY * _scale;

  int borderSize = _fontMetrics->height();

  // Draw grid
  p.setPen(gridPen);

  for (double y = floor(yMin / _axisUnit) * _axisUnit; y < yMax; y += _axisUnit)
    p.drawLine(QPointF(0.0, height - (y - yMin) * _scale),
               QPointF(width, height - (y - yMin) * _scale));

  for (double x = floor(xMin / _axisUnit) * _axisUnit; x < xMax; x += _axisUnit)
    p.drawLine(QPointF((x - xMin) * _scale, 0.0),
               QPointF((x - xMin) * _scale, height));

  p.setPen(axisPen);

  // Draw axis only if in view
  if ((xAxis > 0.0) && (xAxis < width))
  {
    p.drawLine(QPointF(xAxis, 0.0), QPointF(xAxis, height));
    // Arrow
    QVector<QPointF> v;
    v.push_back(QPointF(xAxis, 0.0));
    v.push_back(QPointF(xAxis - 5.0, 5.0 * SQRT_3));
    v.push_back(QPointF(xAxis + 5.0, 5.0 * SQRT_3));
    p.drawConvexPolygon(QPolygonF(v));
  }

  // Draw labels - next to axis
  if ((xAxis > 0.0) && (xAxis < width))
  {
    for (double y = floor(yMin / _axisUnit) * _axisUnit; y < yMax; y += _axisUnit)
    {
      p.drawLine(QPointF(xAxis - 2.0, height - (y - yMin) * _scale),
                 QPointF(xAxis + 2.0, height - (y - yMin) * _scale));
      if (fabs(y) > MIN_UNIT)
      {
        QString text = QString().setNum(y, 'g');
        int tw = _fontMetrics->width(text);
        p.drawText(QRectF(xAxis + 5.0,
                          height - (y - yMin) * _scale - _fontMetrics->height() / 2.0,
                          tw,
                          _fontMetrics->height()),
                   text, QTextOption(Qt::AlignCenter));
      }
    }
  }
  // On the border
  else
  {
    p.setPen(borderFontPen);
    for (double y = floor(yMin / _axisUnit) * _axisUnit; y < yMax; y += _axisUnit)
    {
      if (fabs(y) > MIN_UNIT)
      {
        QString text = QString().setNum(y, 'g');
        int tw = _fontMetrics->width(text);
        if (tw > borderSize)
          borderSize = tw;

        if (xAxis > 0.0)
        {
          p.drawText(QRectF(width - tw,
                            height - (y - yMin) * _scale - _fontMetrics->height() / 2.0,
                            tw,
                            _fontMetrics->height()),
                     text, QTextOption(Qt::AlignCenter));
        }
        else
        {
          p.drawText(QRectF(0.0,
                            height - (y - yMin) * _scale - _fontMetrics->height() / 2.0,
                            tw,
                            _fontMetrics->height()),
                     text, QTextOption(Qt::AlignCenter));
        }
      }
    }
    p.setPen(axisPen);
  }

  if ((yAxis > 0.0) && (yAxis < height))
  {
    p.drawLine(QPointF(0.0, yAxis), QPointF(width, yAxis));
    QVector<QPointF> v;
    v.push_back(QPointF(width, yAxis));
    v.push_back(QPointF(width - 5.0 * SQRT_3, yAxis + 5.0));
    v.push_back(QPointF(width - 5.0 * SQRT_3, yAxis - 5.0));
    p.drawConvexPolygon(QPolygonF(v));
  }

  if ((yAxis > 0.0) && (yAxis < height))
  {
    for (double x = floor(xMin / _axisUnit) * _axisUnit; x < xMax; x += _axisUnit)
    {
      p.drawLine(QPointF((x - xMin) * _scale, yAxis - 2.0),
                 QPointF((x - xMin) * _scale, yAxis + 2.0));
      if (fabs(x) > MIN_UNIT)
      {
        QString text = QString().setNum(x, 'g');
        int tw = _fontMetrics->width(text);
        p.drawText(QRectF((x - xMin) * _scale - tw / 2.0,
                          yAxis + 5.0,
                          tw,
                          _fontMetrics->height()),
                   text, QTextOption(Qt::AlignCenter));
      }
    }
  }
  else
  {
    p.setPen(borderFontPen);
    for (double x = floor(xMin / _axisUnit) * _axisUnit; x < xMax; x += _axisUnit)
    {
      if (fabs(x) > MIN_UNIT)
      {
        QString text = QString().setNum(x, 'g');
        int tw = _fontMetrics->width(text);
        if (yAxis < 0.0)
        {
          p.drawText(QRectF((x - xMin) * _scale - tw / 2.0, 0.0,
                            tw,
                            _fontMetrics->height()),
                     text, QTextOption(Qt::AlignCenter));
        }
        else
        {
          p.drawText(QRectF((x - xMin) * _scale - tw / 2.0,
                            height - _fontMetrics->height(),
                            tw,
                            _fontMetrics->height()),
                     text, QTextOption(Qt::AlignCenter));
        }
      }
    }
    p.setPen(axisPen);
  }

  if (!_drawFlag)
  {
    p.end();
    return;
  }

  FunctionPaintParams params;
  params.area = QRect(0, 0, width, height);
  params.xMin = xMin;
  params.yMin = yMin;
  params.scale = _scale;

  QList<Function*> functionList = FunctionDB::instance()->functionList();
  FunctionDB::instance()->clearRecursionError();
  for (QList<Function*>::iterator it = functionList.begin(); it != functionList.end(); ++it)
  {
    (*it)->paint(p, params);
    // Break on recursion
    if (FunctionDB::instance()->recursionError())
    {
      FunctionDB::instance()->disableFunctions();
      // This signal must be connected asynchronously or else we get into recursive paintEvent()s
      emit recursionDetected((*it)->name());
      break;
    }
  }

  p.end();
}
