#include "Client.h"

Client::Client(int id, const std::string& name, const Date& birthDate, const std::string& phoneNumber, const std::string& accountNumber, ClientType clientType, double balance, const Date& openingDate)
    : id(id), name(name), birthDate(birthDate), phoneNumber(phoneNumber), accountNumber(accountNumber), clientType(clientType), balance(balance), openingDate(openingDate) 
{
}

int Client::getId() const {
    return id;
}

std::string Client::getName() const {
    return name;
}

Date Client::getBirthDate() const {
    return birthDate;
}

std::string Client::getPhoneNumber() const {
    return phoneNumber;
}

std::string Client::getAccountNumber() const {
    return accountNumber;
}

ClientType Client::getClientType() const {
    return clientType;
}

double Client::getBalance() const {
    return balance;
}

Date Client::getOpeningDate() const {
    return openingDate;
}

void Client::setName(const std::string& newName) {
    name = newName;
}

void Client::setBirthDate(const Date& newBirthDate) {
    birthDate = newBirthDate;
}

void Client::setPhoneNumber(const std::string& newPhoneNumber) {
    phoneNumber = newPhoneNumber;
}

void Client::setAccountNumber(const std::string& newAccountNumber) {
    accountNumber = newAccountNumber;
}

void Client::setClientType(ClientType newClientType) {
    clientType = newClientType;
}

void Client::setBalance(double newBalance) {
    balance = newBalance;
}

void Client::setOpeningDate(const Date& newOpeningDate) {
    openingDate = newOpeningDate;
}


