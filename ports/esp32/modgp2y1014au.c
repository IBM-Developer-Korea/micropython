#include <stdio.h>

#include "esp_log.h"

#include "driver/gpio.h"
#include "driver/adc.h"

#include "py/runtime.h"
#include "py/mphal.h"
#include "modmachine.h"

#include "modgp2y1014au.h"

// this is the actual C-structure for our new object
typedef struct _gp2y1014au_GP2Y1014AU_obj_t {
    // base represents some basic information, like type
    mp_obj_base_t base;
    // a member created by us
    gpio_num_t gpio_id;
    adc1_channel_t vo_id;
    // 
    gpio_num_t iled_id;
    mp_float_t voc;
    mp_float_t k;
} gp2y1014au_GP2Y1014AU_obj_t;


STATIC gp2y1014au_GP2Y1014AU_obj_t gp2y1014au_obj[] = {
    {{&gp2y1014au_GP2Y1014AU_type}, GPIO_NUM_36, ADC1_CHANNEL_0},
    {{&gp2y1014au_GP2Y1014AU_type}, GPIO_NUM_37, ADC1_CHANNEL_1},
    {{&gp2y1014au_GP2Y1014AU_type}, GPIO_NUM_38, ADC1_CHANNEL_2},
    {{&gp2y1014au_GP2Y1014AU_type}, GPIO_NUM_39, ADC1_CHANNEL_3},
    {{&gp2y1014au_GP2Y1014AU_type}, GPIO_NUM_32, ADC1_CHANNEL_4},
    {{&gp2y1014au_GP2Y1014AU_type}, GPIO_NUM_33, ADC1_CHANNEL_5},
    {{&gp2y1014au_GP2Y1014AU_type}, GPIO_NUM_34, ADC1_CHANNEL_6},
    {{&gp2y1014au_GP2Y1014AU_type}, GPIO_NUM_35, ADC1_CHANNEL_7},
};

// ILEDPin, VoPin, Resolution=4096
STATIC mp_obj_t gp2y1014au_GP2Y1014AU_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw,
        const mp_obj_t *args) {

    // ILEDPin, VoPin, Resolution=4096
    mp_arg_check_num(n_args, n_kw, 2, 3, true);

    // Create 
    // gp2y1014au_GP2Y1014AU_obj_t *self = m_new_obj(gp2y1014au_GP2Y1014AU_obj_t);

    // VoPin
    gpio_num_t vo_id = machine_pin_get_id(args[1]);
    gp2y1014au_GP2Y1014AU_obj_t *self = NULL;
    for (int i = 0; i < MP_ARRAY_SIZE(gp2y1014au_obj); i++) {
        if (vo_id == gp2y1014au_obj[i].gpio_id) { self = &gp2y1014au_obj[i]; break; }
    }
    if (!self->base.type) mp_raise_ValueError("invalid Pin for ADC");

    // ILEDPin
    self->iled_id = machine_pin_get_id(args[0]);
    gpio_set_direction(self->iled_id, GPIO_MODE_DEF_OUTPUT);
    

    // Set 11db
    esp_err_t err = adc1_config_channel_atten(self->vo_id, ADC_ATTEN_11db);
    if (err == ESP_OK) return MP_OBJ_FROM_PTR(self);
    mp_raise_ValueError("Parameter Error");
}

STATIC void gp2y1014au_GP2Y1014AU_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    gp2y1014au_GP2Y1014AU_obj_t *self = self_in;
    mp_printf(print, "gp2y1014au Vo(Pin(%u))", self->gpio_id);
}

// def readRawVo(self, samplingTime=280, deltaTime=40):
STATIC mp_obj_t gp2y1014au_read_raw_vo(mp_obj_t self_in, mp_obj_t samplingTime, mp_obj_t deltaTime) {
    gp2y1014au_GP2Y1014AU_obj_t *self = self_in;

    static int initialized = 0;
    if (!initialized) {
        adc1_config_width(ADC_WIDTH_12Bit);
        initialized = 1;
    }

    // Turn on ILED
    gpio_set_level(self->iled_id, 0);

    // Wait 0.28ms 
    mp_int_t stime = mp_obj_get_int(samplingTime);
    if (stime > 0) {
        mp_hal_delay_us(stime);
    }

    // Read
    int val = adc1_get_raw(self->vo_id);
    if (val == -1) mp_raise_ValueError("Parameter Error");

    // Wait 0.04ms
    mp_int_t dtime = mp_obj_get_int(deltaTime);
    if (dtime > 0) {
        mp_hal_delay_us(dtime);
    }

    // Turn off ILED
    gpio_set_level(self->iled_id, 1);

    return MP_OBJ_NEW_SMALL_INT(val);
}
MP_DEFINE_CONST_FUN_OBJ_3(gp2y1014au_read_raw_vo_obj, gp2y1014au_read_raw_vo);

// def 
STATIC mp_obj_t gp2y1014au_on(mp_obj_t self_in) {
    gp2y1014au_GP2Y1014AU_obj_t *self = self_in;

    // Turn on LED
    gpio_set_level(GPIO_NUM_17, 1);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(gp2y1014au_on_obj, gp2y1014au_on);

STATIC mp_obj_t gp2y1014au_off(mp_obj_t self_in) {
    gp2y1014au_GP2Y1014AU_obj_t *self = self_in;

    // Turn off LED
    gpio_set_level(GPIO_NUM_17, 0);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(gp2y1014au_off_obj, gp2y1014au_off);


STATIC mp_obj_t gp2y1014au_k(mp_obj_t self_in, mp_obj_t k_in) {
    gp2y1014au_GP2Y1014AU_obj_t *self = self_in;

    self->k = mp_obj_get_float(k_in);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(gp2y1014au_k_obj, gp2y1014au_k);

STATIC mp_obj_t gp2y1014au_voc(mp_obj_t self_in, mp_obj_t voc_in) {
    gp2y1014au_GP2Y1014AU_obj_t *self = self_in;

    self->voc = mp_obj_get_float(voc_in);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(gp2y1014au_voc_obj, gp2y1014au_voc);


///
STATIC const mp_rom_map_elem_t gp2y1014au_GP2Y1014AU_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_readRawVo), MP_ROM_PTR(&gp2y1014au_read_raw_vo_obj) },
    { MP_ROM_QSTR(MP_QSTR_on), MP_ROM_PTR(&gp2y1014au_on_obj) },
    { MP_ROM_QSTR(MP_QSTR_off), MP_ROM_PTR(&gp2y1014au_off_obj) }
    
};

STATIC MP_DEFINE_CONST_DICT(gp2y1014au_GP2Y1014AU_locals_dict, gp2y1014au_GP2Y1014AU_locals_dict_table);

// create the class-object itself
const mp_obj_type_t gp2y1014au_GP2Y1014AU_type = {
    // "inherit" the type "type"
    { &mp_type_type },
    // give it a name
    .name = MP_QSTR_GP2Y1014AU,
    // give it a print-function
    .print = gp2y1014au_GP2Y1014AU_print,
    // give it a constructor
    .make_new = gp2y1014au_GP2Y1014AU_make_new,
    // and the global members
    .locals_dict = (mp_obj_t)&gp2y1014au_GP2Y1014AU_locals_dict,
};

// Global Module
STATIC const mp_rom_map_elem_t gp2y1014au_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),            MP_ROM_QSTR(MP_QSTR_gp2y1014au) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_GP2Y1014AU),      MP_ROM_PTR(&gp2y1014au_GP2Y1014AU_type) },
};

STATIC MP_DEFINE_CONST_DICT(
    mp_module_gp2y1014au_globals,
    gp2y1014au_globals_table
);

const mp_obj_module_t mp_gp2y1014au_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_gp2y1014au_globals,
};
