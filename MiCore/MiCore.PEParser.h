#pragma once


namespace Mi::PEParser
{
    NTSTATUS ImageEnumerateExports(
        _In_     bool(CALLBACK* Callback)(uint32_t Ordinal, const char* Name, const void* Address, void* Context),
        _In_opt_ void* Context,
        _In_     void* BaseOfImage,
        _In_     bool  MappedAsImage
    );

}
