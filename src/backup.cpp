#include "backup.h"
#include <cstdlib>
#include <ctime>
#include <iostream>

#ifdef _WIN32
  #include <stdlib.h>
  static void setPgPassword(const std::string& pw) { _putenv_s("PGPASSWORD", pw.c_str()); }
#else
  #include <stdlib.h>
  static void setPgPassword(const std::string& pw) { setenv("PGPASSWORD", pw.c_str(), 1); }
#endif

static std::string pgTool(const Config& cfg, const std::string& name)
{
    std::string path = cfg.pgBin;
    if (!path.empty() && path.back() != '/' && path.back() != '\\') path += '/';
    path += name;
#ifdef _WIN32
    path += ".exe";
#endif
    // Quote the whole path so spaces (e.g. "Program Files") work with std::system/cmd.exe
    return "\"" + path + "\"";
}

static std::string q(const std::string& s) { return "\"" + s + "\""; }

int backupDatabase(const Config& cfg, const std::string& dbName, const std::string& outFile)
{
    setPgPassword(cfg.password);

    std::string cmd = pgTool(cfg, "pg_dump") +
        " -h " + cfg.host +
        " -p " + cfg.port +
        " -U " + cfg.user +
        " -F c" +                  // custom format (compressed)
        " -f " + q(outFile) +
        " "    + dbName;

    // std::system on Windows wraps the whole command in cmd.exe /C "...",
    // and the outer quoting around a quoted program path requires the entire
    // command to be wrapped in an extra pair of quotes.
#ifdef _WIN32
    cmd = "\"" + cmd + "\"";
#endif

    int rc = std::system(cmd.c_str());
    if (rc != 0) std::cerr << "pg_dump failed (code " << rc << ")\n";
    else         std::cout << "Backup written: " << outFile << '\n';
    return rc;
}

int backupAll(const Config& cfg, const std::string& outFile)
{
    setPgPassword(cfg.password);

    std::string cmd = pgTool(cfg, "pg_dumpall") +
        " -h " + cfg.host +
        " -p " + cfg.port +
        " -U " + cfg.user +
        " -f " + q(outFile);

#ifdef _WIN32
    cmd = "\"" + cmd + "\"";
#endif

    int rc = std::system(cmd.c_str());
    if (rc != 0) std::cerr << "pg_dumpall failed (code " << rc << ")\n";
    else         std::cout << "Full cluster backup written: " << outFile << '\n';
    return rc;
}

int restoreDatabase(const Config& cfg, const std::string& dbName, const std::string& inFile)
{
    setPgPassword(cfg.password);

    std::string cmd = pgTool(cfg, "pg_restore") +
        " -h " + cfg.host +
        " -p " + cfg.port +
        " -U " + cfg.user +
        " -d " + dbName +
        " --clean --if-exists " +
        q(inFile);

#ifdef _WIN32
    cmd = "\"" + cmd + "\"";
#endif

    int rc = std::system(cmd.c_str());
    if (rc != 0) std::cerr << "pg_restore failed (code " << rc << ")\n";
    else         std::cout << "Restore complete from: " << inFile << '\n';
    return rc;
}

int rotationSlot(int totalHours)
{
    std::time_t now = std::time(nullptr);
    long long hours = static_cast<long long>(now) / 3600;
    int slot = static_cast<int>(hours % totalHours);
    if (slot < 0) slot += totalHours;
    return slot;
}

std::string rotatedBackupPath(const Config& cfg, const std::string& dbName, int slot)
{
    std::string dir = cfg.outputDir;
    if (!dir.empty() && dir.back() != '/' && dir.back() != '\\') dir += '/';
    return dir + "backup_" + dbName + "_" + std::to_string(slot) + ".dump";
}
