#pragma once

#include <string>

class Date {
private:
    int day;
    int month;
    int year;

    static bool isLeapYear(int y);
    static int daysInMonth(int m, int y);
public:
    Date();
    Date(int d, int m, int y);

    int getDay() const;
    int getMonth() const;
    int getYear() const;

    bool setDay(int d);
    bool setMonth(int m);
    bool setYear(int y);
    bool setDate(int d, int m, int y);

    static bool isValid(int d, int m, int y);

    bool operator<(const Date& other) const;
    bool operator==(const Date& other) const;
    std::string toString() const;
    bool setFromString(const std::string& text);
};
