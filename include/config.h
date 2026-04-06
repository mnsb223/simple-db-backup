#pragma once
#include <string>
#include <map>

struct Config {
    // [database]
    std::string host;
    std::string port;
    std::string user;
    std::string password;
    std::string dbName;     // default db to operate on

    // [backup]
    std::string pgBin;      // directory containing pg_dump/pg_restore/etc.
    std::string outputDir;  // where backups are written
};

// Load a simple INI file. Returns false if the file can't be opened.
bool loadConfig(const std::string& path, Config& out);

// Build a libpq "key=value key=value ..." connection string.
// If overrideDb is non-empty, it replaces cfg.dbName.
std::string buildConnInfo(const Config& cfg, const std::string& overrideDb = "");
