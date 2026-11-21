#pragma once

#include <cstdint>
#include <string>

#include "Date.h"

enum class ClientType {
    Individual,
    Business
};

class Client {
private:
    int id;
    std::string name;
    Date birthDate;
    std::string phoneNumber;
    std::string accountNumber;
    ClientType clientType;
    bool vip;
    std::int64_t balanceInKopecks;
    Date openingDate;

public:
    Client(int id, const std::string& name, const Date& birthDate, const std::string& phoneNumber,
           const std::string& accountNumber, ClientType clientType, bool vip,
           std::int64_t balanceInKopecks, const Date& openingDate);

    int getId() const;
    const std::string& getName() const;
    Date getBirthDate() const;
    const std::string& getPhoneNumber() const;
    const std::string& getAccountNumber() const;
    ClientType getClientType() const;
    bool isVip() const;
    std::int64_t getBalanceInKopecks() const;
    Date getOpeningDate() const;

    void setName(const std::string& name);
    void setBirthDate(const Date& birthDate);
    void setPhoneNumber(const std::string& phoneNumber);
    void setAccountNumber(const std::string& accountNumber);
    void setClientType(ClientType clientType);
    void setVip(bool vip);
    void setBalanceInKopecks(std::int64_t balanceInKopecks);
    void setOpeningDate(const Date& openingDate);
};
