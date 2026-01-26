#pragma once
#include "Il2CppApiWrapper.h"
#include "Bind.h"
#include <string>
#include <vector>
#include <stdexcept>
#include "CCSharpRuntime.h"

class CModuleManager : public CIl2CppWrapBase
{
public:
    using CIl2CppWrapBase::CIl2CppWrapBase;

	CS_CLASS("RPG.Client.ModuleManager");

	    // ¶ÔÓ¦ Rust µÄ `cs_field!(modules, "modules", self, |v| -> NativeList { NativeList(v.0) });`
	CS_FIELD_INSTANCE(Modules, "modules", CNativeList);
	//inline CNativeList Modules() const {
	//	if (!ptr)
	//		throw std::runtime_error("Object is null! Cannot access field " "modules");
	//	auto obj_class = CRuntimeType::FromClass(GetClass());
	//	auto field_info = obj_class.GetField("modules");
	//	if (!field_info)
	//		throw std::runtime_error("No such field: " "modules");
	//	auto value_obj = field_info.get_value_object(CIl2CppObject(ptr));
	//	if (!value_obj)
	//		throw std::runtime_error("Field " "modules" " is null");
	//	return CNativeList(value_obj.raw_ptr());
	//}

};

