#include "config.h"
#include <fstream>
#include <map>

bool loadConfig(const std::string& path, Config& out)
{
    std::ifstream f(path);
    if (!f.is_open()) return false;

    std::map<std::string, std::map<std::string, std::string>> ini;
    std::string line, section;

    while (std::getline(f, line)) {
        if (line.empty() || line[0] == ';' || line[0] == '#') continue;
        // strip trailing \r for CRLF files
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        if (line.front() == '[' && line.back() == ']') {
            section = line.substr(1, line.size() - 2);
            continue;
        }
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        ini[section][line.substr(0, eq)] = line.substr(eq + 1);
    }

    auto& db = ini["database"];
    out.host     = db["host"];
    out.port     = db["port"];
    out.user     = db["user"];
    out.password = db["password"];
    out.dbName   = db["dbname"];

    auto& bk = ini["backup"];
    out.pgBin     = bk["pg_bin"];
    out.outputDir = bk["output_dir"];

    return true;
}

std::string buildConnInfo(const Config& cfg, const std::string& overrideDb)
{
    const std::string& db = overrideDb.empty() ? cfg.dbName : overrideDb;
    return "host="     + cfg.host     + " " +
           "port="     + cfg.port     + " " +
           "dbname="   + db           + " " +
           "user="     + cfg.user     + " " +
           "password=" + cfg.password;
}
