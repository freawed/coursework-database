#include "Date.h"

#include <iomanip>
#include <locale>
#include <sstream>
#include <stdexcept>

namespace {

int parseDigits(const std::string& text, std::size_t start, std::size_t count) {
    int value = 0;
    for (std::size_t i = start; i < start + count; ++i) {
        const char digit = text[i];
        if (digit < '0' || digit > '9') {
            return -1;
        }
        value = value * 10 + (digit - '0');
    }
    return value;
}

}

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

bool Date::isLeapYear(int y) {
    return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
}

int Date::daysInMonth(int m, int y) {
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (m == 2 && isLeapYear(y)) return 29;
    return days[m - 1];
}

bool Date::isValid(int d, int m, int y) {
    if (y < 1 || y > 9999) return false;
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

bool Date::operator<(const Date& other) const {
    if (this->year != other.year) return this->year < other.year;
    if (this->month != other.month) return this->month < other.month;
    return this->day < other.day;
}

bool Date::operator==(const Date& other) const {
    return this->year == other.year && this->month == other.month && this->day == other.day;
}

std::string Date::toString() const {
    std::ostringstream ss;
    ss.imbue(std::locale::classic());
    ss << std::setfill('0')
       << std::setw(2) << day << "."
       << std::setw(2) << month << "."
       << std::setw(4) << year;
    return ss.str();
}

bool Date::setFromString(const std::string& text) {
    if (text.size() != 10 || text[2] != '.' || text[5] != '.') {
        return false;
    }
    return setDate(parseDigits(text, 0, 2), parseDigits(text, 3, 2), parseDigits(text, 6, 4));
}
