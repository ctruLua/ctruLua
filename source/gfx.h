#ifndef GFX_H
#define GFX_H

typedef struct {
	GPU_SCISSORMODE mode;
	u32 x; u32 y;
	u32 width; u32 height;
} scissor_state;

extern scissor_state lua_scissor;

#endif
