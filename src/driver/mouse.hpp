#pragma once

#include "definitions.hpp"

extern "C" POBJECT_TYPE *IoDriverObjectType;

typedef VOID (*MouseClassServiceCallback)(PDEVICE_OBJECT DeviceObject,
                                          PMOUSE_INPUT_DATA InputDataStart,
                                          PMOUSE_INPUT_DATA InputDataEnd,
                                          PULONG InputDataConsumed);

typedef VOID (*KeyboardClassServiceCallback)(
    PDEVICE_OBJECT DeviceObject, PKEYBOARD_INPUT_DATA InputDataStart,
    PKEYBOARD_INPUT_DATA InputDataEnd, PULONG InputDataConsumed);

typedef struct _MOUSE_OBJECT {
    PDEVICE_OBJECT mouse_device;
    MouseClassServiceCallback service_callback;
} MOUSE_OBJECT, *PMOUSE_OBJECT;

typedef struct _KB_OBJECT {
    PDEVICE_OBJECT kb_device;
    KeyboardClassServiceCallback service_callback;
} KEYBOARD_OBJECT, *PKEYBOARD_OBJECT;

namespace mouse {
NTSTATUS init_mouse(PMOUSE_OBJECT mouse_obj);
NTSTATUS init_keyboard(PKEYBOARD_OBJECT kb_obj);

void keyboard_event(KEYBOARD_OBJECT kb_obj, PNF_KEYBOARD_REQUEST kr);
void mouse_event(MOUSE_OBJECT mouse_obj, long x, long y,
                 unsigned short button_flags, USHORT flags);
} // namespace mouse