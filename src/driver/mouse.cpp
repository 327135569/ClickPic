#include "mouse.hpp"
#include "definitions.hpp"
#include "driver.h"
#include "wdm.h"
#include <basetsd.h>
#include <ntdef.h>
#include <ntstatus.h>

NTSTATUS mouse::init_keyboard(PKEYBOARD_OBJECT keyboard_obj) {
    UNICODE_STRING class_string;
    RtlInitUnicodeString(&class_string, L"\\Driver\\KbdClass");

    PDRIVER_OBJECT class_driver_object = NULL;
    NTSTATUS status = ObReferenceObjectByName(
        &class_string, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType,
        KernelMode, NULL, (PVOID *)&class_driver_object);
    if (!NT_SUCCESS(status)) {
        DbgPrintEx(0, 0, "[norsefire]: kbdclass ref error");
        return status;
    }

    if (osv.dwMajorVersion == 10) {
        hid:
        UNICODE_STRING hid_string;
        RtlInitUnicodeString(&hid_string, L"\\Driver\\KbdHID");

        PDRIVER_OBJECT hid_driver_object = NULL;
        status = ObReferenceObjectByName(
            &hid_string, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType,
            KernelMode, NULL, (PVOID *)&hid_driver_object);
        if (!NT_SUCCESS(status)) {
            if (class_driver_object) {
                ObDereferenceObject(class_driver_object);
            }
            DbgPrintEx(0, 0, "[norsefire]: kbdhid ref error");
            return status;
        }

        PVOID class_driver_base = NULL;
        ULONG class_driver_size = 0;

        PDEVICE_OBJECT hid_device_object = hid_driver_object->DeviceObject;
        while (hid_device_object && !keyboard_obj->service_callback) {
            PDEVICE_OBJECT class_device_object =
                class_driver_object->DeviceObject;
            while (class_device_object && !keyboard_obj->service_callback) {
                if (!class_device_object->NextDevice &&
                    !keyboard_obj->kb_device) {
                    keyboard_obj->kb_device = class_device_object;
                }

                PULONG_PTR device_extension =
                    (PULONG_PTR)hid_device_object->DeviceExtension;
                ULONG_PTR device_ext_size =
                    ((ULONG_PTR)hid_device_object->DeviceObjectExtension -
                     (ULONG_PTR)hid_device_object->DeviceExtension) /
                    4;
                class_driver_base = class_driver_object->DriverStart;
                class_driver_size = class_driver_object->DriverSize;
                for (ULONG_PTR i = 0; i < device_ext_size; i++) {
                    if (device_extension[i] == (ULONG_PTR)class_device_object &&
                        device_extension[i + 1] >
                            (ULONG_PTR)class_driver_base &&
                        device_extension[i + 1] <
                            (ULONG_PTR)((PUCHAR)class_driver_base +
                                        class_driver_size)) {
                        keyboard_obj->service_callback =
                            (KeyboardClassServiceCallback)(device_extension[i +
                                                                            1]);
                        break;
                    }
                }
                class_device_object = class_device_object->NextDevice;
            }
            hid_device_object = hid_device_object->AttachedDevice;
        }

        if (!keyboard_obj->kb_device) {
            PDEVICE_OBJECT target_device_object =
                class_driver_object->DeviceObject;
            while (target_device_object) {
                if (!target_device_object->NextDevice) {
                    keyboard_obj->kb_device = target_device_object;
                    break;
                }
                target_device_object = target_device_object->NextDevice;
            }
        }

        ObDereferenceObject(class_driver_object);
        ObDereferenceObject(hid_driver_object);

        if (keyboard_obj->service_callback) {
            DbgPrintEx(0, 0, "[norsefire]: Keyboard initialized\n");
            return STATUS_SUCCESS;
        }
        DbgPrintEx(0, 0, "[norsefire]: Keyboard initialized fail\n");
        return STATUS_UNSUCCESSFUL;
    }

    if (osv.dwMajorVersion == 6) {

        PDRIVER_OBJECT hid_driver_object = NULL;
        UNICODE_STRING hid_string;
        RtlInitUnicodeString(&hid_string, L"\\Driver\\i8042prt");
        auto status = ObReferenceObjectByName(
            &hid_string, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType,
            KernelMode, NULL, (PVOID *)&hid_driver_object);

        if (!NT_SUCCESS(status)) {
            if (class_driver_object) {
                ObDereferenceObject(class_driver_object);
            }
            DbgPrintEx(0, 0, "[norsefire]: i8042prt ref fail\n");
            goto hid;
            return STATUS_UNSUCCESSFUL;
        }

        keyboard_obj->service_callback =
            (KeyboardClassServiceCallback)(((PUCHAR)class_driver_object
                                                ->DriverStart) +
                                           0x2990);
        DbgPrintEx(0, 0, "[norsefire]: keyboard class service callback %p\n",
                   keyboard_obj->service_callback);

        if (!keyboard_obj->kb_device) {
            PDEVICE_OBJECT target_device_object =
                class_driver_object->DeviceObject;
            while (target_device_object) {
                if (!target_device_object->NextDevice) {
                    keyboard_obj->kb_device = target_device_object;
                    break;
                }
                target_device_object = target_device_object->NextDevice;
            }
        }

        ObDereferenceObject(class_driver_object);
        ObDereferenceObject(hid_driver_object);

        if (keyboard_obj->service_callback) {
            DbgPrintEx(0, 0, "[norsefire]: Keyboard initialized\n");
            return STATUS_SUCCESS;
        }

        DbgPrintEx(0, 0, "[norsefire]: Keyboard initialized fail\n");
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_UNSUCCESSFUL;
}

NTSTATUS mouse::init_mouse(PMOUSE_OBJECT mouse_obj) {
    UNICODE_STRING class_string;
    RtlInitUnicodeString(&class_string, L"\\Driver\\MouClass");

    PDRIVER_OBJECT class_driver_object = NULL;
    NTSTATUS status = ObReferenceObjectByName(
        &class_string, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType,
        KernelMode, NULL, (PVOID *)&class_driver_object);
    if (!NT_SUCCESS(status)) {
        DbgPrintEx(0, 0, "[norsefire]: mouclass ref error");
        return status;
    }

    UNICODE_STRING hid_string;
    RtlInitUnicodeString(&hid_string, L"\\Driver\\MouHID");

    PDRIVER_OBJECT hid_driver_object = NULL;
    status = ObReferenceObjectByName(&hid_string, OBJ_CASE_INSENSITIVE, NULL, 0,
                                     *IoDriverObjectType, KernelMode, NULL,
                                     (PVOID *)&hid_driver_object);
    if (!NT_SUCCESS(status)) {
        if (class_driver_object) {
            ObDereferenceObject(class_driver_object);
        }
        DbgPrintEx(0, 0, "[norsefire]: mouhid ref error");
        return status;
    }

    PVOID class_driver_base = NULL;
    ULONG class_driver_size = 0;

    PDEVICE_OBJECT hid_device_object = hid_driver_object->DeviceObject;
    while (hid_device_object && !mouse_obj->service_callback) {
        PDEVICE_OBJECT class_device_object = class_driver_object->DeviceObject;
        while (class_device_object && !mouse_obj->service_callback) {
            if (!class_device_object->NextDevice && !mouse_obj->mouse_device) {
                mouse_obj->mouse_device = class_device_object;
            }

            PULONG_PTR device_extension =
                (PULONG_PTR)hid_device_object->DeviceExtension;
            ULONG_PTR device_ext_size =
                ((ULONG_PTR)hid_device_object->DeviceObjectExtension -
                 (ULONG_PTR)hid_device_object->DeviceExtension) /
                4;
            class_driver_base = class_driver_object->DriverStart;
            class_driver_size = class_driver_object->DriverSize;
            for (ULONG_PTR i = 0; i < device_ext_size; i++) {
                if (device_extension[i] == (ULONG_PTR)class_device_object &&
                    device_extension[i + 1] > (ULONG_PTR)class_driver_base &&
                    device_extension[i + 1] <
                        (ULONG_PTR)((ULONG_PTR)class_driver_base +
                                    class_driver_size)) {
                    mouse_obj->service_callback =
                        (MouseClassServiceCallback)(device_extension[i + 1]);
                    DbgPrintEx(0, 0,
                               "[norsefire]: Mouse class service callback %p\n",
                               device_extension[i + 1]);
                    break;
                }
            }
            class_device_object = class_device_object->NextDevice;
        }
        hid_device_object = hid_device_object->AttachedDevice;
    }

    if (!mouse_obj->mouse_device) {
        PDEVICE_OBJECT target_device_object = class_driver_object->DeviceObject;
        while (target_device_object) {
            if (!target_device_object->NextDevice) {
                mouse_obj->mouse_device = target_device_object;
                break;
            }
            target_device_object = target_device_object->NextDevice;
        }
    }

    ObDereferenceObject(class_driver_object);
    ObDereferenceObject(hid_driver_object);

    if (mouse_obj->service_callback) {
        DbgPrintEx(0, 0, "[norsefire]: Mouse initialized\n");
        return STATUS_SUCCESS;
    }
    DbgPrintEx(0, 0, "[norsefire]: Mouse initialized fail\n");
    return STATUS_UNSUCCESSFUL;
}

void mouse::keyboard_event(KEYBOARD_OBJECT kb_obj, PNF_KEYBOARD_REQUEST kr) {
    ULONG input_data;
    KIRQL irql;
    KEYBOARD_INPUT_DATA mid = {0};

    mid.ExtraInformation = kr->ExtraInformation;
    mid.Flags = kr->Flags;
    mid.MakeCode = kr->MakeCode;
    mid.Reserved = kr->Reserved;
    mid.UnitId = kr->UnitId;

    KeRaiseIrql(DISPATCH_LEVEL, &irql);
    kb_obj.service_callback(kb_obj.kb_device, &mid,
                            (PKEYBOARD_INPUT_DATA)&mid + 1, &input_data);
    KeLowerIrql(irql);
}

void mouse::mouse_event(MOUSE_OBJECT mouse_obj, long x, long y,
                        unsigned short button_flags, USHORT flags) {
    ULONG input_data;
    KIRQL irql;
    MOUSE_INPUT_DATA mid = {0};

    mid.LastX = x;
    mid.LastY = y;
    mid.Flags = flags | MOUSE_VIRTUAL_DESKTOP | MOUSE_MOVE_ABSOLUTE;
    mid.ButtonFlags = button_flags;

    KeRaiseIrql(DISPATCH_LEVEL, &irql);
    mouse_obj.service_callback(mouse_obj.mouse_device, &mid,
                               (PMOUSE_INPUT_DATA)&mid + 1, &input_data);
    KeLowerIrql(irql);
}
