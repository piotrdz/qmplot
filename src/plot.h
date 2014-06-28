/* plot.h - defines the PlotArea class, which draws the function plots,
            and renders pixmaps for export.

 This file is part of QMPlot licensed under GPLv2.

 Copyright (C) Piotr Dziwinski 2009-2010
*/

#ifndef _QMPLOT_PLOT_H
#define _QMPLOT_PLOT_H

#include "common.h"

#include <QString>
#include <QMap>
#include <QWidget>
#include <QPoint>
#include <QPixmap>

//! \class PlotArea Widget drawing and exporting function plots
/** The class draws all enabled functions from FunctionDB and detects recursion.
User can zoom in and out and translate the view. There is also an exportPlot()
function for exporting plot to a file. */
class PlotArea : public QWidget
{
  Q_OBJECT

  public:
    PlotArea(QWidget *parent);
    ~PlotArea();

    //! Resets all values to default
    void reset();

    // Functions for getting and setting parameters

    inline double scale() const
      { return _scale; }
    inline double xMin() const
      { return _tX - (width() / _scale) * 0.5; }
    inline double xMax() const
      { return _tX + (width() / _scale) * 0.5; }
    inline double yMin() const
      { return _tY - (height() / _scale) * 0.5; }
    inline double yMax() const
      { return _tY + (height() / _scale) * 0.5; }

    //! Returns false if \a vScale is below minimum value
    bool setUnitScale(double vScale);
    void setTranslateX(double vTx);
    void setTranslateY(double vTy);
    void setManualAxisUnitF(bool vOn);
    //! Returns false if \a vUnit is below minimum value
    bool setManualAxisUnit(double vUnit);

    //! Exports the plot given the export data
    /** \a data should contain valid fileName and other values but pixmap should be NULL
        because it will be created in the function and should be destroyed afterwards. */
    void exportPlot(ExportData &data);

  protected:
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);
    void resizeEvent(QResizeEvent *);

  signals:
    void unitScaleChanged(double scale);
    void translateXChanged(double tx);
    void translateYChanged(double ty);
    void axisUnitChanged(double axisUnit);
    //! Emitted when recursion was detected.
    /** \a functionName is the name of function which caused the recursion. */
    void recursionDetected(const QString &functionName);

  private:
    //! Pixel scale
    double _scale;
    //! X, Y translation of coordinate origin
    double _tX, _tY;
    //! Unit distance on X and Y axes
    double _axisUnit;
    //! True, if user requested manual units
    bool _manualAxisUnitF;
    //! Manual, user-requested, units
    double _manualAxisUnit;
    //! Base cursor position on drag
    QPoint _mouseBasePos;
    //! Base translate values on drag
    double _baseTx, _baseTy;
    //! Whether to draw functions (false during drag)
    bool _drawFlag;
    //! Font for drawing units
    QFont *_axisFont;
    //! Font metrics of the above
    QFontMetrics *_fontMetrics;

    //! Auto-scales axis units
    void updateAxisUnit();
    /** Paints the plot on given painter
      (used both in painting on widget and exporting) */
    void paint(QPainter &p, int width, int height);
};


#endif // _QMPLOT_PLOT_H
