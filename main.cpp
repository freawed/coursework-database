#include <charconv>
#include <iostream>
#include <stdexcept>

#include "ClientDatabase.h"
#include "ClientFile.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

struct InputClosed {};

#ifdef _WIN32
struct ConsoleEncoding {
    const UINT inputCodePage = GetConsoleCP();
    const UINT outputCodePage = GetConsoleOutputCP();

    ConsoleEncoding() {
        if (inputCodePage != 0) SetConsoleCP(CP_UTF8);
        if (outputCodePage != 0) SetConsoleOutputCP(CP_UTF8);
    }

    ~ConsoleEncoding() {
        if (inputCodePage != 0) SetConsoleCP(inputCodePage);
        if (outputCodePage != 0) SetConsoleOutputCP(outputCodePage);
    }
};
#endif

std::string readLine(const char* prompt) {
    std::cout << prompt << std::flush;
    std::string text;
    if (!std::getline(std::cin, text)) {
        throw InputClosed{};
    }
    return text;
}

template <typename Integer>
Integer readInteger(const char* prompt) {
    const std::string text = readLine(prompt);
    Integer value{};
    const char* end = text.data() + text.size();
    const auto result = std::from_chars(text.data(), end, value);
    if (result.ec != std::errc{} || result.ptr != end) {
        throw std::invalid_argument("Enter a whole number.");
    }
    return value;
}

Date readDate(const char* prompt) {
    Date date;
    if (!date.setFromString(readLine(prompt))) {
        throw std::invalid_argument("Enter a valid date in DD.MM.YYYY format.");
    }
    return date;
}

std::filesystem::path readPath() {
    const std::string path = readLine("File path: ");
    if (path.empty()) {
        throw std::invalid_argument("Enter a file path.");
    }
    return std::filesystem::u8path(path);
}

Client readClient(int id) {
    const std::string name = readLine("Full name: ");
    const Date birth = readDate("Birth date (DD.MM.YYYY): ");
    const std::string phone = readLine("Phone (optional): ");
    const std::string account = readLine("Account number: ");
    const int type = readInteger<int>("Client type (1 = individual, 2 = business): ");
    if (type != 1 && type != 2) {
        throw std::invalid_argument("Client type must be 1 or 2.");
    }
    const int vip = readInteger<int>("VIP (0 = no, 1 = yes): ");
    if (vip != 0 && vip != 1) {
        throw std::invalid_argument("VIP must be 0 or 1.");
    }
    const auto balance = readInteger<std::int64_t>("Balance in kopecks: ");
    const Date opening = readDate("Opening date (DD.MM.YYYY): ");
    return Client(id, name, birth, phone, account,
                  type == 1 ? ClientType::Individual : ClientType::Business, vip == 1, balance, opening);
}

void printClient(const Client& client) {
    std::cout << client.getId() << " | " << client.getName() << " | " << client.getBirthDate().toString()
              << " | " << client.getPhoneNumber() << " | " << client.getAccountNumber()
              << " | " << (client.getClientType() == ClientType::Individual ? "Individual" : "Business")
              << " | " << (client.isVip() ? "Yes" : "No") << " | " << client.getBalanceInKopecks()
              << " | " << client.getOpeningDate().toString() << '\n';
}

template <typename Clients>
void showClients(const Clients& clients) {
    std::cout << "Records: " << clients.size() << '\n';
    if (clients.size() == 0) {
        return;
    }
    std::cout << "ID | Name | Birth date | Phone | Account | Type | VIP | Balance (kopecks) | Opened\n";
    for (const Client& client : clients) {
        printClient(client);
    }
}

void editClient(ClientDatabase& database) {
    const int id = readInteger<int>("ID to edit: ");
    const Client* current = database.findById(id);
    if (current == nullptr) {
        std::cout << "ID not found.\n";
        return;
    }
    std::cout << "Current record (enter all fields for the replacement):\n";
    printClient(*current);
    const Client replacement = readClient(id);
    std::cout << (database.update(replacement) ? "Updated.\n" : "Account already exists.\n");
}

void sortClients(ClientDatabase& database) {
    switch (readInteger<int>("Sort by (1 = ID, 2 = name, 3 = balance): ")) {
    case 1:
        database.sort([](const Client& a, const Client& b) { return a.getId() < b.getId(); });
        break;
    case 2:
        database.sort([](const Client& a, const Client& b) { return a.getName() < b.getName(); });
        break;
    case 3:
        database.sort([](const Client& a, const Client& b) { return a.getBalanceInKopecks() < b.getBalanceInKopecks(); });
        break;
    default:
        throw std::invalid_argument("Unknown sort field.");
    }
    showClients(database);
}

void findClients(const ClientDatabase& database) {
    const int field = readInteger<int>("Find by (1 = name, 2 = account, 3 = ID): ");
    const Client* client = nullptr;
    switch (field) {
    case 1:
        showClients(database.findByName(readLine("Part of the name: ")));
        return;
    case 2:
        client = database.findByAccountNumber(readLine("Account number: "));
        break;
    case 3:
        client = database.findById(readInteger<int>("ID: "));
        break;
    default:
        throw std::invalid_argument("Unknown search field.");
    }

    if (client == nullptr) {
        std::cout << "No match.\n";
    } else {
        printClient(*client);
    }
}

void printMenu() {
    std::cout << "\n1  Show records\n"
                 "2  Add client\n"
                 "3  Delete client by ID\n"
                 "4  Sort and show records\n"
                 "5  Edit client by ID\n"
                 "6  Find clients\n"
                 "7  Save database to TSV\n"
                 "8  Merge with a TSV file\n"
                 "9  Clear database\n"
                 "0  Exit\n\n";
}

}

int main() {
#ifdef _WIN32
    const ConsoleEncoding encoding;
#endif
    ClientDatabase database;
    bool databaseReady = false;

    while (!databaseReady) {
        std::cout << "\n1  Open database from a TSV file\n"
                     "2  Create a database and enter records manually\n"
                     "0  Exit\n\n";
        try {
            switch (readInteger<int>("Mode: ")) {
            case 0:
                return 0;
            case 1:
                ClientFile::load(database, readPath());
                showClients(database);
                databaseReady = true;
                break;
            case 2:
                std::cout << "Empty database created. Use command 2 to add clients.\n";
                databaseReady = true;
                break;
            default:
                std::cout << "Unknown mode.\n";
                break;
            }
        } catch (const InputClosed&) {
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "Error: " << error.what() << '\n';
        }
    }

    printMenu();
    while (true) {
        try {
            switch (readInteger<int>("Command: ")) {
            case 0:
                return 0;
            case 1:
                showClients(database);
                break;
            case 2: {
                const int id = readInteger<int>("New ID: ");
                if (database.findById(id) != nullptr) {
                    std::cout << "ID already exists.\n";
                    break;
                }
                const Client client = readClient(id);
                std::cout << (database.add(client) ? "Added.\n" : "Account already exists.\n");
                break;
            }
            case 3: {
                const int id = readInteger<int>("ID to delete: ");
                std::cout << (database.remove(id) ? "Deleted.\n" : "ID not found.\n");
                break;
            }
            case 4:
                sortClients(database);
                break;
            case 5:
                editClient(database);
                break;
            case 6:
                findClients(database);
                break;
            case 7:
                ClientFile::save(database, readPath());
                std::cout << "Saved.\n";
                break;
            case 8: {
                ClientDatabase incoming;
                ClientFile::load(incoming, readPath());
                std::cout << (database.merge(incoming) ? "Merged.\n" : "Merge rejected: account conflict or no free ID.\n");
                break;
            }
            case 9:
                database.clear();
                std::cout << "Database cleared.\n";
                break;
            default:
                std::cout << "Unknown command.\n";
                break;
            }
        } catch (const InputClosed&) {
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "Error: " << error.what() << '\n';
        }
    }
}
