#include <iostream>

#include "Client.h"
#include "ClientDatabase.h"
#include "Vector.h"

namespace {

void printClients(const Vector<Client>& clients) {
    for (auto it = clients.begin(); it != clients.end(); ++it) {
        std::cout << it->getId() << "  " << it->getName() << '\n';
    }
    std::cout << "Size: " << clients.size() << ", capacity: " << clients.capacity() << "\n\n";
}

bool byId(const Client& first, const Client& second) {
    return first.getId() < second.getId();
}

}

int main() {
    Vector<Client> clients;
    const Date openingDate(10, 9, 2024);

    clients.push_back(Client(30, "Anna Petrova", Date(8, 4, 1995), "555-0103", "0003",
                             ClientType::Individual, false, 125050, openingDate));
    clients.push_back(Client(10, "Ivan Sokolov", Date(12, 1, 1988), "555-0101", "0001",
                             ClientType::Business, false, 820000, openingDate));
    clients.push_back(Client(20, "Maria Volkova", Date(27, 6, 1999), "555-0102", "0002",
                             ClientType::Individual, true, 560025, openingDate));

    std::cout << "Insertion order\n";
    printClients(clients);

    clients.sort(byId);
    std::cout << "Sorted by ID\n";
    printClients(clients);

    clients.erase(1);
    std::cout << "After removing the second element\n";
    printClients(clients);

    ClientDatabase database;
    std::cout << std::boolalpha;
    std::cout << "First insertion accepted: " << database.add(clients[0]) << '\n';
    std::cout << "Duplicate insertion accepted: " << database.add(clients[0]) << '\n';

    return 0;
}
