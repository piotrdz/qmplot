/* function.h - defines the Function class and its derivatives,
                which represent the types of functions
                that can be plotted and FunctionDB class, which handles them.

 This file is part of QMPlot licensed under GPLv2.

 Copyright (C) Piotr Dziwinski 2009-2010
*/

#ifndef _QMPLOT_FUNCTION_H
#define _QMPLOT_FUNCTION_H

#include "treeparser.h"

#include <QString>
#include <QColor>
#include <QMap>
#include <QPainter>
#include <QDomDocument>
#include <QDomElement>
#include <string>


/*
 *
 *  Functions defined by the user are described by Function class and its derivatives
 *  which are held inside the (singleton) class FunctionDB.
 *
 *  They cannot be created or copied on their own, because:
 *   1. to avoid creating multiple copies of them and to hold them in one place (FunctionDB)
 *   2. copying of TreeParsers would be problematic
 *   3. because of recursive functions.
 *
 *  Functions are created, retrieved, changed, and removed all through FunctionDB calls.
 *
 */

class FunctionDB;

//! \enum FunctionType Describes the three derived types of Function
enum FunctionType
{
  //! Cartesian function, either X -> Y or Y -> X (depends on subtype)
  FT_Cartesian,
  //! Parametric function
  FT_Parametric,
  //! Implicit function
  FT_Implicit
};

//! \enum VerifyError Describes the result of parser verification
enum VerifyError
{
  VE_NoError,
  //! Required variable not found (used in implicit functions)
  VE_MissingVariable,
  //! Variable/or function was not found
  VE_UnresolvedVariable,
  //! Function not found in FunctionDB (to be removed)
  VE_OtherError
};

//! \enum FunctionPaintParams Describes the parameters passed to function for painting
struct FunctionPaintParams
{
  FunctionPaintParams()
  { xMin = yMin = 0.0; scale = 40.0; }

  //! The area to draw
  QRect area;
  //! Minimum X and Y values
  double xMin, yMin;
  //! Pixel scale
  double scale;
};

//! \class Function Abstract base class for functions
/** Contains only the common properties of all functions. */
class Function
{
  // Block copy constructor and assignment operator
    Function(const Function &o) : _type(o._type) {}
    const Function& operator=(const Function &) { return *this; }

  protected:
    //! The constructor is protected, for its use by derived classes and friend class FunctionDB
    Function(FunctionType vType);
    //! Name can only be changed by FunctionDB and derived classes
    inline bool setName(const QString &vName);

  public:
    virtual ~Function() { }

    // Access to these fields is open, so they could even be public members

    inline FunctionType type() const
    { return _type; }

    inline bool& enabled()
    { return _enabled; }

    inline QColor& color()
    { return _color; }

    inline double& width()
    { return _width; }

    // Name has to be validated

    inline QString name() const
    { return _name; }

    //! Draw the function using given painter
    virtual void paint(QPainter &p, const FunctionPaintParams &fp) = 0;

    //! Reparse ( set the same expression again on parser(s) )
    virtual void reparse() = 0;

    //! Check for variable errors
    virtual VerifyError check() = 0;

    //! Read properties from XML document
    virtual bool readProperties(const QDomElement &element);
    //! Save properties from XML document
    virtual void saveProperties(QDomDocument &document, QDomElement &element);

  protected:
    //! The function type - one of the derived classes
    const FunctionType _type;
    //! True if the function is to be drawn
    bool _enabled;
    //! Name of the function (must be unique)
    QString _name;
    //! Color of plot
    QColor _color;
    //! Width of plot line
    double _width;

  friend class FunctionDB;
};

//! \enum CartesianType Decribes the subtype of cartesian function
enum CartesianType
{
  //! Function of X -> Y
  CT_XToY,
  //! Function of Y -> X
  CT_YToX
};

//! \class CartesianFunction A regular function f(x) = ... or f(y) = ...
class CartesianFunction : public Function
{
  private:
    //! Constructor is private for use of FunctionDB
    CartesianFunction(const QString &vName, CartesianType vSubtype = CT_XToY);

  public:
    ~CartesianFunction() { }

    inline CartesianType& subtype()
    { return _subtype; }
    inline bool& minF()
    { return _minF; }
    inline bool& maxF()
    { return _maxF; }
    inline double& min()
    { return _min; }
    inline double& max()
    { return _max; }
    inline TreeParser& formula()
    { return _formula; }

    void paint(QPainter &p, const FunctionPaintParams &fp);

    void reparse();

    VerifyError check();

    bool readProperties(const QDomElement &element);
    void saveProperties(QDomDocument &document, QDomElement &element);

  protected:
    //! Subtype
    CartesianType _subtype;
    //! Flags for whether minimum and maximum domain bounds are enabled
    bool _minF, _maxF;
    //! Minimum and maximum domain bounds
    double _min, _max;
    //! Parsed formula
    TreeParser _formula;

  friend class FunctionDB;
};

//! \class ParametricFunction A parametric function f_x(t) = ... , f_y(t) = ...
class ParametricFunction : public Function
{
  private:
    ParametricFunction(const QString &vName);

  public:
    ~ParametricFunction() { }

    inline double& minParam()
    { return _minParam; }
    inline double& maxParam()
    { return _maxParam; }
    inline double& paramStep()
    { return _paramStep; }
    inline TreeParser& xFormula()
    { return _xFormula; }
    inline TreeParser& yFormula()
    { return _yFormula; }

    void paint(QPainter &p, const FunctionPaintParams &fp);

    void reparse();

    VerifyError check();

    bool readProperties(const QDomElement &element);
    void saveProperties(QDomDocument &document, QDomElement &element);

  protected:
    //! Parsed formulas of f_x and f_y components
    TreeParser _xFormula, _yFormula;
    //! Minimum and maximum values of parameter
    double _minParam, _maxParam;
    //! Step of parameter in drawing
    double _paramStep;

  friend class FunctionDB;
};

//! \class ImplicitFunction An implicit function f(x, y) = ... = 0
class ImplicitFunction : public Function
{
  private:
    //! Constructor is private for use of FunctionDB
    ImplicitFunction(const QString &vName);

  public:
    ~ImplicitFunction() { }

    inline TreeParser& formula()
    { return _formula; }
    inline double& drawAccuracy()
    { return _drawAccuracy; }

    void paint(QPainter &p, const FunctionPaintParams &fp);

    void reparse();

    VerifyError check();

    bool readProperties(const QDomElement &element);
    void saveProperties(QDomDocument &document, QDomElement &element);

  private:
    //! Parsed formula
    TreeParser _formula;
    double _drawAccuracy;

  friend class FunctionDB;
};

//! \class FunctionDB The function database
/** The class creates and stores Function objects.
  Access is given only through pointers to the objects. */
class FunctionDB
{
  private:
    // Block copy constructor and assignment operator
    FunctionDB(const FunctionDB &) {}
    const FunctionDB& operator=(const FunctionDB &) { return *this; }

  public:
    FunctionDB();
    ~FunctionDB();

    //! Returns the only instance of the class
    inline static FunctionDB* instance()
    { return _instance; }

    //! Returns the error encountered after calling verifyFunctions()
    inline VerifyError verifyError() const
    { return _verifyError; }

    //! Returns true, if recursion was detected
    inline bool recursionError() const
    { return _recursionError; }

    inline void clearRecursionError()
    { _recursionError = false; }

    //! Returns a pointer to the function of the given name, or NULL if not found
    Function* function(const QString &name);

    //! Returs a list of names of all functions
    QStringList functionNameList() const;
    //! Returns a list of pointers to all functions
    QList<Function*> functionList();

    //! Adds a new function of the given type and name
    /** If name is not given, it is generated by genName().
        If there already exists a function with the given name,
         it returns a NULL pointer, otherwise, the pointer to the created function.*/
    Function* addFunction(FunctionType type, const QString &name = "");

    //! Removes the function of given name
    /** Returns false if the function was not found. */
    bool removeFunction(const QString &name);

    //! Removes all functions
    void clear();

    //! Changes the name of the function and returns true if successful
    bool changeName(const QString &oldName, const QString &newName);

    //! Verifies the function of given name and returns true if no errors were found
    bool verifyFunction(const QString &name);

    //! Disables all functions (sets their enabled flags to false)
    void disableFunctions();

    //! Calls reparse() on all functions
    void reparseFunctions();

    //! Read a QMPlot document (XML file)
    bool openFile(const QString &fileName);
    //! Save a QMPlot document
    bool saveFile(const QString &fileName);

  private:
    //! The pointer to the only instance of FunctionDB
    static FunctionDB *_instance;
    //! Map of all functions (by unique names)
    QMap<QString, Function*> _functionsMap;
    //! Verify error detected when calling verifyFunction()
    VerifyError _verifyError;
    //! True if recursion was detected (when calling getFunctionValue() )
    bool _recursionError;

    //! Generate an automatic name for new function
    QString genName();

    //! Callback function for TreeParser
    /** Returns true if the given name is a function */
    static bool isFunction(const std::string &name);
    //! Callback function for TreeParser
    /** Computes the value of function given argument x and returns true if successful.
        The function detects recursive calls and returns false. */
    static bool getFunctionValue(const std::string &name, double x, double &value);
};

#endif // _QMPLOT_FUNCTION_H
