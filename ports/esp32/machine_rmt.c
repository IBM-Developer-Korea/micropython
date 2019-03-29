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

#include "driver/gpio.h"
#include "driver/rmt.h"

#include "py/runtime.h"
#include "py/mphal.h"
#include "modmachine.h"

extern const mp_obj_type_t machine_rmt_type;

typedef struct _esp32_rmt_obj_t {
    mp_obj_base_t base;
    rmt_config_t config;
    size_t buf_size;
} esp32_rmt_obj_t;

STATIC void esp32_rmt_tx_config_helper(esp32_rmt_obj_t *self,
        size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_loop_en, ARG_carrier_freq_hz, ARG_carrier_duty_percent, ARG_carrier_level, ARG_carrier_en, ARG_idle_level, ARG_idle_output_en };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_loop_en, MP_ARG_BOOL, {.u_int = false} },
        { MP_QSTR_carrier_freq_hz, MP_ARG_INT, {.u_int = 38000} },
        { MP_QSTR_carrier_duty_percent, MP_ARG_INT, {.u_int = 50} },
        { MP_QSTR_carrier_level, MP_ARG_INT, {.u_int = RMT_CARRIER_LEVEL_HIGH} },
        { MP_QSTR_carrier_en, MP_ARG_BOOL, {.u_int = true} },
        { MP_QSTR_idle_level, MP_ARG_INT, {.u_int = RMT_IDLE_LEVEL_LOW} },
        { MP_QSTR_idle_output_en, MP_ARG_BOOL, {.u_int = false} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args,
        MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    self->config.tx_config.loop_en = args[ARG_loop_en].u_int;
    self->config.tx_config.carrier_freq_hz = args[ARG_carrier_freq_hz].u_int;
    self->config.tx_config.carrier_duty_percent = args[ARG_carrier_duty_percent].u_int;
    self->config.tx_config.carrier_level = args[ARG_carrier_level].u_int;
    self->config.tx_config.carrier_en = args[ARG_carrier_en].u_int;
    self->config.tx_config.idle_level = args[ARG_idle_level].u_int;
    self->config.tx_config.idle_output_en = args[ARG_idle_output_en].u_int;
    rmt_config(&self->config);
    rmt_driver_install(self->config.channel, 0, 0);
}

STATIC void esp32_rmt_rx_config_helper(esp32_rmt_obj_t *self,
        size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum {ARG_filter_en, ARG_filter_ticks_thresh, ARG_idle_threshold, ARG_rx_buf_size };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_filter_en, MP_ARG_BOOL, {.u_int = false} },
        { MP_QSTR_filter_ticks_thresh, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_idle_threshold, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_rx_buf_size, MP_ARG_INT, {.u_int = 1000} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args,
        MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    self->config.rx_config.filter_en = args[ARG_filter_en].u_int;
    self->config.rx_config.filter_ticks_thresh = args[ARG_filter_ticks_thresh].u_int;
    self->config.rx_config.idle_threshold = args[ARG_idle_threshold].u_int;
    self->buf_size = args[ARG_rx_buf_size].u_int;
    rmt_config(&self->config);
    rmt_driver_install(self->config.channel, self->buf_size, 0);
}

STATIC mp_obj_t esp32_rmt_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_rmt_mode, ARG_channel, ARG_clk_div, ARG_pin, ARG_mem_block_num };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_rmt_mode, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = RMT_MODE_TX} },
        { MP_QSTR_channel, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_clk_div, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 80} },
        { MP_QSTR_pin, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_mem_block_num, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    esp32_rmt_obj_t *self = m_new_obj(esp32_rmt_obj_t);
    self->base.type = &machine_rmt_type;
    self->config.rmt_mode = args[ARG_rmt_mode].u_int;
    self->config.channel = args[ARG_channel].u_int;
    self->config.clk_div = args[ARG_clk_div].u_int;
    self->config.gpio_num = machine_pin_get_id(args[ARG_pin].u_obj);
    self->config.mem_block_num = args[ARG_mem_block_num].u_int;

    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t esp32_rmt_config(size_t n_args,
        const mp_obj_t *args, mp_map_t *kw_args) {
    esp32_rmt_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    if (self->config.rmt_mode == RMT_MODE_TX) {
        esp32_rmt_tx_config_helper(self, n_args - 1, args + 1, kw_args);
    } else {
        esp32_rmt_rx_config_helper(self, n_args - 1, args + 1, kw_args);
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(esp32_rmt_config_obj, 1, esp32_rmt_config);

STATIC void esp32_rmt_print(const mp_print_t *print, mp_obj_t self_in,
        mp_print_kind_t kind) {
    esp32_rmt_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "RMT(Buffer Size: %u)", self->buf_size);
}

STATIC mp_obj_t esp32_rmt_tx_send_cmd(mp_obj_t self_in, mp_obj_t items_in) {
    esp32_rmt_obj_t *self = MP_OBJ_FROM_PTR(self_in);
    int channel = self->config.channel;
    size_t item_len;
    mp_obj_t *items_arr;
    mp_obj_get_array(items_in, &item_len, &items_arr);
    mp_obj_t *val, *pulse;

    size_t item_size = (sizeof(rmt_item32_t) * item_len);
    rmt_item32_t* items = (rmt_item32_t*) malloc(item_size);
    for (int i = 0; i < item_len; i++) {
        mp_obj_get_array_fixed_n(items_arr[i], 2, &val);
        mp_obj_get_array_fixed_n(val[0], 2, &pulse);
        items[i].duration0 = mp_obj_get_int(pulse[0]);
        items[i].level0 = mp_obj_get_int(pulse[1]);
        mp_obj_get_array_fixed_n(val[1], 2, &pulse);
        items[i].duration1 = mp_obj_get_int(pulse[0]);
        items[i].level1 = mp_obj_get_int(pulse[1]);
    }
    rmt_write_items(channel, items, item_len, true);
    rmt_wait_tx_done(channel, portMAX_DELAY);
    free(items);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(esp32_rmt_tx_send_cmd_obj, esp32_rmt_tx_send_cmd);

STATIC mp_obj_t esp32_rmt_receive_cmd(mp_obj_t self_in) {
    esp32_rmt_obj_t *self = MP_OBJ_FROM_PTR(self_in);
    mp_obj_t *list = mp_obj_new_list(0, NULL);
    mp_obj_tuple_t *pulse;
    int channel = self->config.channel;
    RingbufHandle_t rb = NULL;

    rmt_get_ringbuf_handle(channel, &rb);
    // rmt_rx_start(rmt_channel_tchannel, bool rx_idx_rst)
    rmt_rx_start(channel, true);

    while(rb) {
        size_t rx_size = 0;
        rmt_item32_t* items = (rmt_item32_t*) xRingbufferReceive(rb, &rx_size, self->buf_size);
        if(items) {
            for (int i = 0; i < rx_size / 4; i++) {
                rmt_item32_t item = items[i];
                mp_obj_tuple_t *val = mp_obj_new_tuple(2, NULL);
                pulse = mp_obj_new_tuple(2, NULL);
                pulse->items[0] = mp_obj_new_int(item.duration0);
                pulse->items[1] = mp_obj_new_int(item.level0);
                val->items[0] = pulse;
                pulse = mp_obj_new_tuple(2, NULL);
                pulse->items[0] = mp_obj_new_int(item.duration1);
                pulse->items[1] = mp_obj_new_int(item.level1);
                val->items[1] = pulse;
                mp_obj_list_append(list, val);

            }

            //after parsing the data, return spaces to ringbuffer.
            vRingbufferReturnItem(rb, (void*) items);
            rmt_rx_stop(channel);

            return list;
        } else {
            break;
        }
    }
    rmt_rx_stop(channel);

    return list;
}
MP_DEFINE_CONST_FUN_OBJ_1(esp32_rmt_receive_cmd_obj, esp32_rmt_receive_cmd);

STATIC const mp_rom_map_elem_t esp32_rmt_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_config), MP_ROM_PTR(&esp32_rmt_config_obj) },
    { MP_ROM_QSTR(MP_QSTR_send), MP_ROM_PTR(&esp32_rmt_tx_send_cmd_obj) },
    { MP_ROM_QSTR(MP_QSTR_receive), MP_ROM_PTR(&esp32_rmt_receive_cmd_obj) },

    { MP_ROM_QSTR(MP_QSTR_RX), MP_ROM_INT(RMT_MODE_RX) },
    { MP_ROM_QSTR(MP_QSTR_TX), MP_ROM_INT(RMT_MODE_TX) },
};

STATIC MP_DEFINE_CONST_DICT(esp32_rmt_locals_dict, esp32_rmt_locals_dict_table);

const mp_obj_type_t machine_rmt_type = {
    { &mp_type_type },
    .name = MP_QSTR_RMT,
    .print = esp32_rmt_print,
    .make_new = esp32_rmt_make_new,
    .locals_dict = (mp_obj_dict_t*)&esp32_rmt_locals_dict,
};
