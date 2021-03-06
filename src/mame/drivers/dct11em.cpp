// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        DEC DCT11-EM

        03/12/2010 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/t11/t11.h"
#include "emupal.h"
#include "screen.h"


class dct11em_state : public driver_device
{
public:
	dct11em_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu") { }
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_dct11em(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	void dct11em(machine_config &config);
	void dct11em_mem(address_map &map);
};

void dct11em_state::dct11em_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).ram();  // RAM
	map(0x2000, 0x2fff).ram();  // Optional RAM
	map(0xa000, 0xdfff).rom();  // RAM
}

/* Input ports */
static INPUT_PORTS_START( dct11em )
INPUT_PORTS_END


void dct11em_state::machine_reset()
{
}

void dct11em_state::video_start()
{
}

uint32_t dct11em_state::screen_update_dct11em(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


MACHINE_CONFIG_START(dct11em_state::dct11em)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu",T11, 7500000) // 7.5MHz XTAL
	MCFG_T11_INITIAL_MODE(0x1403)  /* according to specs */
	MCFG_DEVICE_PROGRAM_MAP(dct11em_mem)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(dct11em_state, screen_update_dct11em)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_MONOCHROME("palette")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( dct11em )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	// Highest address line inverted
	ROM_LOAD16_BYTE( "23-213e4.bin", 0x8000, 0x2000, CRC(bdd82f39) SHA1(347deeff77596b67eee27a39a9c40075fcf5c10d))
	ROM_LOAD16_BYTE( "23-214e4.bin", 0x8001, 0x2000, CRC(b523dae8) SHA1(cd1a64a2bce9730f7a9177d391663919c7f56073))
	ROM_COPY("maincpu", 0x8000, 0xc000, 0x2000)
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY                          FULLNAME    FLAGS */
COMP( 1983, dct11em, 0,      0,      dct11em, dct11em, dct11em_state, empty_init, "Digital Equipment Corporation", "DCT11-EM", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
