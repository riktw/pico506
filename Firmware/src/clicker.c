// Copyright (c) Kuba Szczodrzyński 2025-9-17.

#include "pico506.h"

static void clicker_main();

void clicker_start(pico506_t *pico) {
	multicore_reset_core1();
	multicore_fifo_push_blocking_inline((uint32_t)pico);
	multicore_launch_core1(clicker_main);
}

void clicker_enqueue(uint count) {
	multicore_fifo_push_timeout_us(count, 0);
}

__attribute__((noreturn)) static void clicker_main() {
	pico506_t *pico = (void *)multicore_fifo_pop_blocking_inline();

	while (1) {
		uint count = multicore_fifo_pop_blocking_inline();
		srand(to_us_since_boot(get_absolute_time()) * count);
		clicker_seek(pico, count);
	}
}

void clicker_seek(pico506_t *pico, uint count) {
	uint click_cnt;
	if (count == CYL_INVALID)
		// do only one click in case of write or head change
		click_cnt = 1;
	else
		click_cnt = randrange(2, 4 + count / 50);

	for (uint i = 0; i < click_cnt; i++) {
		uint freq	= randrange(1500, 1800); // Hz
		uint micros = randrange(500, 3000);	 // us
		clicker_pulse(pico, freq, micros);
		sleep_ms(randrange(5, 25));
	}
}

void clicker_pulse(pico506_t *pico, uint freq, uint micros) {
	gpio_set_function(PIN_BUZZER, GPIO_FUNC_PWM);
	uint slice_num = pwm_gpio_to_slice_num(PIN_BUZZER);

	float clkdiv  = 20.0f;
	float clock	  = (float)clock_get_hz(clk_sys) / clkdiv;
	uint16_t wrap = (uint16_t)(clock / (float)freq);

	pwm_set_wrap(slice_num, wrap);
	pwm_set_clkdiv(slice_num, clkdiv);
	pwm_set_chan_level(slice_num, pwm_gpio_to_channel(PIN_BUZZER), wrap / 2); // ~50% duty
	pwm_set_enabled(slice_num, true);

	sleep_us(micros);

	pwm_set_chan_level(slice_num, pwm_gpio_to_channel(PIN_BUZZER), 0);
	pwm_set_enabled(slice_num, false);
	gpio_set_function(PIN_BUZZER, GPIO_FUNC_NULL);
}
