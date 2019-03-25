/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Jin gi KONG <jgkong@kr.ibm.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include <stdio.h>

#include "esp_log.h"

#include "driver/gpio.h"
#include "driver/rmt.h"

#include "py/runtime.h"
#include "py/mphal.h"
#include "modmachine.h"

extern const mp_obj_type_t machine_rmt_type;

typedef struct _esp32_rmt_obj_t {
    mp_obj_base_t base;
    rmt_config_t config;
    uint8_t buf_size;
} esp32_rmt_obj_t;

STATIC mp_obj_t esp32_rmt_make_new(const mp_obj_type_t *type,
        size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);
    esp32_rmt_obj_t *self = m_new_obj(esp32_rmt_obj_t);
    self->base.type = &machine_rmt_type;
    self->buf_size = mp_obj_get_int(args[0]);
    return MP_OBJ_FROM_PTR(self);
}

STATIC void esp32_rmt_print(const mp_print_t *print, mp_obj_t self_in,
        mp_print_kind_t kind) {
    esp32_rmt_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "RMT(Buffer Size: %u)", self->buf_size);
}

STATIC mp_obj_t esp32_rmt_tx_init(mp_obj_t self_in) {
    //esp32_rmt_obj_t *self = self_in;
    rmt_config_t rmt_tx;
    rmt_tx.channel = 1;
    rmt_tx.gpio_num = 16;
    rmt_tx.mem_block_num = 1;
    rmt_tx.clk_div = 100;
    rmt_tx.tx_config.loop_en = false;
    rmt_tx.tx_config.carrier_duty_percent = 50;
    rmt_tx.tx_config.carrier_freq_hz = 38000;
    rmt_tx.tx_config.carrier_level = 1;
    rmt_tx.tx_config.carrier_en = 1;
    rmt_tx.tx_config.idle_level = 0;
    rmt_tx.tx_config.idle_output_en = true;
    rmt_tx.rmt_mode = 0;
    rmt_config(&rmt_tx);
    rmt_driver_install(rmt_tx.channel, 0, 0);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(esp32_rmt_tx_init_obj, esp32_rmt_tx_init);

STATIC mp_obj_t esp32_rmt_tx_send_cmd(mp_obj_t self_in, mp_obj_t cmds_in) {
    //esp32_rmt_obj_t *self = self_in;
    int channel = 1;
    size_t cmd_len;
    mp_obj_t *cmds;
    mp_obj_get_array(cmds_in, &cmd_len, &cmds);

    size_t item_len = cmd_len / 2;
    size_t item_size = (sizeof(rmt_item32_t) * item_len);
    rmt_item32_t* items = (rmt_item32_t*) malloc(item_size);
    for (int i = 0; i < item_len; i++) {
        items[i].level0 = 1;
        items[i].level1 = 0;
        items[i].duration0 = mp_obj_get_int(cmds[i*2]);
        items[i].duration1 = mp_obj_get_int(cmds[i*2+1]);
    }
    rmt_write_items(channel, items, item_len, true);
    rmt_wait_tx_done(channel, portMAX_DELAY);
    free(items);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(esp32_rmt_tx_send_cmd_obj, esp32_rmt_tx_send_cmd);

STATIC const mp_rom_map_elem_t esp32_rmt_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_install), MP_ROM_PTR(&esp32_rmt_tx_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_send), MP_ROM_PTR(&esp32_rmt_tx_send_cmd_obj) },
};

STATIC MP_DEFINE_CONST_DICT(esp32_rmt_locals_dict, esp32_rmt_locals_dict_table);

const mp_obj_type_t machine_rmt_type = {
    { &mp_type_type },
    .name = MP_QSTR_RMT,
    .print = esp32_rmt_print,
    .make_new = esp32_rmt_make_new,
    .locals_dict = (mp_obj_dict_t*)&esp32_rmt_locals_dict,
};
