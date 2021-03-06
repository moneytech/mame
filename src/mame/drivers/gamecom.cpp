// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Robbbert
/***************************************************************************

Driver file to handle emulation of the Tiger Game.com by Wilbert Pol.
Various improvements by Robbbert.

Todo:
- Fix cpu and system problems that prevent the games from working fully.
- RS232 port
- Sound ports 1,2 do not sound anything like the real thing
- Sound port 3 (noise channel)
- Sound dac port (mostly works but is the wrong speed in some places)

Game Status:
- Inbuilt ROM and PDA functions all work
- On the screen where the cart goes into the slot, there are vertical bands of randomness
- Due to an irritating message, the NVRAM is commented out in the machine config
- All carts appear to work except:
- - Henry: crash just after "HENRY" button clicked
- - Lost World: freeze just after entering Stage 2 (the nest)
- Weblink and Internet are of no use as there is nothing to connect to.

***************************************************************************/

#include "emu.h"
#include "includes/gamecom.h"

#include "sound/volt_reg.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

#include "gamecom.lh"


void gamecom_state::gamecom_mem_map(address_map &map)
{
	map(0x0000, 0x0013).ram().region("maincpu", 0x00);
	map(0x0014, 0x0017).rw(FUNC(gamecom_state::gamecom_pio_r), FUNC(gamecom_state::gamecom_pio_w));        // buttons
	map(0x0018, 0x001F).ram().region("maincpu", 0x18);
	map(0x0020, 0x007F).rw(FUNC(gamecom_state::gamecom_internal_r), FUNC(gamecom_state::gamecom_internal_w));/* CPU internal register file */
	map(0x0080, 0x03FF).ram().region("maincpu", 0x80);                     /* RAM */
	map(0x0400, 0x0FFF).noprw();                                                /* Nothing */
	map(0x1000, 0x1FFF).rom();                                                /* Internal ROM (initially), or External ROM/Flash. Controlled by MMU0 (never swapped out in game.com) */
	map(0x2000, 0x3FFF).bankr("bank1");                                   /* External ROM/Flash. Controlled by MMU1 */
	map(0x4000, 0x5FFF).bankr("bank2");                                   /* External ROM/Flash. Controlled by MMU2 */
	map(0x6000, 0x7FFF).bankr("bank3");                                   /* External ROM/Flash. Controlled by MMU3 */
	map(0x8000, 0x9FFF).bankr("bank4");                                   /* External ROM/Flash. Controlled by MMU4 */
	map(0xA000, 0xDFFF).ram().share("videoram");             /* VRAM */
	map(0xE000, 0xFFFF).ram().share("nvram");           /* Extended I/O, Extended RAM */
}

static INPUT_PORTS_START( gamecom )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME( "Up" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME( "Down" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME( "Left" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME( "Right" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME( "Menu" ) PORT_CODE( KEYCODE_M )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME( DEF_STR(Pause) ) PORT_CODE( KEYCODE_V )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME( "Sound" ) PORT_CODE( KEYCODE_S )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Button A" ) PORT_CODE( KEYCODE_A ) PORT_CODE( KEYCODE_LCONTROL )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Button B" ) PORT_CODE( KEYCODE_B ) PORT_CODE( KEYCODE_LALT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "Button C" ) PORT_CODE( KEYCODE_C ) PORT_CODE( KEYCODE_SPACE )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME( "Reset" ) PORT_CODE( KEYCODE_N )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME( "Button D" ) PORT_CODE( KEYCODE_D ) PORT_CODE( KEYCODE_LSHIFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME( "Stylus press" ) PORT_CODE( KEYCODE_Z ) PORT_CODE( MOUSECODE_BUTTON1 )

	PORT_START("GRID.0")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_OTHER)

	PORT_START("GRID.1")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_OTHER)

	PORT_START("GRID.2")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_OTHER)

	PORT_START("GRID.3")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_OTHER)

	PORT_START("GRID.4")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_OTHER)

	PORT_START("GRID.5")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_OTHER)

	PORT_START("GRID.6")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_OTHER)

	PORT_START("GRID.7")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_OTHER)

	PORT_START("GRID.8")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_OTHER)

	PORT_START("GRID.9")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_OTHER)

	PORT_START("GRID.10")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_OTHER)

	PORT_START("GRID.11")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_OTHER)

	PORT_START("GRID.12")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_OTHER)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_OTHER)
	INPUT_PORTS_END

PALETTE_INIT_MEMBER(gamecom_state, gamecom)
{
	palette.set_pen_color(0, 0x00, 0x00, 0x00 ); // Black
	palette.set_pen_color(1, 0x0F, 0x4F, 0x2F ); // Gray 1
	palette.set_pen_color(2, 0x6F, 0x8F, 0x4F ); // Gray 2
	palette.set_pen_color(3, 0x8F, 0xCF, 0x8F ); // Grey 3
	palette.set_pen_color(4, 0xDF, 0xFF, 0x8F ); // White
}

uint32_t gamecom_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

INTERRUPT_GEN_MEMBER(gamecom_state::gamecom_interrupt)
{
	m_maincpu->set_input_line(sm8500_cpu_device::LCDC_INT, ASSERT_LINE );
}

MACHINE_CONFIG_START(gamecom_state::gamecom)
	/* basic machine hardware */
	MCFG_DEVICE_ADD( "maincpu", SM8500, XTAL(11'059'200)/2 )   /* actually it's an sm8521 microcontroller containing an sm8500 cpu */
	MCFG_DEVICE_PROGRAM_MAP( gamecom_mem_map)
	MCFG_SM8500_DMA_CB( WRITE8( *this, gamecom_state, gamecom_handle_dma ) )
	MCFG_SM8500_TIMER_CB( WRITE8( *this, gamecom_state, gamecom_update_timers ) )
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", gamecom_state,  gamecom_interrupt)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	//MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE( 59.732155 )
	MCFG_SCREEN_VBLANK_TIME(500)
	MCFG_SCREEN_UPDATE_DRIVER(gamecom_state, screen_update)
	MCFG_SCREEN_SIZE( 200, 160 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 199, 0, 159 )
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEFAULT_LAYOUT(layout_gamecom)
	MCFG_PALETTE_ADD("palette", 5)
	MCFG_PALETTE_INIT_OWNER(gamecom_state, gamecom)

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	/* TODO: much more complex than this */
	MCFG_DEVICE_ADD("dac", DAC_8BIT_R2R, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.5) // unknown DAC (Digital audio)
	MCFG_DEVICE_ADD("dac0", DAC_4BIT_R2R, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.05) // unknown DAC (Frequency modulation)
	MCFG_DEVICE_ADD("dac1", DAC_4BIT_R2R, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.05) // unknown DAC (Frequency modulation)
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE(0, "dac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE(0, "dac", -1.0, DAC_VREF_NEG_INPUT)
	MCFG_SOUND_ROUTE(0, "dac0", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE(0, "dac0", -1.0, DAC_VREF_NEG_INPUT)
	MCFG_SOUND_ROUTE(0, "dac1", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE(0, "dac1", -1.0, DAC_VREF_NEG_INPUT)

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot1", generic_linear_slot, "gamecom_cart")
	MCFG_GENERIC_EXTENSIONS("bin,tgc")
	MCFG_GENERIC_LOAD(gamecom_state, gamecom_cart1)

	MCFG_GENERIC_CARTSLOT_ADD("cartslot2", generic_linear_slot, "gamecom_cart")
	MCFG_GENERIC_EXTENSIONS("bin,tgc")
	MCFG_GENERIC_LOAD(gamecom_state, gamecom_cart2)

	MCFG_SOFTWARE_LIST_ADD("cart_list","gamecom")
MACHINE_CONFIG_END

ROM_START( gamecom )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "internal.bin", 0x1000,  0x1000, CRC(a0cec361) SHA1(03368237e8fed4a8724f3b4a1596cf4b17c96d33) )

	ROM_REGION( 0x40000, "kernel", 0 )
	ROM_LOAD( "external.bin", 0x00000, 0x40000, CRC(e235a589) SHA1(97f782e72d738f4d7b861363266bf46b438d9b50) )
ROM_END

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT          COMPANY  FULLNAME    FLAGS
CONS( 1997, gamecom, 0,      0,      gamecom, gamecom, gamecom_state, init_gamecom, "Tiger", "Game.com", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
