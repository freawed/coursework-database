#include "ClientDatabase.h"

#include <limits>
#include <utility>

bool ClientDatabase::add(const Client& client) {
    for (const Client& stored : clients) {
        if (stored.getId() == client.getId() || stored.getAccountNumber() == client.getAccountNumber()) {
            return false;
        }
    }

    clients.push_back(client);
    return true;
}

bool ClientDatabase::update(const Client& client) {
    Client* existing = nullptr;
    for (Client& stored : clients) {
        if (stored.getId() == client.getId()) {
            existing = &stored;
        } else if (stored.getAccountNumber() == client.getAccountNumber()) {
            return false;
        }
    }

    if (existing == nullptr) {
        return false;
    }

    Client replacement(client);
    *existing = std::move(replacement);
    return true;
}

bool ClientDatabase::remove(int id) {
    const std::size_t index = indexOfId(id);
    if (index == size()) {
        return false;
    }
    clients.erase(index);
    return true;
}

bool ClientDatabase::merge(const ClientDatabase& other) {
    if (other.clients.empty()) {
        return true;
    }

    int lastId = 0;
    for (const Client& client : clients) {
        if (client.getId() > lastId) {
            lastId = client.getId();
        }
    }

    ClientDatabase merged(*this);
    for (const Client& client : other) {
        if (lastId == std::numeric_limits<int>::max()) {
            return false;
        }
        ++lastId;
        if (!merged.add(Client(lastId, client.getName(), client.getBirthDate(),
                               client.getPhoneNumber(), client.getAccountNumber(),
                               client.getClientType(), client.isVip(),
                               client.getBalanceInKopecks(), client.getOpeningDate(),
                               client.hasOtherBankAccounts()))) {
            return false;
        }
    }

    swap(merged);
    return true;
}

void ClientDatabase::clear() {
    clients.clear();
}

void ClientDatabase::swap(ClientDatabase& other) noexcept {
    clients.swap(other.clients);
}

std::size_t ClientDatabase::size() const {
    return clients.size();
}

const Client& ClientDatabase::at(std::size_t index) const {
    return clients.at(index);
}

Vector<Client>::ConstIterator ClientDatabase::begin() const {
    return clients.begin();
}

Vector<Client>::ConstIterator ClientDatabase::end() const {
    return clients.end();
}

const Client* ClientDatabase::findById(int id) const {
    const std::size_t index = indexOfId(id);
    return index == size() ? nullptr : &clients[index];
}

const Client* ClientDatabase::findByAccountNumber(const std::string& accountNumber) const {
    for (const Client& client : clients) {
        if (client.getAccountNumber() == accountNumber) {
            return &client;
        }
    }
    return nullptr;
}

Vector<Client> ClientDatabase::findByName(const std::string& partOfName) const {
    Vector<Client> matches;
    for (const Client& client : clients) {
        if (client.getName().find(partOfName) != std::string::npos) {
            matches.push_back(client);
        }
    }
    return matches;
}

std::size_t ClientDatabase::indexOfId(int id) const {
    for (std::size_t i = 0; i < size(); ++i) {
        if (clients[i].getId() == id) {
            return i;
        }
    }
    return size();
}
