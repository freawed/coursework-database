#include "ClientFile.h"

#include <array>
#include <charconv>
#include <fstream>
#include <locale>
#include <stdexcept>
#include <system_error>

#include "ClientDatabase.h"

namespace {

namespace fs = std::filesystem;

constexpr char legacyFileHeader[] = "CLIENTS\t1";
constexpr char fileHeader[] = "CLIENTS\t2";

struct TemporaryOutput {
    fs::path directory;
    fs::path file;

    explicit TemporaryOutput(const fs::path& destination) {
        for (int attempt = 0; attempt < 100; ++attempt) {
            directory = destination;
            directory += ".tmp." + std::to_string(attempt);
            file = directory / "data";

            std::error_code error;
            if (fs::create_directory(directory, error)) {
                return;
            }
            if (error && error != std::errc::file_exists) {
                throw fs::filesystem_error("Cannot create a temporary directory", directory, error);
            }
        }
        throw std::runtime_error("No free temporary directory for saving");
    }

    TemporaryOutput(const TemporaryOutput&) = delete;
    TemporaryOutput& operator=(const TemporaryOutput&) = delete;

    ~TemporaryOutput() {
        std::error_code ignored;
        fs::remove(file, ignored);
        fs::remove(directory, ignored);
    }
};

void removeTrailingCarriageReturn(std::string& line) {
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
}

template <typename Integer>
bool parseInteger(const std::string& text, Integer& value) {
    const char* end = text.data() + text.size();
    const auto result = std::from_chars(text.data(), end, value);
    return result.ec == std::errc{} && result.ptr == end;
}

Client parseClient(const std::string& line, bool hasOtherBankAccountsField) {
    std::array<std::string, 10> fields{};
    const std::size_t fieldCount = hasOtherBankAccountsField ? 10 : 9;
    const char* fieldCountError = hasOtherBankAccountsField
                                      ? "Expected ten fields" : "Expected nine fields";

    std::size_t start = 0;
    for (std::size_t i = 0; i + 1 < fieldCount; ++i) {
        const std::size_t separator = line.find('\t', start);
        if (separator == std::string::npos) {
            throw std::invalid_argument(fieldCountError);
        }
        fields[i] = line.substr(start, separator - start);
        start = separator + 1;
    }

    if (line.find('\t', start) != std::string::npos) {
        throw std::invalid_argument(fieldCountError);
    }
    if (line.find('\r') != std::string::npos) {
        throw std::invalid_argument("Unexpected carriage return");
    }
    fields[fieldCount - 1] = line.substr(start);

    const auto& [idText, name, birthText, phone, account, type, vip,
                 balanceText, openingText, otherBankAccounts] = fields;

    int id;
    std::int64_t balance;
    Date birth;
    Date opening;
    if (!parseInteger(idText, id) || !parseInteger(balanceText, balance)) {
        throw std::invalid_argument("Invalid ID or balance");
    }
    if (!birth.setFromString(birthText) || !opening.setFromString(openingText)) {
        throw std::invalid_argument("Invalid date");
    }
    if (type != "Individual" && type != "Business") {
        throw std::invalid_argument("Unknown client type");
    }
    if (vip != "0" && vip != "1") {
        throw std::invalid_argument("VIP status must be 0 or 1");
    }

    if (hasOtherBankAccountsField && otherBankAccounts != "0" &&
        otherBankAccounts != "1") {
        throw std::invalid_argument("Other bank accounts status must be 0 or 1");
    }

    const ClientType clientType = type == "Individual" ? ClientType::Individual : ClientType::Business;
    return Client(id, name, birth, phone, account, clientType,
                  vip == "1", balance, opening, otherBankAccounts == "1");
}

void writeClient(std::ostream& output, const Client& client) {
    if (client.getName().find_first_of("\t\r\n") != std::string::npos ||
        client.getPhoneNumber().find_first_of("\t\r\n") != std::string::npos) {
        throw std::runtime_error("Name and phone must not contain tabs or line breaks");
    }

    const ClientType type = client.getClientType();
    if (type != ClientType::Individual && type != ClientType::Business) {
        throw std::runtime_error("Unknown client type");
    }

    output << client.getId() << '\t'
           << client.getName() << '\t'
           << client.getBirthDate().toString() << '\t'
           << client.getPhoneNumber() << '\t'
           << client.getAccountNumber() << '\t'
           << (type == ClientType::Individual ? "Individual" : "Business") << '\t'
           << (client.isVip() ? '1' : '0') << '\t'
           << client.getBalanceInKopecks() << '\t'
           << client.getOpeningDate().toString() << '\t'
           << (client.hasOtherBankAccounts() ? '1' : '0') << '\n';
}

}

void ClientFile::save(const ClientDatabase& database, const fs::path& path) {
    if (!path.has_filename()) {
        throw std::runtime_error("Select a file for saving");
    }

    const TemporaryOutput temporary(path);
    std::ofstream output(temporary.file, std::ios::binary);
    if (!output) {
        throw std::runtime_error("Cannot open a temporary file for writing");
    }
    output.imbue(std::locale::classic());
    output << fileHeader << '\n';
    for (const Client& client : database) {
        writeClient(output, client);
    }
    output.close();
    if (!output) {
        throw std::runtime_error("Cannot write the database file");
    }

    fs::rename(temporary.file, path);
}

void ClientFile::load(ClientDatabase& database, const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Cannot open the database file");
    }

    std::string line;
    std::getline(input, line);
    removeTrailingCarriageReturn(line);
    if (line.compare(0, 3, "\xEF\xBB\xBF") == 0) {
        line.erase(0, 3);
    }
    const bool hasOtherBankAccountsField = line == fileHeader;

    if (!hasOtherBankAccountsField && line != legacyFileHeader) {
        throw std::runtime_error("Invalid database file header or version");
    }

    ClientDatabase loaded;
    std::size_t lineNumber = 1;
    while (std::getline(input, line)) {
        ++lineNumber;
        removeTrailingCarriageReturn(line);
        try {
            if (!loaded.add(parseClient(line, hasOtherBankAccountsField))) {
                throw std::invalid_argument("Duplicate ID or account number");
            }
        } catch (const std::invalid_argument& error) {
            throw std::runtime_error("Line " + std::to_string(lineNumber) + ": " + error.what());
        }
    }
    if (input.bad() || !input.eof()) {
        throw std::runtime_error("Cannot read the database file");
    }

    database.swap(loaded);
}
