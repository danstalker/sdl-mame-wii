#include "driver.h"
#include "sound/okim6295.h"
#include "includes/toaplan2.h"

/****************************************************************************
  The Toaplan 2 hardware with V25+ secondary CPU controls the sound through
  to a YM2151 and OKI M6295 on some boards. Here we just interperet some of
  commands sent to the V25+, directly onto the OKI M6295

  These tables convert commands sent from the main CPU, into sample numbers
  played back by the sound processor.
  The ADPCM ROMs contain intrument samples which are sequenced by the
  sound processor to create some of the backing tracks. This is beyond the
  scope of this playback file. Time would be better spent elsewhere.
****************************************************************************/


static const UINT8 batsugun_cmd_snd[64] =
{
/* Sound Command 13 (0x0d) is a megamix of OKI sound effects */
/* Sound Command 20 (0x14) repeats the initial crash part of the sample 4 times */
/*00*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*08*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*10*/  0x00, 0x00, 0x00, 0x12, 0x12, 0x10, 0x0e, 0x0f,
/*18*/  0x0d, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00,
/*20*/  0x00, 0x00, 0x00, 0x13, 0x14, 0x17, 0x15, 0x16,
/*28*/  0x18, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*30*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1a,
/*38*/  0x0e, 0x0f, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const UINT8 kbash_cmd_snd[128] =
{
/*00*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*08*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*10*/  0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
/*18*/  0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21,
/*20*/  0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
/*28*/  0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31,
/*30*/  0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
/*38*/  0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41,
/*40*/  0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
/*48*/  0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51,
/*50*/  0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
/*58*/  0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61,
/*60*/  0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
/*68*/  0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x00,
/*70*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*78*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const UINT8 fixeight_cmd_snd[128] =
{
/* Some sound commands are mixed with tones produced by the FM chip */
/* Probably 96(60H), 82(52H), 80(50H) and 70(46H) and maybe others */
/*00*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*08*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*10*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*18*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*20*/  0x18, 0x3e, 0x37, 0x48, 0x38, 0x49, 0x4a, 0x4b,
/*28*/  0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x53, 0x54, 0x51,
/*30*/  0x52, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00,
/*38*/  0x07, 0x08, 0x00, 0x1b, 0x00, 0x43, 0x00, 0x00,
/*40*/  0x00, 0x47, 0x00, 0x3a, 0x44, 0x0a, 0x0a, 0x06,
/*48*/  0x3c, 0x46, 0x3f, 0x45, 0x00, 0x02, 0x04, 0x10,
/*50*/  0x0f, 0x11, 0x09, 0x0d, 0x0c, 0x0b, 0x00, 0x00,
/*58*/  0x00, 0x15, 0x3d, 0x3f, 0x1e, 0x1c, 0x19, 0x13,
/*60*/  0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*68*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*70*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/*78*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static void play_oki_sound(const device_config *device, int game_sound, int data)
{
	int status = okim6295_r(device,0);

	logerror("Playing sample %02x from command %02x\n",game_sound,data);

	if (game_sound != 0)
	{
		if ((status & 0x01) == 0) {
			okim6295_w(device,0,(0x80 | game_sound));
			okim6295_w(device,0,0x11);
		}
		else if ((status & 0x02) == 0) {
			okim6295_w(device,0,(0x80 | game_sound));
			okim6295_w(device,0,0x21);
		}
		else if ((status & 0x04) == 0) {
			okim6295_w(device,0,(0x80 | game_sound));
			okim6295_w(device,0,0x41);
		}
		else if ((status & 0x08) == 0) {
			okim6295_w(device,0,(0x80 | game_sound));
			okim6295_w(device,0,0x81);
		}
	}
}

void dogyuun_okisnd_w(const device_config *device, int data)
{
	/* Need a board to determine the sound commands */
//  popmessage("Writing %04x to Sound CPU",data);
}

void kbash_okisnd_w(const device_config *device, int data)
{
//  popmessage("Writing %04x to Sound CPU",data);

	if (data == 0)
	{
		okim6295_w(device,0,0x78);		/* Stop playing effects */
	}
	else if ((data > 0) && (data < 128))
	{
		play_oki_sound(device, kbash_cmd_snd[data], data);
	}
}

void fixeight_okisnd_w(const device_config *device, int data)
{
//  popmessage("Writing %04x to Sound CPU",data);

	if (data == 0)
	{
		okim6295_w(device,0,0x78);		/* Stop playing effects */
	}
	else if ((data > 0) && (data < 128))
	{
		play_oki_sound(device, fixeight_cmd_snd[data], data);
	}
}

void batsugun_okisnd_w(const device_config *device, int data)
{
//  popmessage("Writing %04x to Sound CPU",data);

	if (data == 0)
	{
		okim6295_w(device,0,0x78);		/* Stop playing effects */
	}
	else if ((data > 0) && (data < 64))
	{
		play_oki_sound(device, batsugun_cmd_snd[data], data);
	}
}
