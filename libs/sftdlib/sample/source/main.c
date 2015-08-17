#include <stdio.h>
#include <string.h>
#include <math.h>
#include <3ds.h>
#include <sf2d.h>
#include <sftd.h>
#include "airstrike_ttf.h"

int main()
{
	sf2d_init();
	sf2d_set_clear_color(RGBA8(0x40, 0x40, 0x40, 0xFF));

	consoleInit(GFX_BOTTOM, NULL);
	printf("sftd sample\n");

	// Font loading
	sftd_init();
	sftd_font *font = sftd_load_font_mem(airstrike_ttf, airstrike_ttf_size);

	while (aptMainLoop()) {

		hidScanInput();
		if (hidKeysDown() & KEY_START) break;

		sf2d_start_frame(GFX_TOP, GFX_LEFT);

			sftd_draw_text(font, 10, 10, RGBA8(255, 0, 0, 255), 20, "Font drawing on the top screen!");
			sftd_draw_textf(font, 10, 40, RGBA8(0, 255, 0, 255), 20, "FPS %f", sf2d_get_fps());

		sf2d_end_frame();

		sf2d_swapbuffers();
	}

	sftd_free_font(font);
	sftd_fini();

	sf2d_fini();
	return 0;
}
