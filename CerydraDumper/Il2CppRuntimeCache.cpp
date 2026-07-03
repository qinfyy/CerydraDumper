#include "pch.h"
#include "Il2CppRuntimeCache.h"
#include "PrintHelper.h"
#include <mutex>
#include <unordered_map>

using namespace Cerydra::IL2CPP;

namespace
{
    constexpr int32_t FIELD_ATTRIBUTE_STATIC = 0x0010;
    constexpr int32_t FIELD_ATTRIBUTE_LITERAL = 0x0040;
    constexpr int32_t METHOD_ATTRIBUTE_STATIC = 0x0010;

    std::once_flag g_initOnce;
    bool g_initialized = false;

    std::vector<Assembly*> g_assemblies;
    std::vector<Image*> g_images;
    std::vector<Class*> g_classes;
    std::vector<Field*> g_fields;
    std::vector<Method*> g_methods;
    std::vector<Type*> g_types;
    std::vector<Method::Arg*> g_args;

    std::unordered_map<void*, Assembly*> g_assemblyByAddress;
    std::unordered_map<void*, Image*> g_imageByAddress;
    std::unordered_map<void*, Class*> g_classByAddress;
    std::unordered_map<void*, Field*> g_fieldByAddress;
    std::unordered_map<void*, Method*> g_methodByAddress;
    std::unordered_map<void*, Type*> g_typeByAddress;
    std::unordered_map<std::string, Assembly*> g_assemblyByName;
    std::unordered_map<std::string, Class*> g_classByFullName;
    std::unordered_map<std::string, Class*> g_classByTypeName;
    std::unordered_multimap<std::string, Class*> g_classesBySimpleName;
    std::unordered_map<std::string, Method*> g_methodBySignature;

    std::string SafeString(const char* value)
    {
        return value ? std::string(value) : std::string();
    }

    std::string StripDllExtension(std::string value)
    {
        if (value.size() > 4) {
            const auto suffix = value.substr(value.size() - 4);
            if (_stricmp(suffix.c_str(), ".dll") == 0) {
                value.resize(value.size() - 4);
            }
        }
        return value;
    }

    void AddClassNameKeys(Class* klass)
    {
        if (!klass) {
            return;
        }

        if (!klass->fullName.empty()) {
            g_classByFullName[klass->fullName] = klass;
        }
        if (!klass->name.empty()) {
            g_classesBySimpleName.emplace(klass->name, klass);
        }
        if (klass->byvalType) {
            if (!klass->byvalType->name.empty()) {
                g_classByTypeName[klass->byvalType->name] = klass;
            }
            if (!klass->byvalType->aliasName.empty() && klass->byvalType->aliasName != klass->byvalType->name) {
                g_classByTypeName[klass->byvalType->aliasName] = klass;
            }
        }
    }
}

void Il2CppRuntimeCache::Init()
{
    std::call_once(g_initOnce, [] {
        DebugPrintA("[RuntimeCache] 开始缓存 IL2CPP 元数据...\n");

        auto domain = il2cpp_domain_get();
        if (!domain) {
            DebugPrintA("[RuntimeCache] [ERROR] il2cpp_domain_get 返回空\n");
            return;
        }

        il2cpp_thread_attach(domain);
        BuildAssemblies();

        for (auto klass : g_classes) {
            if (!klass || !klass->parentClass) {
                auto parent = il2cpp_class_get_parent(reinterpret_cast<Il2CppClass*>(klass->address));
                auto it = g_classByAddress.find(parent);
                if (it != g_classByAddress.end()) {
                    klass->parentClass = it->second;
                }
            }
        }
        BuildInterfaces();

        g_initialized = true;
        DebugPrintA(
            "[RuntimeCache] 完成: assemblies=%zu, classes=%zu, fields=%zu, methods=%zu, types=%zu\n",
            g_assemblies.size(),
            g_classes.size(),
            g_fields.size(),
            g_methods.size(),
            g_types.size());
    });
}

bool Il2CppRuntimeCache::IsInitialized()
{
    return g_initialized;
}

Assembly* Il2CppRuntimeCache::GetAssembly(const std::string& name)
{
    auto it = g_assemblyByName.find(name);
    if (it != g_assemblyByName.end()) {
        return it->second;
    }

    auto stripped = StripDllExtension(name);
    it = g_assemblyByName.find(stripped);
    return it == g_assemblyByName.end() ? nullptr : it->second;
}

Class* Il2CppRuntimeCache::GetClass(const std::string& fullOrAliasName)
{
    auto it = g_classByFullName.find(fullOrAliasName);
    if (it != g_classByFullName.end()) {
        return it->second;
    }

    it = g_classByTypeName.find(fullOrAliasName);
    if (it != g_classByTypeName.end()) {
        return it->second;
    }

    auto range = g_classesBySimpleName.equal_range(fullOrAliasName);
    return range.first == range.second ? nullptr : range.first->second;
}

Class* Il2CppRuntimeCache::GetClass(const std::string& namespaze, const std::string& name)
{
    return GetClass(MakeFullClassName(namespaze, name));
}

Class* Il2CppRuntimeCache::GetClassByAddress(uintptr_t address)
{
    auto it = g_classByAddress.find(reinterpret_cast<void*>(address));
    return it == g_classByAddress.end() ? nullptr : it->second;
}

Method* Il2CppRuntimeCache::GetMethod(const std::string& key)
{
    auto it = g_methodBySignature.find(key);
    return it == g_methodBySignature.end() ? nullptr : it->second;
}

Method* Il2CppRuntimeCache::GetMethodByAddress(uintptr_t address)
{
    auto it = g_methodByAddress.find(reinterpret_cast<void*>(address));
    return it == g_methodByAddress.end() ? nullptr : it->second;
}

Field* Il2CppRuntimeCache::GetFieldByAddress(uintptr_t address)
{
    auto it = g_fieldByAddress.find(reinterpret_cast<void*>(address));
    return it == g_fieldByAddress.end() ? nullptr : it->second;
}

Type* Il2CppRuntimeCache::GetTypeByAddress(uintptr_t address)
{
    auto it = g_typeByAddress.find(reinterpret_cast<void*>(address));
    return it == g_typeByAddress.end() ? nullptr : it->second;
}

const std::vector<Assembly*>& Il2CppRuntimeCache::Assemblies()
{
    return g_assemblies;
}

const std::vector<Class*>& Il2CppRuntimeCache::Classes()
{
    return g_classes;
}

const std::vector<Method*>& Il2CppRuntimeCache::Methods()
{
    return g_methods;
}

void Il2CppRuntimeCache::BuildAssemblies()
{
    auto domain = il2cpp_domain_get();
    size_t assemblyCount = 0;
    auto assemblies = il2cpp_domain_get_assemblies(domain, &assemblyCount);
    if (!assemblies) {
        return;
    }

    for (size_t i = 0; i < assemblyCount; ++i) {
        const auto nativeAssembly = assemblies[i];
        if (!nativeAssembly) {
            continue;
        }

        const auto nativeImage = il2cpp_assembly_get_image(nativeAssembly);
        if (!nativeImage) {
            continue;
        }

        auto assembly = new Assembly();
        assembly->address = const_cast<Il2CppAssembly*>(nativeAssembly);

        auto image = new Image();
        image->address = const_cast<Il2CppImage*>(nativeImage);
        image->name = SafeString(il2cpp_image_get_name(nativeImage));
        image->file = SafeString(il2cpp_image_get_filename(nativeImage));
        image->assembly = assembly;

        assembly->name = StripDllExtension(image->name);
        assembly->file = image->file;
        assembly->image = image;

        g_assemblies.push_back(assembly);
        g_images.push_back(image);
        g_assemblyByAddress[assembly->address] = assembly;
        g_imageByAddress[image->address] = image;
        g_assemblyByName[assembly->name] = assembly;
        if (!image->name.empty()) {
            g_assemblyByName[image->name] = assembly;
        }

        BuildClasses(assembly, image);
    }
}

void Il2CppRuntimeCache::BuildClasses(Assembly*, Image* image)
{
    const auto nativeImage = reinterpret_cast<const Il2CppImage*>(image->address);
    const auto classCount = il2cpp_image_get_class_count(nativeImage);

    for (size_t i = 0; i < classCount; ++i) {
        const auto nativeClass = il2cpp_image_get_class(nativeImage, i);
        if (!nativeClass) {
            continue;
        }

        auto klass = new Class();
        klass->address = const_cast<Il2CppClass*>(nativeClass);
        klass->name = SafeString(il2cpp_class_get_name(const_cast<Il2CppClass*>(nativeClass)));
        klass->namespaze = SafeString(il2cpp_class_get_namespace(const_cast<Il2CppClass*>(nativeClass)));
        klass->fullName = MakeFullClassName(klass->namespaze, klass->name);
        klass->image = image;
        klass->flags = il2cpp_class_get_flags(nativeClass);
        klass->isEnum = il2cpp_class_is_enum(nativeClass);
        klass->isValueType = il2cpp_class_is_valuetype(nativeClass);
        klass->isInterface = il2cpp_class_is_interface(nativeClass);

        auto parent = il2cpp_class_get_parent(const_cast<Il2CppClass*>(nativeClass));
        if (parent) {
            klass->parent = SafeString(il2cpp_class_get_name(parent));
            auto it = g_classByAddress.find(parent);
            if (it != g_classByAddress.end()) {
                klass->parentClass = it->second;
            }
        }

        klass->byvalType = GetOrCreateType(il2cpp_class_get_type(const_cast<Il2CppClass*>(nativeClass)));
        if (klass->byvalType) {
            klass->byvalType->klass = klass;
        }

        image->classes.push_back(klass);
        g_classes.push_back(klass);
        g_classByAddress[klass->address] = klass;
        AddClassNameKeys(klass);

        BuildFields(klass);
        BuildMethods(klass);
    }
}

void Il2CppRuntimeCache::BuildFields(Class* klass)
{
    void* iter = nullptr;
    while (true) {
        auto nativeField = il2cpp_class_get_fields(reinterpret_cast<Il2CppClass*>(klass->address), &iter);
        if (!nativeField) {
            break;
        }

        auto field = new Field();
        field->address = nativeField;
        field->name = SafeString(il2cpp_field_get_name(nativeField));
        field->type = GetOrCreateType(il2cpp_field_get_type(nativeField));
        field->klass = klass;
        field->flags = il2cpp_field_get_flags(nativeField);
        field->offset = static_cast<int32_t>(il2cpp_field_get_offset(nativeField));
        field->isStatic = (field->flags & FIELD_ATTRIBUTE_STATIC) != 0;
        field->isLiteral = (field->flags & FIELD_ATTRIBUTE_LITERAL) != 0;

        klass->fields.push_back(field);
        g_fields.push_back(field);
        g_fieldByAddress[field->address] = field;
    }
}

void Il2CppRuntimeCache::BuildMethods(Class* klass)
{
    void* iter = nullptr;
    while (true) {
        const auto nativeMethod = il2cpp_class_get_methods(reinterpret_cast<Il2CppClass*>(klass->address), &iter);
        if (!nativeMethod) {
            break;
        }

        auto method = new Method();
        method->address = const_cast<MethodInfo*>(nativeMethod);
        method->name = SafeString(il2cpp_method_get_name(nativeMethod));
        method->klass = klass;
        method->returnType = GetOrCreateType(il2cpp_method_get_return_type(nativeMethod));

        uint32_t iflags = 0;
        method->flags = static_cast<int32_t>(il2cpp_method_get_flags(nativeMethod, &iflags));
        method->isStatic = (method->flags & METHOD_ATTRIBUTE_STATIC) != 0;
        method->function = const_cast<MethodInfo*>(nativeMethod)->method_pointer;

        const auto paramCount = il2cpp_method_get_param_count(nativeMethod);
        method->args.reserve(paramCount);
        for (uint32_t i = 0; i < paramCount; ++i) {
            auto arg = new Method::Arg();
            if (il2cpp_method_get_param_name.address()) {
                arg->name = SafeString(il2cpp_method_get_param_name(nativeMethod, i));
            }
            if (arg->name.empty()) {
                arg->name = "arg" + std::to_string(i + 1);
            }
            arg->type = GetOrCreateType(il2cpp_method_get_param(nativeMethod, i));
            method->args.push_back(arg);
            g_args.push_back(arg);
        }

        klass->methods.push_back(method);
        g_methods.push_back(method);
        g_methodByAddress[method->address] = method;
        g_methodBySignature[method->SignatureKey()] = method;
    }
}

void Il2CppRuntimeCache::BuildInterfaces()
{
    for (auto* klass : g_classes) {
        if (!klass || !klass->address) {
            continue;
        }

        void* iter = nullptr;
        while (true) {
            auto* nativeInterface = il2cpp_class_get_interfaces(reinterpret_cast<Il2CppClass*>(klass->address), &iter);
            if (!nativeInterface) {
                break;
            }

            auto it = g_classByAddress.find(nativeInterface);
            if (it != g_classByAddress.end()) {
                klass->interfaces.push_back(it->second);
            }
        }
    }
}

Type* Il2CppRuntimeCache::GetOrCreateType(const Il2CppType* nativeType)
{
    if (!nativeType) {
        return nullptr;
    }

    auto key = const_cast<Il2CppType*>(nativeType);
    auto it = g_typeByAddress.find(key);
    if (it != g_typeByAddress.end()) {
        return it->second;
    }

    auto type = new Type();
    type->address = key;
    type->name = SafeString(il2cpp_type_get_name(nativeType));
    type->aliasName = AliasTypeName(type->name);
    if (il2cpp_type_get_type.address()) {
        type->typeEnum = il2cpp_type_get_type(nativeType);
    }
    type->attrs = il2cpp_type_get_attrs(nativeType);
    type->byRef = il2cpp_type_is_byref(nativeType);

    auto nativeClass = il2cpp_class_from_type(nativeType);
    auto classIt = g_classByAddress.find(nativeClass);
    if (classIt != g_classByAddress.end()) {
        type->klass = classIt->second;
    }

    g_types.push_back(type);
    g_typeByAddress[type->address] = type;
    return type;
}
