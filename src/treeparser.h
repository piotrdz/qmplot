/* treeparser.h - defines the TreeParser class, which parses and evaluates arithmetic expressions.

 This file is part of QMPlot licensed under GPLv2.

 Copyright (C) Piotr Dziwinski 2009-2010
*/

#ifndef _QMPLOT_TREEPARSER_H
#define _QMPLOT_TREEPARSER_H

#include <string>
#include <list>
#include <vector>
#include <map>
#include <iostream>


/*
 *  TreeParser class and other structs/types defined here are independent of the rest of the program.
 *  I initially wrote this parser class and only later built the program on top of it. It is written
 *  only using STL classes and that necessitates some conversions, but I want to keep it this way
 *  because it may be useful as part of other projects.
 *
 */


//! The type of numerical values used in expression
/** This is double by default, but can be long double, float or even int if apropriate changes are made */
typedef double NumType;

//! \enum NumberFormat Format of the numbers as displayed by parser
enum NumberFormat
{
  //! Default (no flag changes)
  NF_Auto = 0,
  //! With STL flag fixed
  NF_Fixed = 1,
  //! With STL flag scientific
  NF_Scientific = 2
};

//! Enumerates the types of tokens
enum TokenType
{
  //! Number
  TT_Number = 1,
  //! This is either a variable or constant - the distinction is made in parsing
  TT_Variable,
  //! Left bracket
  TT_LeftBracket,
  //! Right bracket
  TT_RightBracket,
  //! Comma, as argument separator for min and max functions as in "min(4,5)" (alternative - ';')
  TT_Comma,
  //! Unary plus as in "+4" - basically doesn't do anything
  TT_Plus,
  //! Unary minus as in "-4" - changes the sign
  TT_Minus,
  //! Adding as in "4+5"
  TT_Add,
  //! Subtracting as in "4-5"
  TT_Subtract,
  //! Multiplying as in "4*5"
  TT_Multiply,
  //! Dividing as in "4/5"
  TT_Divide,
  //! Modulo (as the C operator `%') as in "4%5"
  TT_Modulus,
  //! Exponentiation as in "4^5"
  TT_Power,
  //! Factorial as in "4!"
  TT_Factorial,
  //! Absolute value as in "abs(-4)"
  TT_Abs,
  //! Square root as in "sqrt 4"
  TT_Sqrt,
  //! Exp function (e^x) as in "exp 2" (=e^2)
  TT_Exp,
  //! Natural logarithm as in "ln e" (=1)
  TT_Ln,
  //! Logarithm to base 10 as in "log 10" (=1)
  TT_Log,
  //! Sine (argument in radians) as in "sin pi" (=0)
  TT_Sin,
  //! Cosine as in "cos pi/2" (=0)
  TT_Cos,
  //! Tangens as in "tan pi" (=0)
  TT_Tan,
  //! Arcus sinus as in "arcsin 1" (=pi/2)
  TT_Asin,
  //! Arcus cosinus as in "arccos 1" (=0)
  TT_Acos,
  //! Arcus tangens as in "arctan 0" (=0)
  TT_Atan,
  //! Hyperbolic sine as in "sinh 0" (=0)
  TT_Sinh,
  //! Hyperbolic cosine as in "cosh 0" (=1)
  TT_Cosh,
  //! Hyperbolic tangens as in "tanh 0" (=0)
  TT_Tanh,
  //! Signum as in "sgn -10" (=-1)
  TT_Signum,
  //! Ceiling (equivalent to C ceil() ) as in "ceil 3.4" (=4)
  TT_Ceil,
  //! Floor (equivalent to C floor() ) as in "floor 3.4" (=3)
  TT_Floor,
  //! Minimum as in "min(4, 5)" (=4)
  TT_Min,
  //! Maximum as in "max(4, 5)" (=5)
  TT_Max,
  //! External function (taking one argument) retrieved using callbacks
  TT_ExternalFunction,
  //! Used only in default constructor, should not appear in expression
  TT_None = 1000
};

//! Enumerates what type of argument(s) the token takes
enum ArgType
{
  AT_Standalone, //! No arguments (number or variable)
  AT_Binary,     //! Two arguments on either side
  AT_RightUnary, //! One argument on the right
  AT_LeftUnary,  //! One argument on the left
  AT_CommaBinary //! Two arguments to the right, separated by commas
};

//! A single token in input
class Token
{
  public:
    //! Default constructor
    /** Sets the type to TT_None */
    Token();
    //! Constructor for type
    /** You should normally use this constructor. \a vType is the type of token. \a vPosition is the position
      in input text. */
    Token(const TokenType &vType, const int &vPosition = -1);

    inline TokenType type() const
      { return _type; }

    inline int position() const
      { return _position; }

    inline NumType number() const
      { return _number; }

    inline void setNumber(NumType newNumber)
      { _number = newNumber; }

    inline std::string name() const
      { return _name; }

    inline void setName(const std::string &newName)
      { _name = newName; }

    //! Return priority of token
    int priority() const;
    //! Return argument type of token
    ArgType argType() const;

    //! Use this function to change type
    void changeType(const TokenType &newType);

  private:
    //! Type of token
    TokenType _type;
    //! Position of token in the input string, used in error messages
    int _position;
    //! Number associated with the token; it only makes sense for TT_Number
    NumType _number;
    /** Name associated with token; it is either the name of TT_Variable,
     or the character associated with the token, for example '+' for TT_Add;
     for TT_Number it is empty */
    std::string _name;
};

// Helpful typedefs
typedef std::vector<Token> TokenArray;
typedef std::vector<Token>::iterator TokenIterator;
typedef std::vector<Token>::const_iterator ConstTokenIterator;

typedef std::map<std::string, NumType> ValueMap;
typedef std::map<std::string, NumType>::iterator ValueMapIterator;
typedef std::map<std::string, NumType>::const_iterator ConstValueMapIterator;

typedef std::map<std::string, NumType*> PtrValueMap;
typedef std::map<std::string, NumType*>::iterator PtrValueMapIterator;
typedef std::map<std::string, NumType*>::const_iterator ConstPtrValueMapIterator;

//! Enumerates the errors that may happen while parsing input before any computations
enum ParseError
{
  PE_None,               //! This means there are no errors
  PE_EmptyExpression,
  PE_EmptyBrackets,
  PE_InvalidNumber,
  PE_InvalidCharacter,
  PE_MismatchedBrackets,
  PE_MisplacedComma,
  PE_MissingArgument,
  PE_ExtraArgument,
  PE_InvalidArgument,
  PE_GeneralError,
  PE_LogicError          //! This error shouldn't happen - if it does it means there's a bug somewhere :(
};

//! Enumerates the math-related errors that may happen while computing
enum MathError
{
  ME_None,              //! No error
  ME_InvalidExpression, //! Means that parsing failed and computation cannot be performed
  ME_DivisionByZero,
  ME_RangeError,
  ME_DomainError
};

//! The status of the parsing
struct ParseStatus
{
  ParseStatus()
    { reset(); }

  void reset()
  {
    error = PE_None;
    position = -1;
    code = 0;
    token = Token();
  }

  //! Returs true if there's no error
  inline bool ok() const
    { return (error == PE_None); }

  //! Returns a text message associtated with the error (in English)
  std::string errorString() const;

  //! The error
  ParseError error;
  //! Position of the nearest token in input string or -1 if it can't be resolved
  int position;
  /** Unique code associated with this occurence of the error; it is currently the line number
   in cpp module */
  int code;
  //! The token that caused the error; it may be null if the error isn't related to any token
  Token token;
};

//! The result of computation
struct ComputeResult
{
  ComputeResult()
  {
    expansions = 0;
    mathError = logicError = 0;
    variableError = false;
  }

  //! Returns a text message associtated with the error (in English)
  std::string errorString() const;

  //! Join results
  /** This is used in computing in multiple steps */
  void join(const ComputeResult &other)
  {
    expansions += other.expansions;
    if (other.mathError != 0)
    {
      mathError = other.mathError;
    }
    if (other.logicError != 0)
    {
      logicError = other.logicError;
    }
    if (variableError || other.variableError)
    {
      variableError = true;
    }
  }

  //! Return true if no math or logic errors are present, but there can be a variable error
  inline bool ok() const
  {
    return ((mathError == ME_None) && (logicError == 0));
  }

  //! Return true if no errors are present, including variable error
  inline bool allOk() const
  {
    return ((mathError == ME_None) && (logicError == 0) && (!variableError));
  }

  //! Number of expansions made; each expansion means doing one math operation
  int expansions;
  //! Math error related to the computation
  int mathError;
  /** This is normally set to zero; if it isn't - then it probably means broken build,
   or implementation error - it is then set to the line number where the error occured */
  int logicError;
  /** This isn't exactly an error - it means that an unresolved variable was found;
    in the case of computeExpression() it doesn't even stop the computing - it continues
    in branches around the variable */
  bool variableError;
};

//! \class TreeParser Main parser class
/** TreeParser is a parser and evaluator of mathematical expressions. What it does basically is, given the string
   "(2-6)*4 + 8" will parse it, splitting the expression into symbols (tokens) in a tree structure (hence the name)
   and can compute the expression by subsequent expansions to give the end result: "-8".

   The expression can be set as string or as a list of tokens.

   Expressions can contain the symbols listed in TokenType. In addition to built-in operators and functions,
   there can also be user-defined variables, constants and functions.
 */
class TreeParser
{
    /** Token node/tree struct
      It contains the parsed input in the form of a binary tree. Because this format is explicit,
     it doesn't require brackets or commas, only the numbers/variables and operators */
    struct TokenNode
    {
      /** This contains the array of tokens only during parsing; after that it should contain
        only a single token */
      TokenArray tokens;
      TokenNode *leftChild, *rightChild;
      /** True if the token and its arguments should be enclosed in brackets;
        it is only an indicator - the tree contains no brackets */
      bool brackets;

      TokenNode();
      ~TokenNode();

      //! Returns a deep copy of object
      TokenNode* copy() const;

      //! Returns the linear string of tokens with brackets and commas
      TokenArray tokensArray() const;

      /** Divides the tokens into operator and arguments which are stored as children;
        it is the primary parsing function but it is 'dumb' - it doesn't make any checks */
      ParseStatus divide();
      //! This checks that the tree is valid
      ParseStatus check();
      /** This substitutes variables in input with known values of constants;
        the tokens are converted to numbers and treated as such */
      void substitute(const ValueMap &constants);

      /** This computes the end expression - by removing subsequent tokens as long as possible,
        or only one step if once is true */
      ComputeResult computeExpression(const PtrValueMap &variables, bool once = false);
      //! This computes only the value of the expression without removing any tokens
      ComputeResult computeValue(NumType &value, const PtrValueMap &variables) const;

      //! Returs a list of names of all tokens of the given type
      void listTokenNames(std::vector<std::string> &list, TokenType type) const;

      //! Prints the nodes in as 'dot' graph
      int print(std::ostream &out) const;

      //! Processes the node by computing the value of the operation
      ComputeResult process(NumType &value, const PtrValueMap &variables,
                            NumType *leftValue = 0, NumType *rightValue = 0) const;
    };

    //! Private constructor to support copying
    TreeParser(bool copy);
    void init();


    /** Copy constructor and assignment operator are currently blocked.
       If you want to have a copy of the object - use the functions
       shallowCopy() deepCopy(). */
    TreeParser(const TreeParser &);
    const TreeParser& operator=(const TreeParser &);

  public:
    //! Constructor; sets the expression to "0" (zero)
    TreeParser();

    //! Destructor
    ~TreeParser();

    //! Cleans all tokens and sets the expression to empty
    void reset();

    /** Parses the expression contained in string and returns true if successful;
      to see what errors were encoutered, use status() */
    bool setExpression(const std::string &expr);
    //! Converts the tokens back to string expression
    std::string expression() const;

    /** Re-parses the expression that was set using setExpression()
      Useful when, for instance when the constants, functions or variables change. */
    bool reparseExpression();

    //! Similar to setExpression() but instead of string takes an array of tokens
    bool setTokens(const TokenArray &tokens);
    //! Returns the list of tokens
    TokenArray tokens() const;

    //! Returns parse status
    inline ParseStatus status() const
      { return _status; }

    //! Returns a list of names of all variables in expression; the names can repeat
    std::vector<std::string> variablesInExpression() const;
    //! As above, returns a list of all external functions in expression
    std::vector<std::string> externalFunctionsInExpression() const;

    //! Returns constants map
    static inline const ValueMap& constants()
      { return _constants; }

    //! Sets constant, if successful returns true
    static bool setConstant(const std::string &name, NumType value);
    //! Removes constant, if successful returns true
    static bool unsetConstant(const std::string &name);
    //! Returns the value of given constant; exists is a boolean flag to tell if it exists
    static NumType constant(const std::string &name, bool *exists = 0);

    //! Return variables map
    inline const PtrValueMap& variables()
      { return _variables; }

    /** Set variable - takes a pointer to the value; the pointer must remain valid
        for the entire use of variable (until unset), but the pointed value may change.
        The variable's name can only contain letters and underscores. If invalid, the function will return false.
        If replace is true, the existing value will be replaced, otherwise, the function will return false. */
    bool setVariable(const std::string &name, NumType *value, bool replace = true);
    //! Removes the variable from list, forgetting the associated pointer; returns true if successful
    bool unsetVariable(const std::string &name);
    //! Returns pointer associated with given variable or NULL if it doesn't exist
    NumType* variable(const std::string &name);

    //! Sets the number precision
    inline static void setNumberPrecision(int vPrecision)
      { _numberPrecision = vPrecision; }
    //! Retusn the number precision
    inline static int numberPrecision()
      { return _numberPrecision; }

    //! Sets the number format
    inline static void setNumberFormat(NumberFormat vFormat)
      { _numberFormat = vFormat; }
    //! Returns the number format
    inline static NumberFormat numberFormat()
      { return _numberFormat; }

    //! Sets the pointer to _isFunction
    inline static void setIsFunction(bool (*isF)(const std::string&))
      { _isFunction = isF; }
    //! Sets the pointer to _getFunctionValue
    inline static void setGetFunctionValue(bool (*gFV)(const std::string&, NumType, NumType&))
      { _getFunctionValue = gFV; }

    /** Substitutes the constants in the expressions with the numbers in constants map;
      it is called when setting the expression/tokens but you can call it manually if you
      change the constants map */
    void substitute();

    /** Computes the end expression by removing subsequent tokens and substituting them
     with the results */
    ComputeResult computeExpression();
    //! Same as above but computes only one step of the expression
    ComputeResult computeExpressionStep();
    //! Computes only the value of the expression without removing any tokens
    ComputeResult computeValue(NumType &value) const;

    //! Prints the token tree as 'dot' graph
    void print(std::ostream &out = std::cout) const;

    /** Returns a shallow copy of the object (it shares the same pointer to the token tree)
     This object should be properly destroyed; destroying it doesn't destroy the token tree.
     The token tree is destroyed when the original object is destroyed. */
    TreeParser* shallowCopy() const;

    //! Returns true if the object is a shallow copy of another object
    inline bool isShallowCopy() const
      { return _isShallowCopy; }

    /** Returns a deep copy of the object (copies the whole token tree)
       The new object has all the properties of the original object and
       is completely independent. */
    TreeParser* deepCopy() const;

  private:
    //! True, if the object is a shallow copy
    const bool _isShallowCopy;
    //! The root node of the tree
    TokenNode *_root;
    //! Copy of the original expression; used by reparseExpression()
    std::string _originalExpression;
    //! Status of the parsing process
    ParseStatus _status;
    //! Map of variables (local)
    PtrValueMap _variables;

    //! Map of constants (shared between all objects)
    static ValueMap _constants;

    //! Format of numbers in strings returned by parser (shared between all objects)
    static NumberFormat _numberFormat;
    //! Precision of numbers in strings returned by parser (shared between all objects)
    static int _numberPrecision;

    //! Pointer to a function that checks whether 'func' is a name of available function
    static bool (*_isFunction)(const std::string &func);
    /** Pointer to a function that sets value to func(x), if func exists
        and x is in function's domain - then returns true; otherwise returns false */
    static bool (*_getFunctionValue)(const std::string &func, NumType x, NumType &value);
};

#endif // _QMPLOT_TREEPARSER_H
