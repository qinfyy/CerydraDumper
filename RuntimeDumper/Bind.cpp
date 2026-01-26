#include "pch.h"
#include "Bind.h"

void SehTranslator(unsigned int code, _EXCEPTION_POINTERS* ep) {
    std::ostringstream msg;
    msg << "SEH Exception code: 0x" << std::hex << code
        << " at address: " << ep->ExceptionRecord->ExceptionAddress;
    throw std::runtime_error(msg.str());
}

void InitSehTranslator() {
    static std::once_flag flag;
    std::call_once(flag, []() {
        _set_se_translator(SehTranslator);
        });
}