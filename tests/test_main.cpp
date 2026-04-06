#include <catch2/catch_test_macros.hpp>

#include "config.h"
#include "backup.h"

#include <fstream>
#include <cstdio>

TEST_CASE("buildConnInfo assembles libpq key=value string", "[config]") {
    Config cfg;
    cfg.host = "localhost";
    cfg.port = "5432";
    cfg.user = "postgres";
    cfg.password = "secret";
    cfg.dbName = "mydb";

    std::string conn = buildConnInfo(cfg);
    REQUIRE(conn.find("host=localhost")   != std::string::npos);
    REQUIRE(conn.find("port=5432")        != std::string::npos);
    REQUIRE(conn.find("dbname=mydb")      != std::string::npos);
    REQUIRE(conn.find("user=postgres")    != std::string::npos);
    REQUIRE(conn.find("password=secret")  != std::string::npos);
}

TEST_CASE("buildConnInfo overrideDb replaces dbName", "[config]") {
    Config cfg;
    cfg.host = "h"; cfg.port = "1"; cfg.user = "u";
    cfg.password = "p"; cfg.dbName = "default_db";

    std::string conn = buildConnInfo(cfg, "other_db");
    REQUIRE(conn.find("dbname=other_db")   != std::string::npos);
    REQUIRE(conn.find("dbname=default_db") == std::string::npos);
}

TEST_CASE("loadConfig parses a valid INI file", "[config]") {
    const char* path = "test_config_tmp.ini";
    {
        std::ofstream f(path);
        f << "[database]\n"
             "host=example.com\n"
             "port=6543\n"
             "user=alice\n"
             "password=wonderland\n"
             "dbname=appdb\n"
             "\n"
             "[backup]\n"
             "pg_bin=/usr/bin\n"
             "output_dir=/tmp/backups\n";
    }

    Config cfg;
    REQUIRE(loadConfig(path, cfg));
    REQUIRE(cfg.host      == "example.com");
    REQUIRE(cfg.port      == "6543");
    REQUIRE(cfg.user      == "alice");
    REQUIRE(cfg.password  == "wonderland");
    REQUIRE(cfg.dbName    == "appdb");
    REQUIRE(cfg.pgBin     == "/usr/bin");
    REQUIRE(cfg.outputDir == "/tmp/backups");

    std::remove(path);
}

TEST_CASE("loadConfig ignores comments and blank lines", "[config]") {
    const char* path = "test_config_comments.ini";
    {
        std::ofstream f(path);
        f << "# top comment\n"
             "; also a comment\n"
             "\n"
             "[database]\n"
             "host=h\n";
    }
    Config cfg;
    REQUIRE(loadConfig(path, cfg));
    REQUIRE(cfg.host == "h");
    std::remove(path);
}

TEST_CASE("loadConfig returns false for missing file", "[config]") {
    Config cfg;
    REQUIRE_FALSE(loadConfig("definitely_does_not_exist_12345.ini", cfg));
}

TEST_CASE("rotationSlot stays within range", "[backup]") {
    for (int i = 0; i < 10; ++i) {
        int slot = rotationSlot(168);
        REQUIRE(slot >= 0);
        REQUIRE(slot < 168);
    }
    REQUIRE(rotationSlot(24) < 24);
    REQUIRE(rotationSlot(1) == 0);
}

TEST_CASE("rotatedBackupPath formats correctly", "[backup]") {
    Config cfg;
    cfg.outputDir = "/var/backups";
    REQUIRE(rotatedBackupPath(cfg, "mydb", 42) == "/var/backups/backup_mydb_42.dump");

    cfg.outputDir = "/var/backups/"; // trailing slash should be fine
    REQUIRE(rotatedBackupPath(cfg, "mydb", 0) == "/var/backups/backup_mydb_0.dump");

    cfg.outputDir = ""; // empty dir still produces a valid relative path
    REQUIRE(rotatedBackupPath(cfg, "db", 5) == "backup_db_5.dump");
}
