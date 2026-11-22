#pragma once

#include <filesystem>

class ClientDatabase;

namespace ClientFile {

void save(const ClientDatabase& database, const std::filesystem::path& path);
void load(ClientDatabase& database, const std::filesystem::path& path);

}
