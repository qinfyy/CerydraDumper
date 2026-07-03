#include "pch.h"
#include "Il2CppRuntimeCache.h"
#include "PrintHelper.h"
#include <mutex>
#include <unordered_set>

using namespace Cerydra::IL2CPP;

namespace
{
    constexpr int32_t FIELD_ATTRIBUTE_STATIC = 0x0010;
    constexpr int32_t FIELD_ATTRIBUTE_LITERAL = 0x0040;
    constexpr int32_t METHOD_ATTRIBUTE_STATIC = 0x0010;

    std::once_flag g_initOnce;
    bool g_initialized = false;

    std::vector<Assembly*> s_assemblies;

    struct MetadataCount
    {
        size_t assemblies{};
        size_t classes{};
        size_t fields{};
        size_t methods{};
        size_t types{};
    };

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

    template <typename Func>
    void ForEachClass(Func&& func)
    {
        for (auto* assembly : s_assemblies) {
            if (!assembly || !assembly->image) {
                continue;
            }

            for (auto* klass : assembly->image->classes) {
                if (klass) {
                    func(klass);
                }
            }
        }
    }

    void AddTypeAddress(std::unordered_set<void*>& addresses, const Type* type)
    {
        if (type && type->address) {
            addresses.insert(type->address);
        }
    }

    MetadataCount CountMetadata()
    {
        MetadataCount count{};
        std::unordered_set<void*> typeAddresses;
        count.assemblies = s_assemblies.size();

        ForEachClass([&](const Class* klass) {
            ++count.classes;
            count.fields += klass->fields.size();
            count.methods += klass->methods.size();
            AddTypeAddress(typeAddresses, klass->byvalType);

            for (const auto* field : klass->fields) {
                if (field) {
                    AddTypeAddress(typeAddresses, field->type);
                }
            }

            for (const auto* method : klass->methods) {
                if (!method) {
                    continue;
                }

                AddTypeAddress(typeAddresses, method->returnType);
                for (const auto* arg : method->args) {
                    if (arg) {
                        AddTypeAddress(typeAddresses, arg->type);
                    }
                }
            }
        });

        count.types = typeAddresses.size();
        return count;
    }

    void ResolveTypeClass(Type* type)
    {
        if (!type || type->klass || !type->address) {
            return;
        }

        auto* nativeClass = il2cpp_class_from_type(reinterpret_cast<const Il2CppType*>(type->address));
        type->klass = FindClassByAddress(reinterpret_cast<uintptr_t>(nativeClass));
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
        ResolveClassLinks();

        g_initialized = true;
        const auto count = CountMetadata();
        DebugPrintA(
            "[RuntimeCache] 完成: assemblies=%zu, classes=%zu, fields=%zu, methods=%zu, types=%zu\n",
            count.assemblies,
            count.classes,
            count.fields,
            count.methods,
            count.types);
    });
}

bool Il2CppRuntimeCache::IsInitialized()
{
    return g_initialized;
}

Assembly* Il2CppRuntimeCache::GetAssembly(const std::string& name)
{
    const auto strippedName = StripDllExtension(name);
    for (auto* assembly : s_assemblies) {
        if (!assembly) {
            continue;
        }

        if (assembly->name == name || assembly->name == strippedName) {
            return assembly;
        }

        if (assembly->image && assembly->image->name == name) {
            return assembly;
        }
    }

    return nullptr;
}

const std::vector<Assembly*>& Il2CppRuntimeCache::Assemblies()
{
    return s_assemblies;
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

        s_assemblies.push_back(assembly);

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
        }

        klass->byvalType = CreateType(il2cpp_class_get_type(const_cast<Il2CppClass*>(nativeClass)));
        if (klass->byvalType) {
            klass->byvalType->klass = klass;
        }

        image->classes.push_back(klass);

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
        field->type = CreateType(il2cpp_field_get_type(nativeField));
        field->klass = klass;
        field->flags = il2cpp_field_get_flags(nativeField);
        field->offset = static_cast<int32_t>(il2cpp_field_get_offset(nativeField));
        field->isStatic = (field->flags & FIELD_ATTRIBUTE_STATIC) != 0;
        field->isLiteral = (field->flags & FIELD_ATTRIBUTE_LITERAL) != 0;

        klass->fields.push_back(field);
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
        method->returnType = CreateType(il2cpp_method_get_return_type(nativeMethod));

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
            arg->type = CreateType(il2cpp_method_get_param(nativeMethod, i));
            method->args.push_back(arg);
        }

        klass->methods.push_back(method);
    }
}

void Il2CppRuntimeCache::ResolveClassLinks()
{
    ForEachClass([](Class* klass) {
        if (!klass || !klass->address) {
            return;
        }

        auto* parent = il2cpp_class_get_parent(reinterpret_cast<Il2CppClass*>(klass->address));
        klass->parentClass = FindClassByAddress(reinterpret_cast<uintptr_t>(parent));

        ResolveTypeClass(klass->byvalType);
        for (auto* field : klass->fields) {
            if (field) {
                ResolveTypeClass(field->type);
            }
        }
        for (auto* method : klass->methods) {
            if (!method) {
                continue;
            }

            ResolveTypeClass(method->returnType);
            for (auto* arg : method->args) {
                if (arg) {
                    ResolveTypeClass(arg->type);
                }
            }
        }

        klass->interfaces.clear();
        void* iter = nullptr;
        while (true) {
            auto* nativeInterface = il2cpp_class_get_interfaces(reinterpret_cast<Il2CppClass*>(klass->address), &iter);
            if (!nativeInterface) {
                break;
            }

            auto* interfaceClass = FindClassByAddress(reinterpret_cast<uintptr_t>(nativeInterface));
            if (interfaceClass) {
                klass->interfaces.push_back(interfaceClass);
            }
        }
    });
}

Type* Il2CppRuntimeCache::CreateType(const Il2CppType* nativeType)
{
    if (!nativeType) {
        return nullptr;
    }

    auto type = new Type();
    type->address = const_cast<Il2CppType*>(nativeType);
    type->name = SafeString(il2cpp_type_get_name(nativeType));
    type->aliasName = AliasTypeName(type->name);
    if (il2cpp_type_get_type.address()) {
        type->typeEnum = il2cpp_type_get_type(nativeType);
    }
    type->attrs = il2cpp_type_get_attrs(nativeType);
    type->byRef = il2cpp_type_is_byref(nativeType);

    auto nativeClass = il2cpp_class_from_type(nativeType);
    type->klass = FindClassByAddress(reinterpret_cast<uintptr_t>(nativeClass));

    return type;
}
