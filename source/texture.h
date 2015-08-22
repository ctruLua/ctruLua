#ifndef TEXTURE_H
#define TEXTURE_H

typedef struct {
	sf2d_texture *texture;
	float scaleX;
	float scaleY;
	u32 blendColor;
} texture_userdata;

#endif
