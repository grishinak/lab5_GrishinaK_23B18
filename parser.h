#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <cmath>
#include <limits>

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
        } else if (isdigit(expression[i]) || expression[i] == '-' || expression[i] == '+') {
            size_t j;
            result = std::stod(expression.substr(i), &j);
            i += j;
        } else {
            std::cerr << "Error: Invalid expression" << std::endl;
            return std::numeric_limits<double>::quiet_NaN();
        }
    }

    return result;
}

#endif
