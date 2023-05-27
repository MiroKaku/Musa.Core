#include "miutil.hpp"


namespace Mi::Util
{
    NTSTATUS ImageEnumerateExports(
        _In_ void* BaseOfImage,
        _In_ bool(CALLBACK *Callback)(uint32_t Ordinal, const char* Name, const void* Address, void* Context),
        _In_opt_ void* Context
    )
    {
        ULONG DataEntrySize   = 0;
        const void* DataEntry = RtlImageDirectoryEntryToData(BaseOfImage, TRUE,
            IMAGE_DIRECTORY_ENTRY_EXPORT, &DataEntrySize);
        if (DataEntry == nullptr) {
            return STATUS_INVALID_IMAGE_FORMAT;
        }

        const auto ExportEntry    = static_cast<PCIMAGE_EXPORT_DIRECTORY>(DataEntry);
        const auto AddressOfNames = reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(BaseOfImage) + ExportEntry->AddressOfNames);
        const auto AddressOfFuncs = reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(BaseOfImage) + ExportEntry->AddressOfFunctions);
        const auto AddressOfOrdis = reinterpret_cast<uint16_t*>(static_cast<uint8_t*>(BaseOfImage) + ExportEntry->AddressOfNameOrdinals);

        for (uint32_t Ordinal = 0; Ordinal < ExportEntry->NumberOfFunctions; ++Ordinal) {
            if (0 == AddressOfFuncs[Ordinal]) {
                continue;
            }

            const char*    ExportName    = nullptr;
            const uint32_t ExportOrdinal = Ordinal + ExportEntry->Base;
            const void*    ExportAddress = static_cast<void*>(static_cast<uint8_t*>(BaseOfImage) + AddressOfFuncs[Ordinal]);

            for (uint32_t Idx = 0u; Idx <= ExportEntry->NumberOfNames; ++Idx) {
                if (Ordinal == AddressOfOrdis[Idx]) {
                    ExportName = reinterpret_cast<const char*>(static_cast<uint8_t*>(BaseOfImage) + AddressOfNames[Idx]);
                    break;
                }
            }

            if (Callback(ExportOrdinal, ExportName, ExportAddress, Context)) {
                break;
            }
        }

        return STATUS_SUCCESS;
    }


}
