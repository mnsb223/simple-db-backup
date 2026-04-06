#pragma once
#include <string>

int  selectFromDatabase(const std::string& connInfo, const std::string& selectQuery);

void insertData(const std::string& connInfo,
                const std::string& tableName, int id,
                const std::string& fName,
                const std::string& lName);

void deleteData(const std::string& connInfo,
                const std::string& tableName,
                const std::string& fName,
                const std::string& lName);

void updateData(const std::string& connInfo,
                const std::string& tableName, int id,
                const std::string& fName,
                const std::string& lName);
