#include "Client.h"

#include <stdexcept>

namespace {

void validateName(const std::string& name) {
    if (name.find_first_not_of(" \t\n\r\f\v") == std::string::npos) {
        throw std::invalid_argument("Client name must not be blank");
    }
}

void validateAccountNumber(const std::string& accountNumber) {
    if (accountNumber.empty() || accountNumber.find_first_not_of("0123456789") != std::string::npos) {
        throw std::invalid_argument("Account number must contain only digits");
    }
}

void validateDates(const Date& birthDate, const Date& openingDate) {
    if (openingDate < birthDate) {
        throw std::invalid_argument("Opening date precedes birth date");
    }
}

}

Client::Client(int id, const std::string& name, const Date& birthDate, const std::string& phoneNumber,
               const std::string& accountNumber, ClientType clientType, bool vip,
               std::int64_t balanceInKopecks, const Date& openingDate, bool otherBankAccounts)
    : id(id),
    name(name),
    birthDate(birthDate),
    phoneNumber(phoneNumber),
    accountNumber(accountNumber),
    clientType(clientType),
    vip(vip),
    balanceInKopecks(balanceInKopecks),
    openingDate(openingDate),
    otherBankAccounts(otherBankAccounts) {
    validateName(name);
    validateAccountNumber(accountNumber);
    validateDates(birthDate, openingDate);
}

int Client::getId() const {
    return id;
}

const std::string& Client::getName() const {
    return name;
}

Date Client::getBirthDate() const {
    return birthDate;
}

const std::string& Client::getPhoneNumber() const {
    return phoneNumber;
}

const std::string& Client::getAccountNumber() const {
    return accountNumber;
}

ClientType Client::getClientType() const {
    return clientType;
}

bool Client::isVip() const {
    return vip;
}

std::int64_t Client::getBalanceInKopecks() const {
    return balanceInKopecks;
}

Date Client::getOpeningDate() const {
    return openingDate;
}

bool Client::hasOtherBankAccounts() const {
    return otherBankAccounts;
}

void Client::setName(const std::string& newName) {
    validateName(newName);
    name = newName;
}

void Client::setBirthDate(const Date& newBirthDate) {
    validateDates(newBirthDate, openingDate);
    birthDate = newBirthDate;
}

void Client::setPhoneNumber(const std::string& newPhoneNumber) {
    phoneNumber = newPhoneNumber;
}

void Client::setAccountNumber(const std::string& newAccountNumber) {
    validateAccountNumber(newAccountNumber);
    accountNumber = newAccountNumber;
}

void Client::setClientType(ClientType newClientType) {
    clientType = newClientType;
}

void Client::setVip(bool newVip) {
    vip = newVip;
}

void Client::setBalanceInKopecks(std::int64_t newBalanceInKopecks) {
    balanceInKopecks = newBalanceInKopecks;
}

void Client::setOpeningDate(const Date& newOpeningDate) {
    validateDates(birthDate, newOpeningDate);
    openingDate = newOpeningDate;
}
