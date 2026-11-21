#include "ClientDatabase.h"

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

std::size_t ClientDatabase::size() const {
    return clients.size();
}

const Client& ClientDatabase::at(std::size_t index) const {
    return clients.at(index);
}
