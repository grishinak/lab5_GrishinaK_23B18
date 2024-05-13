#ifndef PARSER_H
#define PARSER_H

#include <cmath>
#include <limits>
#include <string>

// Предварительные объявления функций парсера
double parse_expression(const std::string& expression, int& i);
double parse_term(const std::string& expression, int& i);
double parse_factor(const std::string& expression, int& i);

// Функция для вычисления математического выражения
double evaluate_expression(const std::string& expression) {
  // Реализация простого парсера
  int i = 0;
  return parse_expression(expression, i);
}

// Функция для разбора выражения
double parse_expression(const std::string& expression, int& i) {
  double result = parse_term(expression, i);

  while (i < expression.length()) {
    if (expression[i] == '+') {
      i++;
      result += parse_term(expression, i);
    } else if (expression[i] == '-') {
      i++;
      result -= parse_term(expression, i);
    } else {
      break;
    }
  }

  return result;
}

// Функция для разбора терма
double parse_term(const std::string& expression, int& i) {
  double result = parse_factor(expression, i);

  while (i < expression.length()) {
    if (expression[i] == '*') {
      i++;
      result *= parse_factor(expression, i);
    } else if (expression[i] == '/') {
      i++;
      double denominator = parse_factor(expression, i);
      if (denominator != 0.0) {
        result /= denominator;
      } else {
        std::cerr << "Error: Division by zero" << std::endl;
        return std::numeric_limits<double>::quiet_NaN();
      }
    } else {
      break;
    }
  }

  return result;
}

// Функция для разбора фактора
double parse_factor(const std::string& expression, int& i) {
  double result = 0.0;

  if (i < expression.length()) {
    if (expression[i] == '(') {
      i++;
      result = parse_expression(expression, i);
      if (i < expression.length() && expression[i] == ')') {
        i++;
      } else {
        std::cerr << "Error: Unbalanced parentheses" << std::endl;
        return std::numeric_limits<double>::quiet_NaN();
      }
    } else if (isdigit(expression[i]) || expression[i] == '-' ||
               expression[i] == '+') {
      size_t j;
      result = std::stod(expression.substr(i), &j);
      i += j;
    } else if (std::isalpha(expression[i])) {
      size_t func_end = expression.find_first_of("()", i);
      std::string func_name = expression.substr(i, func_end - i);
      i = func_end;

      if (func_name == "sin") {
        // Обработка функции синуса
        if (expression[i] != '(') {
          std::cerr << "Error: Expected '(' after 'sin'" << std::endl;
          return std::numeric_limits<double>::quiet_NaN();
        }
        i++;
        double arg = parse_expression(expression, i);
        if (i < expression.length() && expression[i] == ')') {
          i++;
          result = sin(arg);
        } else {
          std::cerr << "Error: Unbalanced parentheses after 'sin' argument"
                    << std::endl;
          return std::numeric_limits<double>::quiet_NaN();
        }
      } else if (func_name == "sqrt") {
        // Обработка функции квадратного корня
        if (expression[i] != '(') {
          std::cerr << "Error: Expected '(' after 'sqrt'" << std::endl;
          return std::numeric_limits<double>::quiet_NaN();
        }
        i++;
        double arg = parse_expression(expression, i);
        if (arg < 0) {
          std::cerr << "Error: Square root of a negative number" << std::endl;
          return std::numeric_limits<double>::quiet_NaN();
        }
        if (i < expression.length() && expression[i] == ')') {
          i++;
          result = sqrt(arg);
        } else {
          std::cerr << "Error: Unbalanced parentheses after 'sqrt' argument"
                    << std::endl;
          return std::numeric_limits<double>::quiet_NaN();
        }
      } else if (func_name == "log") {
        // Обработка функции логарифма
        if (expression[i] != '(') {
          std::cerr << "Error: Expected '(' after 'log'" << std::endl;
          return std::numeric_limits<double>::quiet_NaN();
        }
        i++;
        double arg = parse_expression(expression, i);
        if (arg <= 0) {
          std::cerr << "Error: Logarithm of a non-positive number" << std::endl;
          return std::numeric_limits<double>::quiet_NaN();
        }
        if (i < expression.length() && expression[i] == ')') {
          i++;
          result = log(arg);
        } else {
          std::cerr << "Error: Unbalanced parentheses after 'log' argument"
                    << std::endl;
          return std::numeric_limits<double>::quiet_NaN();
        }
      } else {
        std::cerr << "Error: Unknown function '" << func_name << "'"
                  << std::endl;
        return std::numeric_limits<double>::quiet_NaN();
      }
    } else {
      std::cerr << "Error: Invalid expression" << std::endl;
      return std::numeric_limits<double>::quiet_NaN();
    }
  }

  return result;
}

#endif  //PARSER_H
