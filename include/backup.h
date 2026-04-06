#pragma once
#include <string>
#include "config.h"

// Back up a single database to outFile (custom format, restorable via pg_restore).
// Returns the pg_dump exit code (0 = success).
int backupDatabase(const Config& cfg, const std::string& dbName, const std::string& outFile);

// Back up the entire cluster (all DBs + roles) to outFile as plain SQL.
int backupAll(const Config& cfg, const std::string& outFile);

// Restore a custom-format dump into dbName. The database must already exist.
int restoreDatabase(const Config& cfg, const std::string& dbName, const std::string& inFile);

// Compute a rotation slot 0..(hours-1) for time-based file rotation.
// Default: 7 days * 24 hours = 168 slots.
int rotationSlot(int totalHours = 7 * 24);

// Produce an output path in cfg.outputDir using the given dbName and slot.
std::string rotatedBackupPath(const Config& cfg, const std::string& dbName, int slot);
