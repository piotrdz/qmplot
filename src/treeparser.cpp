/* treeparser.cpp - implements the TreeParser class, which parses and evaluates arithmetic expressions.

 This file is part of QMPlot licensed under GPLv2.

 Copyright (C) Piotr Dziwinski 2009-2010
*/

#include "treeparser.h"

#include <cerrno>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cctype>

using namespace std;

/* NOTE: The following functions must be updated when adding/removing tokens
   - nameForToken()
   - priorityForToken()
   - argTypeForToken()
   - TreeParser::setExpression()
   - TreeParser::TreeNode::process()
*/


// Returns a string associated with the token
std::string nameForToken(const TokenType &type)
{
  string result;
  switch (type)
  {
    case TT_None:
    case TT_Number:
    case TT_Variable:
    case TT_ExternalFunction:
    {
      break;
    }
    case TT_LeftBracket:
    {
      result = "(";
      break;
    }
    case TT_RightBracket:
    {
      result = ")";
      break;
    }
    case TT_Comma:
    {
      result = ",";
      break;
    }
    case TT_Add:
    case TT_Plus:
    {
      result = "+";
      break;
    }
    case TT_Subtract:
    case TT_Minus:
    {
      result = "-";
      break;
    }
    case TT_Multiply:
    {
      result = "*";
      break;
    }
    case TT_Divide:
    {
      result = "/";
      break;
    }
    case TT_Modulus:
    {
      result = "%";
      break;
    }
    case TT_Power:
    {
      result = "^";
      break;
    }
    case TT_Factorial:
    {
      result = "!";
      break;
    }
    case TT_Abs:
    {
      result = "abs";
      break;
    }
    case TT_Sqrt:
    {
      result = "sqrt";
      break;
    }
    case TT_Exp:
    {
      result = "exp";
      break;
    }
    case TT_Ln:
    {
      result = "ln";
      break;
    }
    case TT_Log:
    {
      result = "log";
      break;
    }
    case TT_Sin:
    {
      result = "sin";
      break;
    }
    case TT_Cos:
    {
      result = "cos";
      break;
    }
    case TT_Tan:
    {
      result = "tan";
      break;
    }
    case TT_Asin:
    {
      result = "asin";
      break;
    }
    case TT_Acos:
    {
      result = "acos";
      break;
    }
    case TT_Atan:
    {
      result = "atan";
      break;
    }
    case TT_Sinh:
    {
      result = "sinh";
      break;
    }
    case TT_Cosh:
    {
      result = "cosh";
      break;
    }
    case TT_Tanh:
    {
      result = "tanh";
      break;
    }
    case TT_Signum:
    {
      result = "sgn";
      break;
    }
    case TT_Ceil:
    {
      result = "ceil";
      break;
    }
    case TT_Floor:
    {
      result = "floor";
      break;
    }
    case TT_Min:
    {
      result = "min";
      break;
    }
    case TT_Max:
    {
      result = "max";
      break;
    }
  }
  return result;
}

/* Returns the token priority; during parsing the token with the highest
  priority is selected as the top-level one */
int priorityForToken(const TokenType &type)
{
  int result = 0;
  switch (type)
  {
    case TT_None:
    case TT_Number:
    case TT_Variable:
    case TT_RightBracket:
    case TT_LeftBracket:
    {
      result = 0;
      break;
    }
    case TT_Comma:
    {
      result = 6;
      break;
    }
    case TT_Add:
    case TT_Subtract:
    {
      result = 5;
      break;
    }
    case TT_Plus:
    case TT_Minus:
    {
      result = 4;
      break;
    }
    case TT_Multiply:
    case TT_Divide:
    case TT_Modulus:
    {
      result = 3;
      break;
    }
    case TT_Power:
    case TT_Factorial:
    {
      result = 2;
      break;
    }
    case TT_Abs:
    case TT_Sqrt:
    case TT_Exp:
    case TT_Ln:
    case TT_Log:
    case TT_Sin:
    case TT_Cos:
    case TT_Tan:
    case TT_Asin:
    case TT_Acos:
    case TT_Atan:
    case TT_Sinh:
    case TT_Cosh:
    case TT_Tanh:
    case TT_Signum:
    case TT_Ceil:
    case TT_Floor:
    case TT_Min:
    case TT_Max:
    case TT_ExternalFunction:
    {
      result = 1;
      break;
    }
  }
  return result;
}

// Returns the type of argument(s) that the token takes
ArgType argTypeForToken(const TokenType &type)
{
  ArgType result = AT_Standalone;
  switch (type)
  {
    case TT_None:
    case TT_Number:
    case TT_Variable:
    case TT_LeftBracket:
    case TT_RightBracket:
    {
      result = AT_Standalone;
      break;
    }
    case TT_Comma:
    case TT_Add:
    case TT_Subtract:
    case TT_Multiply:
    case TT_Divide:
    case TT_Power:
    case TT_Modulus:
    {
      result = AT_Binary;
      break;
    }
    case TT_Plus:
    case TT_Minus:
    case TT_Abs:
    case TT_Sqrt:
    case TT_Exp:
    case TT_Ln:
    case TT_Log:
    case TT_Sin:
    case TT_Cos:
    case TT_Tan:
    case TT_Asin:
    case TT_Acos:
    case TT_Atan:
    case TT_Sinh:
    case TT_Cosh:
    case TT_Tanh:
    case TT_Signum:
    case TT_Ceil:
    case TT_Floor:
    case TT_ExternalFunction:
    {
      result = AT_RightUnary;
      break;
    }
    case TT_Factorial:
    {
      result = AT_LeftUnary;
      break;
    }
    case TT_Min:
    case TT_Max:
    {
      result = AT_CommaBinary;
      break;
    }
  }
  return result;
}


Token::Token() : _type(TT_None)
{
  _position = -1;
  _number = 0.0;
}

Token::Token(const TokenType &vType, const int &vPosition)
  : _type(vType), _position(vPosition)
{
  _name = nameForToken(_type);
  _number = 0.0;
}

int Token::priority() const
{
  return priorityForToken(_type);
}

ArgType Token::argType() const
{
  return argTypeForToken(_type);
}

void Token::changeType(const TokenType &newType)
{
  _type = newType;
  _name = nameForToken(_type);
  _number = 0.0;
}

std::string ParseStatus::errorString() const
{
  string result = "";
  switch (error)
  {
    case PE_None:
    {
      break;
    }
    case PE_EmptyExpression:
    {
      result = "empty expression";
      break;
    }
    case PE_EmptyBrackets:
    {
      result = "empty brackets";
      break;
    }
    case PE_InvalidNumber:
    {
      result = "invalid number";
      break;
    }
    case PE_InvalidCharacter:
    {
      result = "invalid character";
      break;
    }
    case PE_MismatchedBrackets:
    {
      result = "mismatched brackets";
      break;
    }
    case PE_MisplacedComma:
    {
      result = "misplaced comma";
      break;
    }
    case PE_MissingArgument:
    {
      result = "missing argument";
      break;
    }
    case PE_ExtraArgument:
    {
      result = "extra argument";
      break;
    }
    case PE_InvalidArgument:
    {
      result = "invalid argument";
      break;
    }
    case PE_GeneralError:
    {
      result = "general error";
      break;
    }
    case PE_LogicError:
    {
      result = "logic error [bug?]";
      break;
    }
  }
  return result;
}

std::string ComputeResult::errorString() const
{
  string result = "";
  if (logicError != 0)
  {
    result = "logic error [bug?]";
  }
  if (mathError != 0)
  {
    switch (mathError)
    {
      case ME_InvalidExpression:
      {
        result = "invalid expression";
        break;
      }
      case ME_DivisionByZero:
      {
        result = "division by zero";
        break;
      }
      case ME_RangeError:
      {
        result = "range error";
        break;
      }
      case ME_DomainError:
      {
        result = "domain error";
        break;
      }
    }
  }

  return result;
}


TreeParser::TokenNode::TokenNode()
{
  leftChild = rightChild = NULL;
  brackets = false;
}

TreeParser::TokenNode::~TokenNode()
{
  if (leftChild != NULL)
  {
    delete leftChild;
    leftChild = NULL;
  }
  if (rightChild != NULL)
  {
    delete rightChild;
    rightChild = NULL;
  }
}

// Returns a deep copy of TokenNode
TreeParser::TokenNode* TreeParser::TokenNode::copy() const
{
  TokenNode* result = new TokenNode();
  result->tokens = tokens;
  result->brackets = brackets;

  if (leftChild != NULL)
    result->leftChild = leftChild->copy();

  if (rightChild != NULL)
    result->rightChild = rightChild->copy();

  return result;
}

// Parses the tokens contained in the node
ParseStatus TreeParser::TokenNode::divide()
{
  ParseStatus result;
  if (tokens.empty())
  {
    result.error = PE_EmptyExpression;
    result.code = __LINE__;
    return result;
  }

  // Find and fix missing multiplications
  bool changed = false;
  for (TokenIterator token = tokens.begin(); token != tokens.end(); ++token)
  {
    TokenIterator next = token + 1;
    if (next != tokens.end())
    {
      if ( ((*token).type() == TT_Number) &&
              (((*next).type() == TT_Variable) || ((*next).type() == TT_LeftBracket)) )
      {
        ++token;
        token = tokens.insert(token, Token(TT_Multiply));
        changed = true;
      }
      else if ( ((*token).type() == TT_Variable) &&
                (((*next).type() == TT_Variable) || ((*next).type() == TT_LeftBracket)) )
      {
        ++token;
        token = tokens.insert(token, Token(TT_Multiply));
        changed = true;
      }
      else if ( ((*token).type() == TT_RightBracket) && ((*next).type() == TT_LeftBracket) )
      {
        ++token;
        token = tokens.insert(token, Token(TT_Multiply));
        changed = true;
      }
    }
  }
  if (changed)
    return divide();

  // Delete the unnecessary enclosing brackets in the expression
  bool done = false;
  while (!done)
  {
    done = true;
    if (tokens.front().type() == TT_LeftBracket)
    {
      int bracketLevel = 0;
      unsigned int index = 0;
      unsigned int i = 0;
      for (TokenIterator token = tokens.begin(); token != tokens.end(); ++token, ++i)
      {
        if ((*token).type() == TT_LeftBracket)
          ++bracketLevel;
        else if ((*token).type() == TT_RightBracket)
        {
          --bracketLevel;
          if (bracketLevel == 0)
          {
            index = i;
            break;
          }
        }
      }
      if (index + 1 == tokens.size())
      {
        tokens.erase(tokens.begin());
        tokens.erase(tokens.end() - 1);
        brackets = true;
        done = false;
      }
    }
  }

  if (tokens.empty())
  {
    result.error = PE_EmptyBrackets;
    result.code = __LINE__;
    return result;
  }

  if (tokens.size() > 1)
  {
    // Change add and subtract to plus and minus
    if (tokens.front().type() == TT_Add)
      tokens.front().changeType(TT_Plus);
    else if (tokens.front().type() == TT_Subtract)
      tokens.front().changeType(TT_Minus);

    // Search backwards for the token with highest priority
    TokenIterator token = tokens.end() - 1;
    TokenIterator midpoint = tokens.end();
    int bracketLevel = 0;
    int highestPriority = 0;
    bool done = false;
    while (!done)
    {
      if ((*token).type() == TT_LeftBracket)
        --bracketLevel;

      else if ((*token).type() == TT_RightBracket)
        ++bracketLevel;

      else if ((bracketLevel == 0) && ((*token).priority() != 0))
      {
        if ((*token).priority() > highestPriority)
        {
          highestPriority = (*token).priority();
          midpoint = token;
        }
      }

      if (token == tokens.begin())
        done = true;
      else
        --token;
    }

    if (midpoint == tokens.end())
    {
      result.error = PE_GeneralError;
      result.token = tokens.front();
      result.position = tokens.front().position();
      result.code = __LINE__;
      return result;
    }
    else // midpoint != tokens.end()
    {
      if (leftChild != NULL)
      {
        delete leftChild;
        leftChild = NULL;
      }

      if (rightChild != NULL)
      {
        delete rightChild;
        rightChild = NULL;
      }

      TokenArray leftTokens;
      TokenArray rightTokens;

      for (token = tokens.begin(); token != midpoint; ++token)
        leftTokens.push_back(*token);

      for (token = midpoint + 1; token != tokens.end(); ++token)
        rightTokens.push_back(*token);

      if (rightTokens.empty() && leftTokens.empty())
      {
        result.error = PE_MissingArgument;
        result.token = (*midpoint);
        result.position = (*midpoint).position();
        result.code = __LINE__;
        return result;
      }

      if (!leftTokens.empty())
      {
        leftChild = new TokenNode();
        leftChild->tokens = leftTokens;
        result = leftChild->divide();
        if (result.error != PE_None) return result;
      }

      if (!rightTokens.empty())
      {
        rightChild = new TokenNode();
        rightChild->tokens = rightTokens;
        result = rightChild->divide();
        if (result.error != PE_None) return result;
      }

      Token midToken = (*midpoint);
      tokens.clear();
      tokens.push_back(midToken);
    }
  }

  return result;
}

ParseStatus TreeParser::TokenNode::check()
{
  ParseStatus result;
  if (tokens.empty())
  {
    result.error = PE_EmptyExpression;
    return result;
  }

  if (tokens.size() > 1)
  {
    result.error = PE_GeneralError;
    result.token = tokens.front();
    result.position = tokens.front().position();
    result.code = __LINE__;
    return result;
  }

  if ((tokens.front().type() == TT_LeftBracket)
       || (tokens.front().type() == TT_RightBracket))
  {
    result.error = PE_LogicError;
    result.token = tokens.front();
    result.position = tokens.front().position();
    result.code = __LINE__;
    return result;
  }

  if (leftChild != NULL)
  {
    result = leftChild->check();
    if (result.error != PE_None)
    {
      return result;
    }
  }

  if (rightChild != NULL)
  {
    result = rightChild->check();
    if (result.error != PE_None)
    {
      return result;
    }
  }

  switch (tokens.front().argType())
  {
    case AT_Standalone:
    {
      if ((leftChild != NULL) || (rightChild != NULL))
      {
        result.error = PE_LogicError;
        result.token = tokens.front();
        result.position = tokens.front().position();
        result.code = __LINE__;
        return result;
      }
      break;
    }
    case AT_Binary:
    {
      if ((leftChild == NULL) || (rightChild == NULL))
      {
        result.error = PE_MissingArgument;
        result.token = tokens.front();
        result.position = tokens.front().position();
        result.code = __LINE__;
        return result;
      }
      break;
    }
    case AT_RightUnary:
    {
      if (rightChild == NULL)
      {
        result.error = PE_MissingArgument;
        result.token = tokens.front();
        result.position = tokens.front().position();
        result.code = __LINE__;
        return result;
      }
      if (leftChild != NULL)
      {
        if ((leftChild->tokens.front().type() == TT_Number) ||
             (leftChild->tokens.front().type() == TT_Number))
        {
          TokenNode *newNode = new TokenNode();
          newNode->tokens.push_back(tokens.front());
          tokens.front().changeType(TT_Multiply);
          newNode->rightChild = rightChild;
          rightChild = newNode;
          newNode = NULL;
        }
        else
        {
          result.error = PE_ExtraArgument;
          result.token = tokens.front();
          result.position = tokens.front().position();
          result.code = __LINE__;
          return result;
        }
      }
      break;
    }
    case AT_LeftUnary:
    {
      if (leftChild == NULL)
      {
        result.error = PE_MissingArgument;
        result.token = tokens.front();
        result.position = tokens.front().position();
        result.code = __LINE__;
        return result;
      }
      if (rightChild != NULL)
      {
        result.error = PE_ExtraArgument;
        result.token = tokens.front();
        result.position = tokens.front().position();
        result.code = __LINE__;
        return result;
      }
      break;
    }
    case AT_CommaBinary:
    {
      if (rightChild == NULL)
      {
        result.error = PE_MissingArgument;
        result.token = tokens.front();
        result.position = tokens.front().position();
        result.code = __LINE__;
        return result;
      }
      if (leftChild != NULL)
      {
        result.error = PE_ExtraArgument;
        result.token = tokens.front();
        result.position = tokens.front().position();
        result.code = __LINE__;
        return result;
      }
      break;
    }
  }

  // Delete comma node
  if (tokens.front().argType() == AT_CommaBinary)
  {
    if (rightChild->tokens.front().type() != TT_Comma)
    {
      result.error = PE_InvalidArgument;
      result.token = tokens.front();
      result.position = tokens.front().position();
      result.code = __LINE__;
      return result;
    }
    if ((rightChild->leftChild->tokens.front().type() == TT_Comma) ||
         (rightChild->rightChild->tokens.front().type() == TT_Comma))
    {
      result.error = PE_ExtraArgument;
      result.token = tokens.front();
      result.position = tokens.front().position();
      result.code = __LINE__;
      return result;
    }

    TokenNode *commaNode = rightChild;
    leftChild = commaNode->leftChild;
    commaNode->leftChild = NULL;
    rightChild = commaNode->rightChild;
    commaNode->rightChild = NULL;
    delete commaNode;
    commaNode = NULL;
  }
  else
  {
    if (leftChild != NULL)
    {
      if (leftChild->tokens.front().type() == TT_Comma)
      {
        result.error = PE_MisplacedComma;
        result.token = leftChild->tokens.front();
        result.position = leftChild->tokens.front().position();
        result.code = __LINE__;
        return result;
      }
    }
    if (rightChild != NULL)
    {
      if (rightChild->tokens.front().type() == TT_Comma)
      {
        result.error = PE_MisplacedComma;
        result.token = rightChild->tokens.front();
        result.position = rightChild->tokens.front().position();
        result.code = __LINE__;
        return result;
      }
    }
  }

  return result;
}

int TreeParser::TokenNode::print(std::ostream &out) const
{
  static int count = 0;
  int result = 0;

  string expression;
  ConstTokenIterator currentToken = tokens.begin();
  while (currentToken != tokens.end())
  {
    if ((*currentToken).type() == TT_Number)
    {
      stringstream stream;
      stream << (*currentToken).number();
      expression += stream.str();
    }
    else
      expression += (*currentToken).name();

    if (++currentToken != tokens.end())
      expression += ' ';
  }

  if (expression.empty())
    expression = "empty";

  out << "\tnode" << count << " [label=\"" << expression << "\"]" << endl;

  result = count;
  ++count;

  int leftIndex = -1;
  int rightIndex = -1;

  if (leftChild != NULL)
    leftIndex = leftChild->print(out);

  if (rightChild != NULL)
    rightIndex = rightChild->print(out);

  if (leftIndex != -1)
    out << "\tnode" << result << " -> " << "node" << leftIndex << endl;

  if (rightIndex != -1)
    out << "\tnode" << result << " -> " << "node" << rightIndex << endl;

  return result;
}

TokenArray TreeParser::TokenNode::tokensArray() const
{
  TokenArray result;
  if ((leftChild != NULL) || (rightChild != NULL))
  {
    TokenArray leftArray, rightArray;
    if (leftChild != NULL)
    {
      if (leftChild->brackets)
        leftArray.push_back(Token(TT_LeftBracket));

      TokenArray temp = leftChild->tokensArray();
      for (ConstTokenIterator token = temp.begin(); token != temp.end(); ++token)
        leftArray.push_back(*token);

      if (leftChild->brackets)
        leftArray.push_back(Token(TT_RightBracket));
    }
    if (rightChild != NULL)
    {
      if (rightChild->brackets)
        rightArray.push_back(Token(TT_LeftBracket));

      TokenArray temp = rightChild->tokensArray();
      for (ConstTokenIterator token = temp.begin(); token != temp.end(); ++token)
        rightArray.push_back(*token);

      if (rightChild->brackets)
        rightArray.push_back(Token(TT_RightBracket));
    }

    // Special case - for example min(a,b) - add brackets and comma
    if (tokens.front().argType() == AT_CommaBinary)
    {
      for (ConstTokenIterator token = tokens.begin(); token != tokens.end(); ++token)
        result.push_back(*token);

      result.push_back(Token(TT_LeftBracket));

      for (ConstTokenIterator token = leftArray.begin(); token != leftArray.end(); ++token)
        result.push_back(*token);

      result.push_back(Token(TT_Comma));

      for (ConstTokenIterator token = rightArray.begin(); token != rightArray.end(); ++token)
        result.push_back(*token);

      result.push_back(Token(TT_RightBracket));
    }
    else
    {
      if (!leftArray.empty())
      {
        for (ConstTokenIterator token = leftArray.begin(); token != leftArray.end(); ++token)
          result.push_back(*token);
      }

      for (ConstTokenIterator token = tokens.begin(); token != tokens.end(); ++token)
        result.push_back(*token);

      if (!rightArray.empty())
      {
        for (ConstTokenIterator token = rightArray.begin(); token != rightArray.end(); ++token)
          result.push_back(*token);
      }
    }
  }
  else
  {
    result = tokens;
  }
  return result;
}

void TreeParser::TokenNode::substitute(const ValueMap &constants)
{
  if (tokens.front().type() == TT_Variable)
  {
    ConstValueMapIterator it = constants.find(tokens.front().name());
    if (it != constants.end())
    {
      tokens.front().changeType(TT_Number);
      tokens.front().setNumber((*it).second);
    }
  }

  if (leftChild != NULL) leftChild->substitute(constants);
  if (rightChild != NULL) rightChild->substitute(constants);
}

ComputeResult TreeParser::TokenNode::computeExpression(const PtrValueMap &variables, bool once)
{
  ComputeResult result;

  if ((leftChild == NULL) && (rightChild == NULL)) return result;

  if (leftChild != NULL)
  {
    if ((leftChild->leftChild != NULL) || (leftChild->rightChild != NULL))
    {
      ComputeResult childResult = leftChild->computeExpression(variables, once);
      result.join(childResult);

      if (once) return result;

      if ((result.logicError != 0) || (result.mathError != 0)) return result;
    }
  }

  if (rightChild != NULL)
  {
    if ((rightChild->leftChild != NULL) || (rightChild->rightChild != NULL))
    {
      ComputeResult childResult = rightChild->computeExpression(variables, once);
      result.join(childResult);

      if (once) return result;

      if ((result.logicError != 0) || (result.mathError != 0)) return result;
    }
  }

  NumType value = 0.0;
  ComputeResult processResult = process(value, variables);
  result.join(processResult);
  if ((result.logicError == 0) && (result.mathError == 0) && (!result.variableError))
  {
    Token &thisToken = tokens.front();
    thisToken.changeType(TT_Number);
    thisToken.setNumber(value);

    if (leftChild != NULL)
    {
      delete leftChild;
      leftChild = NULL;
    }

    if (rightChild != NULL)
    {
      delete rightChild;
      rightChild = NULL;
    }
  }

  return result;
}

ComputeResult TreeParser::TokenNode::computeValue(NumType &value,
     const PtrValueMap &variables) const
{
  ComputeResult result;
  if ((leftChild == NULL) && (rightChild == NULL))
  {
    if (tokens.front().type() == TT_Number)
      value = tokens.front().number();
    else if (tokens.front().type() == TT_Variable)
    {
      ConstPtrValueMapIterator it = variables.find(tokens.front().name());
      if (it != variables.end())
        value = *((*it).second);
      else
        result.variableError = true;
    }
    else
      result.logicError = __LINE__;

    return result;
  }

  NumType leftValue = 0.0;

  if (leftChild != NULL)
  {
    ComputeResult childResult = leftChild->computeValue(leftValue, variables);
    result.join(childResult);

    if ((result.logicError != 0) || (result.mathError != 0) || result.variableError) return result;
  }

  NumType rightValue = 0.0;

  if (rightChild != NULL)
  {
    ComputeResult childResult = rightChild->computeValue(rightValue, variables);
    result.join(childResult);

    if ((result.logicError != 0) || (result.mathError != 0) || result.variableError) return result;
  }

  ComputeResult processResult = process(value, variables, &leftValue, &rightValue);
  result.join(processResult);

  return result;
}

void TreeParser::TokenNode::listTokenNames(std::vector<std::string> &list, TokenType type) const
{
  if (tokens.front().type() == type)
  {
    list.push_back(tokens.front().name());
  }

  if (leftChild != NULL)
  {
    leftChild->listTokenNames(list, type);
  }

  if (rightChild != NULL)
  {
    rightChild->listTokenNames(list, type);
  }
}

ComputeResult TreeParser::TokenNode::process(NumType &value, const PtrValueMap &variables,
                                             NumType *leftValue, NumType *rightValue) const
{
  ComputeResult result;
  const Token &thisToken = tokens.front();
  Token leftToken;
  if (leftValue != NULL)
  {
    leftToken = Token(TT_Number);
    leftToken.setNumber(*leftValue);
  }
  else if (leftChild != NULL)
  {
    if (leftChild->tokens.size() != 1)
    {
      result.logicError = __LINE__;
      return result;
    }

    leftToken = leftChild->tokens.front();

    if (leftToken.type() == TT_Variable)
    {
      ConstPtrValueMapIterator it = variables.find(leftToken.name());
      if (it != variables.end())
      {
        leftToken.changeType(TT_Number);
        leftToken.setNumber(*((*it).second));
      }
      else
      {
        result.variableError = true;
        return result;
      }
    }
  }

  Token rightToken;
  if (rightValue != NULL)
  {
    rightToken = Token(TT_Number);
    rightToken.setNumber(*rightValue);
  }
  else if (rightChild != NULL)
  {
    if (rightChild->tokens.size() != 1)
    {
      result.logicError = __LINE__;
      return result;
    }
    rightToken = rightChild->tokens.front();

    if (rightToken.type() == TT_Variable)
    {
      ConstPtrValueMapIterator it = variables.find(rightToken.name());
      if (it != variables.end())
      {
        rightToken.changeType(TT_Number);
        rightToken.setNumber(*((*it).second));
      }
      else
      {
        result.variableError = true;
        return result;
      }
    }
  }

  switch (thisToken.type())
  {
    // Should not happen
    case TT_Number:
    case TT_Variable:
    {
      result.logicError = __LINE__;
      break;
    }
    case TT_Add:
    {
      if ((leftToken.type() == TT_Number) && (rightToken.type() == TT_Number))
      {
        value = leftToken.number() + rightToken.number();

        ++result.expansions;
      }
      break;
    }
    case TT_Plus:
    {
      if (rightToken.type() == TT_Number)
      {
        value = rightToken.number();

        ++result.expansions;
      }
      break;
    }
    case TT_Subtract:
    {
      if ((leftToken.type() == TT_Number) && (rightToken.type() == TT_Number))
      {
        value = leftToken.number() - rightToken.number();

        ++result.expansions;
      }
      break;
    }
    case TT_Minus:
    {
      if (rightToken.type() == TT_Number)
      {
        value = -rightToken.number();

        ++result.expansions;
      }
      break;
    }
    case TT_Multiply:
    {
      if ((leftToken.type() == TT_Number) && (rightToken.type() == TT_Number))
      {
        value = leftToken.number() * rightToken.number();

        ++result.expansions;
      }
      break;
    }
    case TT_Divide:
    {
      if ((leftToken.type() == TT_Number) && (rightToken.type() == TT_Number))
      {
        if (rightToken.number() == 0.0)
        {
          result.mathError = ME_DivisionByZero;
          break;
        }

        value = leftToken.number() / rightToken.number();

        ++result.expansions;
      }
      break;
    }
    case TT_Power:
    {
      if ((leftToken.type() == TT_Number) && (rightToken.type() == TT_Number))
      {
        // Reset errno
        errno = 0;

        value = pow(leftToken.number(), rightToken.number());
        if (errno == ERANGE)
        {
          result.mathError = ME_RangeError;
          break;
        }
        else if (errno == EDOM)
        {
          result.mathError = ME_DomainError;
          break;
        }
        // Normally, there should not be any other errrors
        else if (errno != 0)
        {
          result.logicError = __LINE__;
          break;
        }

        ++result.expansions;
      }
      break;
    }
    case TT_Modulus:
    {
      if ((leftToken.type() == TT_Number) && (rightToken.type() == TT_Number))
      {
        if (rightToken.number() == 0.0)
        {
          result.mathError = ME_DivisionByZero;
          break;
        }

        value = fmod(leftToken.number(), rightToken.number());

        ++result.expansions;
      }
      break;
    }
    case TT_Factorial:
    {
      if (leftToken.type() == TT_Number)
      {
        if (leftToken.number() < 0.0)
        {
          result.mathError = ME_DomainError;
          break;
        }

        value = 1.0;
        for (NumType i = 1.0; i < leftToken.number() + 1.0; ++i) value *= i;

        ++result.expansions;
      }
      break;
    }
    // Functions computed using standard library
    case TT_Abs:
    case TT_Sqrt:
    case TT_Exp:
    case TT_Ln:
    case TT_Log:
    case TT_Sin:
    case TT_Cos:
    case TT_Tan:
    case TT_Asin:
    case TT_Acos:
    case TT_Atan:
    case TT_Sinh:
    case TT_Cosh:
    case TT_Tanh:
    case TT_Ceil:
    case TT_Floor:
    {
      if (rightToken.type() != TT_Number) break;

      // Remember to reset errno
      errno = 0;

      if (thisToken.type() == TT_Abs)
      {
        value = fabs(rightToken.number());
      }
      else if (thisToken.type() == TT_Sqrt)
      {
        value = sqrt(rightToken.number());
      }
      else if (thisToken.type() == TT_Exp)
      {
        value = exp(rightToken.number());
      }
      else if (thisToken.type() == TT_Ln)
      {
        value = log(rightToken.number());
      }
      else if (thisToken.type() == TT_Log)
      {
        value = log10(rightToken.number());
      }
      else if (thisToken.type() == TT_Sin)
      {
        value = sin(rightToken.number());
      }
      else if (thisToken.type() == TT_Cos)
      {
        value = cos(rightToken.number());
      }
      else if (thisToken.type() == TT_Tan)
      {
        value = tan(rightToken.number());
      }
      else if (thisToken.type() == TT_Asin)
      {
        value = asin(rightToken.number());
      }
      else if (thisToken.type() == TT_Acos)
      {
        value = acos(rightToken.number());
      }
      else if (thisToken.type() == TT_Atan)
      {
        value = atan(rightToken.number());
      }
      else if (thisToken.type() == TT_Sinh)
      {
        value = sinh(rightToken.number());
      }
      else if (thisToken.type() == TT_Cosh)
      {
        value = cosh(rightToken.number());
      }
      else if (thisToken.type() == TT_Tanh)
      {
        value = tanh(rightToken.number());
      }
      else if (thisToken.type() == TT_Ceil)
      {
        value = ceil(rightToken.number());
      }
      else if (thisToken.type() == TT_Floor)
      {
        value = floor(rightToken.number());
      }

      // Handle errors (if any)
      if (errno == ERANGE)
      {
        result.mathError = ME_RangeError;
        break;
      }
      else if (errno == EDOM)
      {
        result.mathError = ME_DomainError;
        break;
      }
      // Normally, there should not be any other errors
      else if (errno != 0)
      {
        result.logicError = __LINE__;
        break;
      }

      ++result.expansions;
      break;
    }
    case TT_Signum:
    {
      if (rightToken.type() == TT_Number)
      {
        value = 0.0;
        if (rightToken.number() < 0.0) value = -1.0;
        else if (rightToken.number() > 0.0) value = 1.0;
        else value = 0.0;

        ++result.expansions;
      }
      break;
    }
    case TT_Min:
    case TT_Max:
    {
      if ((leftToken.type() == TT_Number) && (rightToken.type() == TT_Number))
      {
        if (thisToken.type() == TT_Min)
        {
          if (leftToken.number() < rightToken.number()) value = leftToken.number();
          else value = rightToken.number();
        }
        // TT_Max
        else
        {
          if (leftToken.number() < rightToken.number()) value = rightToken.number();
          else value = leftToken.number();
        }

        ++result.expansions;
      }
      break;
    }
    case TT_ExternalFunction:
    {
      if (_getFunctionValue == NULL)
      {
        // Treat it as a double error
        result.mathError = ME_DomainError;
        result.logicError = __LINE__;
        break;
      }

      if (rightToken.type() == TT_Number)
      {
        bool ok = _getFunctionValue(thisToken.name(), rightToken.number(), value);
        if (!ok) result.mathError = ME_DomainError;
        else ++result.expansions;
      }

      break;
    }
    case TT_None:
    case TT_LeftBracket:
    case TT_RightBracket:
    case TT_Comma:
    {
      result.logicError = __LINE__;
      break;
    }
  }

  return result;
}

TreeParser::TreeParser() : _isShallowCopy(false)
{
  init();
}

/* private */ TreeParser::TreeParser(bool copy) : _isShallowCopy(copy)
{
  _root = NULL;
}

void TreeParser::init()
{
  _root = new TokenNode();
  Token zero(TT_Number, 0.0);
  _root->tokens.push_back(zero);
  if (_constants.empty())
  {
    _constants.insert(make_pair<std::string, NumType>("pi", M_PI));
    _constants.insert(make_pair<std::string, NumType>("e", M_E));
  }
  reset();
}

TreeParser::~TreeParser()
{
  if (!_isShallowCopy) delete _root;
  _root = NULL;
}

/* private */ TreeParser::TreeParser(const TreeParser &) : _isShallowCopy(false)
{
}

/* private */ const TreeParser& TreeParser::operator=(const TreeParser &)
{
  return *this;
}

TreeParser* TreeParser::shallowCopy() const
{
  TreeParser *result = new TreeParser(true);
  result->_root = _root;
  result->_originalExpression = _originalExpression;
  result->_status = _status;
  result->_variables = _variables;
  return result;
}

TreeParser* TreeParser::deepCopy() const
{
  TreeParser *result = new TreeParser(false);
  result->_root = _root->copy();
  result->_originalExpression = _originalExpression;
  result->_status = _status;
  result->_variables = _variables;
  return result;
}

void TreeParser::reset()
{
  if (_root->leftChild != NULL)
  {
    delete _root->leftChild;
    _root->leftChild = NULL;
  }
  if (_root->rightChild != NULL)
  {
    delete _root->rightChild;
    _root->rightChild = NULL;
  }
  _root->tokens.clear();
  static const Token zero(TT_Number, 0);
  _root->tokens.push_back(zero);
  _status.reset();
}

bool TreeParser::setExpression(const std::string &expr)
{
  reset();

  _originalExpression = expr;

  if (expr.empty())
  {
    _status.error = PE_EmptyExpression;
    return false;
  }

  TokenArray tokens;

  size_t index = 0;
  while (index < expr.size())
  {
    char character = expr.at(index);
    // Skip whitespace
    if ((character == ' ') || (character == '\t') || (character == '\r')
          || (character == '\n'))
    {
      ++index;
    }
    // Read a number
    /* Valid formats are for example: '3.4' '.4' '3.' and variants with an exponent,
     for example 'e5' at the end */
    else if (((character >= '0') && (character <= '9')) || (character == '.'))
    {
      string numberString;
      numberString += character;
      bool hasDot = (character == '.');
      bool hasE = false;
      while (++index < expr.size())
      {
        if ((expr.at(index) >= '0') && (expr.at(index) <= '9'))
          numberString += expr.at(index);
        else if ((expr.at(index) == '.'))
        {
          // Can't have two dots
          if (hasDot)
          {
            _status.error = PE_InvalidNumber;
            _status.position = index + 1;
            if (!tokens.empty()) _status.token = tokens.back();
            return false;
          }

          hasDot = true;
          numberString += '.';
        }
        // Try to interpret the e as exponent suffix, but don't go too far if it seems invalid
        else if ((tolower(expr.at(index)) == 'e'))
        {
          // Can't have two e's
          if (hasE)
          {
            _status.error = PE_InvalidNumber;
            _status.position = index + 1;
            if (!tokens.empty()) _status.token = tokens.back();
            return false;
          }

          if (index + 1 >= expr.size()) break;

          if ((expr.at(index + 1) >= '0') &&
               (expr.at(index + 1) <= '9'))
          {
            numberString += 'e';
            numberString += expr.at(++index);
            // Set the flags and continue with the loop, reading the exponent digits
            hasDot = hasE = true;
          }
          else if ((expr.at(index + 1) == '+') ||
                    (expr.at(index + 1) == '-'))
          {
            if (index + 2 >= expr.size()) break;

            if ((expr.at(index + 2) >= '0') &&
                 (expr.at(index + 2) <= '9'))
            {
              numberString += 'e';
              numberString += expr.at(++index);
              numberString += expr.at(++index);
              // As above, continue with the loop
              hasDot = hasE = true;
            }
            else break;
          }
          // expr.at(index + 1) != '+' '-', '0' - '9'
          else break;
        }
        // expr.at(index) != '0' - '9', '.', 'e'
        else break;
      }

      stringstream stream;
      stream.str(numberString);
      NumType number = 0.0;
      stream >> number;

      Token token(TT_Number, index + 1);
      token.setNumber(number);
      tokens.push_back(token);
    }
    else if (((tolower(character) >= 'a') && (tolower(character) <= 'z')))
    {
      string text;
      text += tolower(character);
      while (++index < expr.size())
      {
        if (((tolower(expr.at(index)) >= 'a') &&
              (tolower(expr.at(index)) <= 'z')) ||
              (expr.at(index) == '_'))
          text += tolower(expr.at(index));
        else break;
      }

      if (text == "abs")
      {
        tokens.push_back(Token(TT_Abs, index + 1));
      }
      else if (text == "sqrt")
      {
        tokens.push_back(Token(TT_Sqrt, index + 1));
      }
      else if (text == "exp")
      {
        tokens.push_back(Token(TT_Exp, index + 1));
      }
      else if (text == "ln")
      {
        tokens.push_back(Token(TT_Ln, index + 1));
      }
      else if (text == "log")
      {
        tokens.push_back(Token(TT_Log, index + 1));
      }
      else if (text == "sin")
      {
        tokens.push_back(Token(TT_Sin, index + 1));
      }
      else if (text == "cos")
      {
        tokens.push_back(Token(TT_Cos, index + 1));
      }
      else if ((text == "tg") || (text == "tan"))
      {
        tokens.push_back(Token(TT_Tan, index + 1));
      }
      else if ((text == "asin") || (text == "arcsin"))
      {
        tokens.push_back(Token(TT_Asin, index + 1));
      }
      else if ((text == "acos") || (text == "arccos"))
      {
        tokens.push_back(Token(TT_Acos, index + 1));
      }
      else if ((text == "atan") || (text == "arctan"))
      {
        tokens.push_back(Token(TT_Atan, index + 1));
      }
      else if (text == "min")
      {
        tokens.push_back(Token(TT_Min, index + 1));
      }
      else if (text == "max")
      {
        tokens.push_back(Token(TT_Max, index + 1));
      }
      else if ((text == "sgn") || (text == "signum"))
      {
        tokens.push_back(Token(TT_Signum, index + 1));
      }
      else if (text == "ceil")
      {
        tokens.push_back(Token(TT_Ceil, index + 1));
      }
      else if (text == "floor")
      {
        tokens.push_back(Token(TT_Floor, index + 1));
      }
      else if (text == "sinh")
      {
        tokens.push_back(Token(TT_Sinh, index + 1));
      }
      else if (text == "cosh")
      {
        tokens.push_back(Token(TT_Cosh, index + 1));
      }
      else if (text == "tanh")
      {
        tokens.push_back(Token(TT_Tanh, index + 1));
      }
      else
      {
        if ((_isFunction != NULL) && (_isFunction(text)))
        {
          Token token(TT_ExternalFunction, index + 1);
          token.setName(text);
          tokens.push_back(token);
        }
        else
        {
          Token token(TT_Variable, index + 1);
          token.setName(text);
          tokens.push_back(token);
        }
      }
    }
    else
    {
      if ((character == '(') || (character == '[') || (character == '{'))
      {
        tokens.push_back(Token(TT_LeftBracket));
      }
      else if ((character == ')') || (character == ']') || (character == '}'))
      {
        tokens.push_back(Token(TT_RightBracket));
      }
      else if (character == '+')
      {
        tokens.push_back(Token(TT_Add));
      }
      else if (character == '-')
      {
        tokens.push_back(Token(TT_Subtract));
      }
      else if (character == '*')
      {
        tokens.push_back(Token(TT_Multiply));
      }
      else if (character == '/')
      {
        tokens.push_back(Token(TT_Divide));
      }
      else if (character == '|')
      {
        tokens.push_back(Token(TT_Modulus));
      }
      else if (character == '^')
      {
        tokens.push_back(Token(TT_Power));
      }
      else if ((character == ',') || (character == ';'))
      {
        tokens.push_back(Token(TT_Comma));
      }
      else if (character == '!')
      {
        tokens.push_back(Token(TT_Factorial));
      }
      else
      {
        _status.error = PE_InvalidCharacter;
        _status.position = index + 1;
        if (!tokens.empty()) _status.token = tokens.back();
        return false;
      }

      ++index;
    }
  }

  return setTokens(tokens);
}

std::string TreeParser::expression() const
{
  string result;
  if (_status.error == PE_None)
  {
    TokenArray tokens = _root->tokensArray();
    for (ConstTokenIterator token = tokens.begin(); token != tokens.end(); ++token)
    {
      if (token != tokens.begin())
        result += ' ';

      if ((*token).type() == TT_Number)
      {
        stringstream stream;
        if (_numberFormat == NF_Fixed)
          stream << fixed;
        else if (_numberFormat == NF_Scientific)
          stream << scientific;

        stream << setprecision(_numberPrecision);

        stream << (*token).number();
        result += stream.str();
      }
      else
        result += (*token).name();
    }
  }
  return result;
}

bool TreeParser::reparseExpression()
{
  if (_originalExpression.empty()) return false;
  return setExpression(_originalExpression);
}

bool TreeParser::setTokens(const TokenArray &tokens)
{
  reset();

  if (tokens.empty())
  {
    _status.error = PE_EmptyExpression;
    return false;
  }

  int bracketLevel = 0;
  for (ConstTokenIterator token = tokens.begin(); token != tokens.end(); ++token)
  {
    if ((*token).type() == TT_LeftBracket)
      ++bracketLevel;
    else if ((*token).type() == TT_RightBracket)
      --bracketLevel;
  }
  if (bracketLevel != 0)
  {
    _status.error = PE_MismatchedBrackets;
    return false;
  }

  _root->tokens = tokens;
  _status = _root->divide();
  _root->brackets = false;

  if (_status.error != PE_None)
    return false;

  _status = _root->check();

  if (_root->tokens.front().type() == TT_Comma)
    _status.error = PE_MisplacedComma;

  if (_status.error != PE_None)
    return false;

  substitute();

  return true;
}

TokenArray TreeParser::tokens() const
{
  TokenArray result;
  if (_status.error == PE_None)
    result = _root->tokensArray();

  return result;
}

std::vector<std::string> TreeParser::variablesInExpression() const
{
  vector<string> result;
  _root->listTokenNames(result, TT_Variable);
  return result;
}

std::vector<std::string> TreeParser::externalFunctionsInExpression() const
{
  vector<string> result;
  _root->listTokenNames(result, TT_ExternalFunction);
  return result;
}

/* static */ bool TreeParser::setConstant(const std::string &pName, NumType value)
{
  if (pName.empty()) return false;

  string name;
  for (unsigned int i = 0; i < pName.size(); ++i)
  {
    char ch = tolower(pName.at(i));
    if (((ch >= 'a') || (ch <= 'z')) || ((ch == '_') && (i != 0)))
      name += ch;
    else
      return false;
  }

  _constants.insert(make_pair<std::string, NumType>(name, value));
  return true;
}

/* static */ bool TreeParser::unsetConstant(const std::string &pName)
{
  if (pName.empty()) return false;

  string name;
  for (unsigned int i = 0; i < pName.size(); ++i)
  {
    char ch = tolower(pName.at(i));
    if (((ch >= 'a') || (ch <= 'z')) || ((ch == '_') && (i != 0)))
      name += ch;
    else
      return false;
  }

  ValueMapIterator it = _constants.find(name);
  if (it == _constants.end())
    return false;
  else
    _constants.erase(it);

  return true;
}

/* static */ NumType TreeParser::constant(const std::string &name, bool *exists)
{
  NumType result = 0.0;
  map<std::string, NumType>::iterator var = _constants.find(name);
  if (var != _constants.end())
  {
    result = (*var).second;
    if (exists != NULL) *exists = true;
  }
  else if (exists != NULL) *exists = false;

  return result;
}

bool TreeParser::setVariable(const std::string &pName, NumType *value, bool replace)
{
  if (pName.empty())
  {
    return false;
  }

  string name;
  for (unsigned int i = 0; i < pName.size(); ++i)
  {
    char ch = tolower(pName.at(i));
    if (((ch >= 'a') || (ch <= 'z')) || ((ch == '_') && (i != 0)))
      name += ch;
    else
      return false;
  }

  PtrValueMapIterator it = _variables.find(name);
  if (it == _variables.end())
    _variables.insert(make_pair<std::string, NumType*>(name, value));
  else
  {
    if (replace)
      it->second = value;
    else
      return false;
  }

  return true;
}

bool TreeParser::unsetVariable(const std::string &pName)
{
  if (pName.empty()) return false;

  string name;
  for (unsigned int i = 0; i < pName.size(); ++i)
  {
    char ch = tolower(pName.at(i));
    if (((ch >= 'a') || (ch <= 'z')) || ((ch == '_') && (i != 0)))
      name += ch;
    else
      return false;
  }

  PtrValueMapIterator it = _variables.find(name);
  if (it == _variables.end())
    return false;
  else
    _variables.erase(it);

  return true;
}

NumType* TreeParser::variable(const std::string &name)
{
  NumType *result = NULL;
  PtrValueMapIterator var = _variables.find(name);
  if (var != _variables.end())
    result = (*var).second;

  return result;
}

void TreeParser::print(std::ostream &out) const
{
  out << "digraph G {" << endl;
  _root->print(out);
  out << "}" << endl;
}

void TreeParser::substitute()
{
  if (_status.error == PE_None)
    _root->substitute(_constants);
}

ComputeResult TreeParser::computeExpressionStep()
{
  ComputeResult result;
  if (_status.error == PE_None)
    result = _root->computeExpression(_variables, true);
  else
    result.mathError = ME_InvalidExpression;

  return result;
}

ComputeResult TreeParser::computeExpression()
{
  ComputeResult result;
  if (_status.error == PE_None)
    result = _root->computeExpression(_variables, false);
  else
    result.mathError = ME_InvalidExpression;

  return result;
}

ComputeResult TreeParser::computeValue(NumType &value) const
{
  ComputeResult result;
  if (_status.error == PE_None)
    result = _root->computeValue(value, _variables);
  else
    result.mathError = ME_InvalidExpression;

  return result;
}

// Default values of static variables
NumberFormat TreeParser::_numberFormat = NF_Auto;
int TreeParser::_numberPrecision = 6;
ValueMap TreeParser::_constants = ValueMap();
bool (*TreeParser::_isFunction)(const std::string&) = NULL;
bool (*TreeParser::_getFunctionValue)(const std::string&, NumType, NumType&) = NULL;
