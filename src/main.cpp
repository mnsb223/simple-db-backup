#include "config.h"
#include "crud.h"
#include "backup.h"

#include <iostream>
#include <string>
#include <vector>

#ifndef RESOURCES_PATH
#define RESOURCES_PATH "./"
#endif

static void printUsage()
{
    std::cout <<
        "simple_db_backup - Postgres CLI utility\n"
        "\n"
        "Usage:\n"
        "  simple_db_backup backup [<dbname>]             Backup one DB (custom format)\n"
        "  simple_db_backup backup-all                    Backup entire cluster (plain SQL)\n"
        "  simple_db_backup backup-rotate [<dbname>]      Backup into rotating 7-day slot\n"
        "  simple_db_backup restore <dbname> <file>       Restore custom-format dump into DB\n"
        "  simple_db_backup select <query>                Run a SELECT against default DB\n"
        "  simple_db_backup insert <table> <id> <fn> <ln>\n"
        "  simple_db_backup update <table> <id> <fn> <ln>\n"
        "  simple_db_backup delete <table> <fn> <ln>\n"
        "\n"
        "Config is loaded from " RESOURCES_PATH "database_config.ini\n";
}

int main(int argc, char** argv)
{
    Config cfg;
    const std::string cfgPath = std::string(RESOURCES_PATH) + "database_config.ini";
    if (!loadConfig(cfgPath, cfg)) {
        std::cerr << "Could not open config: " << cfgPath << '\n';
        return 1;
    }

    if (argc < 2) { printUsage(); return 0; }

    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) args.emplace_back(argv[i]);
    const std::string& cmd = args[0];

    auto need = [&](size_t n) {
        if (args.size() < n + 1) {
            std::cerr << "Not enough arguments for '" << cmd << "'.\n";
            printUsage();
            std::exit(2);
        }
    };

    if (cmd == "help" || cmd == "-h" || cmd == "--help") {
        printUsage();
        return 0;
    }

    if (cmd == "backup") {
        std::string db = (args.size() >= 2) ? args[1] : cfg.dbName;
        std::string out = cfg.outputDir.empty()
            ? (db + ".dump")
            : (cfg.outputDir + "/" + db + ".dump");
        return backupDatabase(cfg, db, out);
    }

    if (cmd == "backup-all") {
        std::string out = cfg.outputDir.empty()
            ? std::string("cluster.sql")
            : (cfg.outputDir + "/cluster.sql");
        return backupAll(cfg, out);
    }

    if (cmd == "backup-rotate") {
        std::string db = (args.size() >= 2) ? args[1] : cfg.dbName;
        int slot = rotationSlot();
        std::string out = rotatedBackupPath(cfg, db, slot);
        std::cout << "Rotation slot: " << slot << " (of 168)\n";
        return backupDatabase(cfg, db, out);
    }

    if (cmd == "restore") {
        need(2);
        return restoreDatabase(cfg, args[1], args[2]);
    }

    std::string conn = buildConnInfo(cfg);

    if (cmd == "select") {
        need(1);
        return selectFromDatabase(conn, args[1]);
    }
    if (cmd == "insert") {
        need(4);
        insertData(conn, args[1], std::stoi(args[2]), args[3], args[4]);
        return 0;
    }
    if (cmd == "update") {
        need(4);
        updateData(conn, args[1], std::stoi(args[2]), args[3], args[4]);
        return 0;
    }
    if (cmd == "delete") {
        need(3);
        deleteData(conn, args[1], args[2], args[3]);
        return 0;
    }

    std::cerr << "Unknown command: " << cmd << "\n\n";
    printUsage();
    return 2;
}
