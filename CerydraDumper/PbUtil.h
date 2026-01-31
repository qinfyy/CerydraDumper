#pragma once
#include <unordered_map>
#include <string>
#include <stdexcept>
#include "CSharpRuntime.h"

const std::unordered_map<std::string, std::string>& GetSystemTypeMap();

const std::unordered_map<std::string, std::string>& GetFieldTypeMap();

bool Contains(const std::string& s, const std::string& sub);

size_t CountOccurrences(const std::string& s, const std::string& sub);

std::string GetReflectedType(CRuntimeType t);

std::string GetRuntimeTypeName(CRuntimeType t, bool alias);

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to);
