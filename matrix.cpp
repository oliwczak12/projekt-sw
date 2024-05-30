
#include "../Pico-74HC595/src/shift_register_74hc595.h"
#include "hardware/adc.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include <stdio.h>

#define ROW 5
#define COL 10

int CLK_PIN = 11;
int DATA_PIN = 12;
int LATCH_PIN = 10;

struct shift_register_74hc595_t_chain *myreg = new_shreg_74hc595(CLK_PIN, DATA_PIN, LATCH_PIN, 2);

queue_t matrix_queue;

bool matrix[ROW][COL] = {0};

bool matrix_SG[ROW][COL] = {
    {0, 1, 1, 1, 0, 1, 1, 1, 1, 0},
    {0, 1, 0, 0, 0, 1, 0, 0, 0, 0},
    {0, 1, 1, 1, 0, 1, 0, 1, 1, 0},
    {0, 0, 0, 1, 0, 1, 0, 0, 1, 0},
    {0, 1, 0, 1, 0, 1, 1, 1, 1, 0}};

void setup() {
    stdio_init_all();
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);

    gpio_init(21);
    gpio_set_dir(21, GPIO_IN);

    gpio_init(CLK_PIN);
    gpio_set_dir(CLK_PIN, GPIO_OUT);
    gpio_init(DATA_PIN);
    gpio_set_dir(DATA_PIN, GPIO_OUT);
    gpio_init(LATCH_PIN);
    gpio_set_dir(LATCH_PIN, GPIO_OUT);

    adc_init();
    adc_gpio_init(26);
}

void core1() {
    bool matrix_buffer[ROW][COL] = {0};
    while (true) {
        if (multicore_fifo_rvalid()) {
            if (multicore_fifo_pop_blocking() == 0xFFFFFFFF) {
                for (size_t i = 0; i < ROW; i++) {
                    for (size_t j = 0; j < COL; j++) {
                        matrix_buffer[i][j] = matrix[i][j];
                    }
                }
            }
        }
        if (multicore_fifo_wready()) {
            multicore_fifo_push_blocking(0xFFFFFFFF);
        }
        for (size_t i = 0; i < ROW; i++) {
            shreg_74hc595_put(myreg, i + COL, 1);
            for (size_t j = 0; j < COL; j++) {
                if (matrix_buffer[i][j]) {
                    shreg_74hc595_put(myreg, j, 1);
                }
            }
            sleep_us(900);
            for (size_t j = 0; j < COL; j++) {
                shreg_74hc595_put(myreg, j, 0);
            }
            shreg_74hc595_put(myreg, i + COL, 0);
        }
    }
}

void draw(int x, int y, bool data) {
    matrix[x][y] = data;
    if (multicore_fifo_wready()) {
        multicore_fifo_push_blocking(0xFFFFFFFF);
    }
}
void draw(uint32_t matrix_to_pass[][COL]) {

    if (multicore_fifo_wready()) {
        multicore_fifo_push_blocking(0xFFFFFFFF);
    }
}

void snake() {
}

void game_of_life() {
}

int button_right_counter = 0;
void select_game() {
    static bool last_state = false;
    static uint32_t last_time = 0;
    bool current_state = gpio_get(21);

    if (current_state != last_state) {
        if (current_state) {
            button_right_counter++;
            if (button_right_counter > 1) {
                button_right_counter = 0;
            }
        }
    }
    last_state = current_state;
}

void (*func_ptr)() = nullptr;

void update() {
    if (func_ptr == nullptr) {
        func_ptr = select_game;
    }

    if (button_right_counter == 1) {
        gpio_put(25, 1);
    } else if (button_right_counter == 0) {
        gpio_put(25, 0);
    }

    func_ptr();
}

void loop() {
    while (true) {
        if (multicore_fifo_rvalid()) {
            multicore_fifo_pop_blocking();
            update();
        }
    }
}

int main() {
    setup();
    // multicore_launch_core1(draw);
    multicore_launch_core1(core1);
    loop();
}
