#pragma once
#include <unordered_map>
#include <string>
#include <stdexcept>
#include "CSharpRuntime.h"

inline const std::unordered_map<std::string, std::string>& get_system_type_map() {
    static const std::unordered_map<std::string, std::string> m = {
        {"System.Int32",   "int32"},
        {"System.UInt32",  "uint32"},
        {"System.Int16",   "short"},
        {"System.UInt16",  "ushort"},
        {"System.Int64",   "int64"},
        {"System.UInt64",  "uint64"},
        {"System.Byte",    "byte"},
        {"System.SByte",   "sbyte"},
        {"System.Boolean","bool"},
        {"System.Single", "float"},
        {"System.Double", "double"},
        {"System.String", "string"},
        {"System.Char",   "char"},
        {"System.Object","object"},
        {"System.Void",   "void"},
    };
    return m;
}

inline const std::unordered_map<std::string, std::string>& get_field_type_map() {
    static const std::unordered_map<std::string, std::string> m = {
        {"MapField<",      "map<"},
        {"ByteString",     "bytes"},
        {"RepeatedField<", "repeated "}
    };
    return m;
}

inline bool contains(const std::string& s, const std::string& sub) {
    return s.find(sub) != std::string::npos;
}

inline size_t count_occurrences(const std::string& s, const std::string& sub) {
    if (sub.empty()) return 0;

    size_t count = 0;
    size_t pos = 0;
    while ((pos = s.find(sub, pos)) != std::string::npos) {
        ++count;
        pos += sub.size();
    }
    return count;
}


inline std::string get_reflected_type(CRuntimeType t) {
    if (!t) return "";

    std::string name = t.Name().AsString();
    CRuntimeType rt = t.ReflectedType();

    if (!t.IsGenericType() && rt && rt.raw_ptr() != 0) {
        std::string parent = get_reflected_type(rt);
        if (!parent.empty()) {
            return parent + "." + name;
        }
    }
    return name;
}

inline std::string get_runtime_type_name(CRuntimeType t, bool alias) {
    if (!t || t.raw_ptr() == 0)
        return "";

    // array
    if (t.IsArray()) {
        CRuntimeType elem = t.ElementType();
        std::string out = get_runtime_type_name(elem, alias);
        int rank = t.ArrayRank();
        out.push_back('[');
        if (rank > 1) {
            out.append(rank - 1, ',');
        }
        out.push_back(']');
        return out;
    }

    // pointer
    if (t.IsPointer()) {
        CRuntimeType elem = t.ElementType();
        return get_runtime_type_name(elem, alias) + "*";
    }

    // by-ref
    if (t.IsByRef()) {
        CRuntimeType elem = t.ElementType();
        return get_runtime_type_name(elem, alias) + "&";
    }

    // generic
    if (t.IsGenericType()) {
        std::string name = t.Name().AsString();
        auto pos = name.find('`');
        if (pos != std::string::npos) {
            name = name.substr(0, pos);
        }

        auto args = t.GenericArguments();
        std::vector<std::string> parts;
        for (size_t i = 0; i < args.length(); ++i) {
            parts.push_back(get_runtime_type_name(args.get<CRuntimeType>(i), alias));
        }

        std::string joined;
        for (size_t i = 0; i < parts.size(); ++i) {
            if (i) joined += ", ";
            joined += parts[i];
        }

        return name + "<" + joined + ">";
    }

    // System alias
    if (alias) {
        CIl2CppObject obj(t.raw_ptr());
        std::string ns = obj.get_class().namespace_name();
        if (ns == "System") {
            std::string full = t.FullName().AsString();
            const auto& map = get_system_type_map();
            auto it = map.find(full);
            if (it != map.end()) {
                return it->second;
            }
        }
    }

    // default
    return get_reflected_type(t);
}

inline std::string replace_all(std::string str, const std::string& from, const std::string& to) {
    if (from.empty()) return str;

    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos) {
        str.replace(pos, from.length(), to);
        pos += to.length();
    }
    return str;
}
