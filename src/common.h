/* common.h - defines structs shared between modules.

This file is part of QMPlot licensed under GPLv2.

Copyright (C) Piotr Dziwinski 2009-2010
*/


#ifndef _QMPLOT_COMMON_H
#define _QMPLOT_COMMON_H

#include "treeparser.h"

#include <QString>

//! \struct Settings Application settings
struct Settings
{
  Settings()
  {
    // Default values
    numberFormat = TreeParser::numberFormat();
    numberPrecision = TreeParser::numberPrecision();
    language = 0;
    warnUnsavedNew = warnUnsavedAtExit = true;
  }

  //! Number format as a value of NumberFormat from TreeParser
  NumberFormat numberFormat;
  //! Precision of numbers displayed
  int numberPrecision;
  //! Language as the index in translation list (see MainWindow)
  int language;
  //! Whether to warn of unsaved changes when creating or opening new file
  bool warnUnsavedNew;
  //! Whether to warn of unsaved changes at exit
  bool warnUnsavedAtExit;
};

class QPixmap;

/** \struct ExportData A simple struct for getting and setting data between
 ExportDialog, MainWindow and PlotArea */
struct ExportData
{
  ExportData()
  {
    // Default values
    scale = 40.0;
    xMin = yMin = -10.0;
    xMax = yMax = 10.0;
    pixmap = NULL;
  }

  //! Name of the export file
  QString fileName;
  //! Pointer to a created pixmap
  QPixmap *pixmap;
  //! Pixel scale and the area to plot
  double scale, xMin, xMax, yMin, yMax;
};

#endif // _QMPLOT_COMMON_H
