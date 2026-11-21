#pragma once

#include <cstddef>

#include "Client.h"
#include "Vector.h"

class ClientDatabase {
public:
    bool add(const Client& client);
    bool update(const Client& client);
    std::size_t size() const;
    const Client& at(std::size_t index) const;

private:
    Vector<Client> clients;
};
