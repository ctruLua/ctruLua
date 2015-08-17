#include <3ds.h>
#include <sf2d.h>
#include <sfil.h>

#include "citra_jpeg.h"
#include "3dbrew_png.h"


int main()
{
	sf2d_init();
	sf2d_set_clear_color(RGBA8(0x40, 0x40, 0x40, 0xFF));

	sf2d_texture *tex1 = sfil_load_JPEG_buffer(citra_jpeg, citra_jpeg_size, SF2D_PLACE_RAM);
	sf2d_texture *tex2 = sfil_load_PNG_buffer(_3dbrew_png, SF2D_PLACE_RAM);

	while (aptMainLoop()) {

		hidScanInput();

		if (hidKeysHeld() & KEY_START) {
			break;
		}

		sf2d_start_frame(GFX_TOP, GFX_LEFT);

			sf2d_draw_texture(tex1, 400/2 - tex1->width/2, 240/2 - tex1->height/2);

		sf2d_end_frame();

		sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);

			sf2d_draw_texture(tex2, 320/2 - tex2->width/2, 240/2 - tex2->height/2);

		sf2d_end_frame();

		sf2d_swapbuffers();
	}

	sf2d_free_texture(tex1);
	sf2d_free_texture(tex2);

	sf2d_fini();
	return 0;
}
