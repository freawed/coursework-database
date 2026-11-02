#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <stdexcept>
#include "Date.h"

Date::Date()
    : day(1), month(1), year(2000)
{
}

Date::Date(int day, int month, int year)
    : day(day), month(month), year(year)
{
    if (!isValid(day, month, year)) {
        throw std::invalid_argument("Invalid date");
    }
}

Date::~Date()
{
}

bool Date::isLeapYear(int y) {
    return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
}

int Date::daysInMonth(int m, int y) {
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (m == 2 && isLeapYear(y)) return 29;
    return days[m - 1];
}

bool Date::isValid(int d, int m, int y) {
    if (y < 1) return false;
    if (m < 1 || m > 12) return false;
    return d >= 1 && d <= daysInMonth(m, y);
}

int Date::getDay() const {
    return day;
}

int Date::getMonth() const {
    return month;
}

int Date::getYear() const {
    return year;
}

bool Date::setDay(int d) {
    return setDate(d, month, year);
}

bool Date::setMonth(int m) {
    return setDate(day, m, year);
}

bool Date::setYear(int y) {
    return setDate(day, month, y);
}

bool Date::setDate(int d, int m, int y) {
    if (!isValid(d, m, y)) return false;
    day = d;
    month = m;
    year = y;
    return true;
}

void Date::printDate() const {
    std::cout << DateToString() << std::endl;
}

bool Date::operator<(const Date& other) const {
    if (this->year != other.year) return this->year < other.year;
    if (this->month != other.month) return this->month < other.month;
    return this->day < other.day;
}

bool Date::operator==(const Date& other) const {
    return this->year == other.year && this->month == other.month && this->day == other.day;
}

std::string Date::DateToString() const {
    std::stringstream ss;
    ss << std::setfill('0')
       << std::setw(2) << day << "."
       << std::setw(2) << month << "."
       << std::setw(4) << year;
    return ss.str();
}

bool Date::StringToDate(const std::string& str) {
    std::stringstream ss(str);
    int d, m, y;
    char ch1, ch2;
    if (ss >> d >> ch1 >> m >> ch2 >> y) {
        if (ch1 == '.' && ch2 == '.' && ss.eof()) {
            return setDate(d, m, y);
        }
    }
    return false;
}
