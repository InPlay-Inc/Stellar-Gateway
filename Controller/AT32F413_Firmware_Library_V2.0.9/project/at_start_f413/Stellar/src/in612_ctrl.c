#include <stdlib.h>
#include <string.h>

#include "at32f413_board.h"
#include "at32f413_clock.h"

typedef struct
{
	gpio_type 					*gpio_base;
	uint16_t 					pin;
	int 						active_level;

} gpio_cfg_t;

const gpio_cfg_t in612_boost_io[4] = {
	{GPIOB,GPIO_PINS_0, 0}, //U1, BOOST_1
	{GPIOC,GPIO_PINS_2, 0}, //U3, BOOST_2
	{GPIOC,GPIO_PINS_9, 0}, //U6, BOOST_3
	{GPIOB,GPIO_PINS_5, 0}, //U8, BOOST_4
};

const gpio_cfg_t in612_rst_io[4] = {
	{GPIOB,GPIO_PINS_1, 0}, //U1, RST_1
	{GPIOC,GPIO_PINS_3, 0}, //U3, RST_2
	{GPIOA,GPIO_PINS_8, 0}, //U6, RST_3
	{GPIOB,GPIO_PINS_6, 0}, //U8, RST_4
};

#define WAKEUP_IN_GPIO_BASE GPIOA
#define WAKEUP_IN_GPIO_PIN	GPIO_PINS_0
#define SLEEP_IND_GPIO_BASE GPIOC
#define SLEEP_IND_GPIO_PIN  GPIO_PINS_4

#define PB8_GPIO_BASE  GPIOB
#define PB8_GPIO_PIN   GPIO_PINS_8
#define PB9_GPIO_BASE  GPIOB
#define PB9_GPIO_PIN   GPIO_PINS_9

//const gpio_cfg_t in612_bootmode_io[4] = {

//	{GPIOA,GPIO_PINS_0, 0}, //WAKEUP_IN, 0-normal mode; 1-bootmode
//	{GPIOC,GPIO_PINS_4, 0}, //SLEEP_IND, 0-normal mode; 1-bootmode

//	{GPIOB,GPIO_PINS_8, 0}, //I2C_SCL
//	{GPIOB,GPIO_PINS_9, 0}, //I2C_SDA

//};

static void io_output(gpio_type *p_base, uint16_t pin, int actv_level, _Bool actv_en)
{
//	typedef void (*gpio_output_func)(gpio_type *gpio_x, uint16_t pins);
//	const gpio_output_func io_out_func[2] = {gpio_bits_set, gpio_bits_reset};
//	const gpio_output_func io_out_revert_func[2] = {gpio_bits_reset, gpio_bits_set};

	if (actv_level == 0) {

		if (actv_en)
			gpio_bits_reset(p_base, pin);
		else
			gpio_bits_set(p_base, pin);
	} else {

		if (actv_en)
			gpio_bits_set(p_base, pin);
		else
			gpio_bits_reset(p_base, pin);
	}
}

static void toggle_rst(int cs)
{
	delay_ms(10);
	io_output(in612_rst_io[cs].gpio_base, in612_rst_io[cs].pin, in612_rst_io[cs].active_level, TRUE);
	delay_ms(100);
	io_output(in612_rst_io[cs].gpio_base, in612_rst_io[cs].pin, in612_rst_io[cs].active_level, FALSE);
}

void in612_enter_boot_mode(int cs)
{
	printf("cs %d enter boot mode\n", cs);

	io_output(in612_boost_io[cs].gpio_base, in612_boost_io[cs].pin, in612_boost_io[cs].active_level, TRUE);
	toggle_rst(cs);
}

void in612_exit_boot_mode(int cs)
{
	printf("cs %d exit boot mode\n", cs);

	io_output(in612_boost_io[cs].gpio_base, in612_boost_io[cs].pin, in612_boost_io[cs].active_level, FALSE);
	toggle_rst(cs);
}

int in612_ctrl_init(void)
{
	int mode = 0;
	gpio_init_type gpio_init_struct;

	for (int i=0;i<4;i++) {

		/* configure the BOOST_x pin */
		gpio_default_para_init(&gpio_init_struct);

		gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
		gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
		gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
		gpio_init_struct.gpio_pins = in612_boost_io[i].pin;
		gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
		gpio_init(in612_boost_io[i].gpio_base, &gpio_init_struct);
		io_output(in612_boost_io[i].gpio_base, in612_boost_io[i].pin, in612_boost_io[i].active_level, FALSE);

		/* configure the RST_x pin */
		gpio_init_struct.gpio_pins = in612_rst_io[i].pin;
		gpio_init(in612_rst_io[i].gpio_base, &gpio_init_struct);
		io_output(in612_rst_io[i].gpio_base, in612_rst_io[i].pin, in612_rst_io[i].active_level, TRUE);
		//toggle_rst(i);
	}

	gpio_default_para_init(&gpio_init_struct);
	gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
	gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;

	//WAKEUP_IN (PA0)
	gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
	gpio_init_struct.gpio_pins = WAKEUP_IN_GPIO_PIN;
	gpio_init_struct.gpio_pull = GPIO_PULL_DOWN;
	gpio_init(WAKEUP_IN_GPIO_BASE, &gpio_init_struct);

	//SLEEP_IND (PC4)
//	gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
//	gpio_init_struct.gpio_pins = SLEEP_IND_GPIO_PIN;
//	gpio_init_struct.gpio_pull = GPIO_PULL_DOWN;
//	gpio_init(SLEEP_IND_GPIO_BASE, &gpio_init_struct);

	//PB8
	gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
	gpio_init_struct.gpio_pins = PB8_GPIO_PIN;
	gpio_init_struct.gpio_pull = GPIO_PULL_UP;
	gpio_init(PB8_GPIO_BASE, &gpio_init_struct);

	//PB9
	gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
	gpio_init_struct.gpio_pins = PB9_GPIO_PIN;
	gpio_init_struct.gpio_pull = GPIO_PULL_UP;
	gpio_init(PB9_GPIO_BASE, &gpio_init_struct);

	mode = gpio_input_data_bit_read(WAKEUP_IN_GPIO_BASE, WAKEUP_IN_GPIO_PIN)<<1;
	//mode |= gpio_input_data_bit_read(SLEEP_IND_GPIO_BASE, SLEEP_IND_GPIO_PIN);

	return mode;
}

void in612_ctrl_reset(int cs)
{
	io_output(in612_boost_io[cs].gpio_base, in612_boost_io[cs].pin, in612_boost_io[cs].active_level, FALSE);
	toggle_rst(cs);
}

int in612_ctrl_get_cs(void)
{
	/*	cs = PB9<<1|PB8 */

	int cs = gpio_input_data_bit_read(PB9_GPIO_BASE, PB9_GPIO_PIN) << 1;
	cs |= gpio_input_data_bit_read(PB8_GPIO_BASE, PB8_GPIO_PIN);
	
	return cs;
}

_Bool in612_ctrl_check_cmd_ready(uint8_t buf[], int len)
{
	int cs = -1;
	uint8_t cmd_ready[5] = {0x7e, 0x01, 0x00, 0x00, 0x7e};

	if (len == sizeof(cmd_ready) && !memcmp(buf, cmd_ready, len))
		return TRUE;

	return FALSE;
}