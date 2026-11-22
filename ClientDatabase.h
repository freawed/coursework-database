#pragma once

#include <cstddef>
#include <string>

#include "Client.h"
#include "Vector.h"

class ClientDatabase {
public:
    bool add(const Client& client);
    bool update(const Client& client);
    bool remove(int id);
    bool merge(const ClientDatabase& other);
    void clear();
    void swap(ClientDatabase& other) noexcept;

    std::size_t size() const;
    const Client& at(std::size_t index) const;
    Vector<Client>::ConstIterator begin() const;
    Vector<Client>::ConstIterator end() const;

    const Client* findById(int id) const;
    const Client* findByAccountNumber(const std::string& accountNumber) const;
    Vector<Client> findByName(const std::string& partOfName) const;

    template <typename Compare>
    void sort(Compare comesBefore) {
        Vector<Client> sorted(clients);
        sorted.sort([&comesBefore](const Client& first, const Client& second) {
            return comesBefore(first, second);
        });
        clients.swap(sorted);
    }

private:
    std::size_t indexOfId(int id) const;
    Vector<Client> clients;
};
