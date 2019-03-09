#include <stdio.h>
#include <string.h>

#include "py/obj.h"
#include "py/runtime.h"

STATIC mp_obj_t jgkong_hello(void) {
    char *msg = "Hello World!";
    return mp_obj_new_str(msg, strlen(msg));
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(jgkong_hello_obj, jgkong_hello);

STATIC const mp_rom_map_elem_t jgkong_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_jgkong) },

    { MP_ROM_QSTR(MP_QSTR_hello), MP_ROM_PTR(&jgkong_hello_obj) },
};


STATIC MP_DEFINE_CONST_DICT(jgkong_module_globals, jgkong_module_globals_table);

const mp_obj_module_t jgkong_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&jgkong_module_globals,
};
