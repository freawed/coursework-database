#include <string>
#include "Date.h"

enum class ClientType { 
    Individual, 
    Business, 
    VIP 
};

class Client {
private:
    int id;
    std::string name;
    Date birthDate;
    std::string phoneNumber;
    std::string accountNumber;
    ClientType clientType;
    double balance;
    Date openingDate;

public: 
    Client(int id, const std::string& name, const Date& birthDate, const std::string& phoneNumber,
           const std::string& accountNumber, ClientType clientType, double balance, const Date& openingDate);

    int getId() const;
    std::string getName() const;
    Date getBirthDate() const;
    std::string getPhoneNumber() const;
    std::string getAccountNumber() const;
    ClientType getClientType() const;
    double getBalance() const;
    Date getOpeningDate() const;

    void setName(const std::string& name);
    void setBirthDate(const Date& birthDate);
    void setPhoneNumber(const std::string& phoneNumber);
    void setAccountNumber(const std::string& accountNumber);
    void setClientType(ClientType clientType);
    void setBalance(double balance);
    void setOpeningDate(const Date& openingDate);
};