#ifndef DEDICATED_ONLY

#include "gusanos/blitters/blitters.h"
#include "gusanos/blitters/colors.h"
#include "gusanos/blitters/macros.h"

#include <algorithm>

namespace Blitters
{
	
void drawSprite_multsec_32_with_8(ALLEGRO_BITMAP* where, ALLEGRO_BITMAP* from, ALLEGRO_BITMAP* secondary, int x, int y, int sx, int sy, int cutl, int cutt, int cutr, int cutb)
{
	typedef Pixel32 pixel_t_1;
	typedef Pixel8 pixel_t_sec;
	
	if(bitmap_color_depth(from) != 32
	|| bitmap_color_depth(secondary) != 8)
		return;

	CLIP_SPRITE_REGION_SEC();
	
	SPRITE_Y_LOOP_SEC(
		SPRITE_X_LOOP_SEC(
			*dest = scaleColor_32(*src, *sec);
		)
	)

}

void drawSprite_mult_8_to_32(ALLEGRO_BITMAP* where, ALLEGRO_BITMAP* from, int x, int y, int cutl, int cutt, int cutr, int cutb)
{
	typedef Pixel32 pixel_t_dest;
	typedef Pixel8 pixel_t_src;
	
	if(bitmap_color_depth(where) != 32
	|| bitmap_color_depth(from) != 8)
		return;

	CLIP_SPRITE_REGION();
	
	SPRITE_Y_LOOP(
		SPRITE_X_LOOP_T(
			*dest = scaleColor_32(*dest, *src);
		)
	)

}

void drawSprite_mult_8_to_16(ALLEGRO_BITMAP* where, ALLEGRO_BITMAP* from, int x, int y, int cutl, int cutt, int cutr, int cutb)
{
	typedef Pixel16 pixel_t_dest;
	typedef Pixel8 pixel_t_src;
	
	if(bitmap_color_depth(where) != 16
	|| bitmap_color_depth(from) != 8)
		return;

	CLIP_SPRITE_REGION();
	
	SPRITE_Y_LOOP(
		SPRITE_X_LOOP_T(
			*dest = (pixel_t_dest)scaleColor_16(*dest, *src / 8);
		)
	)
}

} // namespace Blitters

#endif
