#include "driver_control.hpp"
#include <exception>

driver_control::driver_control(const char *path) {
    this->driver_handle =
        CreateFileA(path, GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
}

bool driver_control::ready() { return driver_handle != 0; }

driver_control::~driver_control() { CloseHandle(this->driver_handle); }

ULONG driver_control::get_process_id_by_name(const char *process_name,
                                             size_t process_name_size) {
    if (this->driver_handle) {
        ULONG bytes;
        NF_PROCESS_ID_REQUEST process_id_request = {0};
        memcpy(process_id_request.process_name, process_name,
               process_name_size);
        if (DeviceIoControl(driver_handle, NF_GET_PROCESS_ID,
                            &process_id_request, sizeof(process_id_request),
                            &process_id_request, sizeof(process_id_request),
                            &bytes, 0)) {
            return process_id_request.process_id;
        } else {
            return 0;
        }
    }
    return 0;
}

ULONG driver_control::get_module_base(ULONG process_id,
                                      const wchar_t *module_name,
                                      size_t module_name_size) {
    if (this->driver_handle) {
        ULONG bytes;
        NF_MODULE_REQUEST module_request = {0};
        module_request.process_id = process_id;
        memcpy(module_request.module_name, module_name, module_name_size);
        if (DeviceIoControl(driver_handle, NF_GET_MODULE, &module_request,
                            sizeof(module_request), &module_request,
                            sizeof(module_request), &bytes, 0)) {
            return module_request.address;
        } else {
            return 0;
        }
    }
    return 0;
}

void driver_control::keyboard_event(USHORT unitId, USHORT makecode,
                                    USHORT flags, USHORT reserved,
                                    ULONG extraInformation) {
    if (this->driver_handle) {
        ULONG bytes;
        NF_KEYBOARD_REQUEST kb_request = {0};
        kb_request.ExtraInformation = extraInformation;
        kb_request.Flags = flags;
        kb_request.MakeCode = makecode;
        kb_request.Reserved = reserved;
        kb_request.UnitId = unitId;
        DeviceIoControl(driver_handle, NF_KB_EVENT, &kb_request,
                        sizeof(kb_request), &kb_request, sizeof(kb_request),
                        &bytes, 0);
    }
}

void driver_control::mouse_event(long x, long y, unsigned short button_flags,
                                 unsigned short flags) {
    if (this->driver_handle) {
        ULONG bytes;
        NF_MOUSE_REQUEST mouse_request = {0};
        mouse_request.x = x;
        mouse_request.y = y;
        mouse_request.flags = flags;
        mouse_request.button_flags = button_flags;
        DeviceIoControl(driver_handle, NF_MOUSE_EVENT, &mouse_request,
                        sizeof(mouse_request), &mouse_request,
                        sizeof(mouse_request), &bytes, 0);
    }
}
