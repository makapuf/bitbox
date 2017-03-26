#pragma once
/* this file describes a ROM map : 
   - the positions taken by objects on the map , 
   - the sprites animations

   it mostly describes what will be produced by the export script tmx2.py

   */

#include <stdint.h>

struct StateDef {
    uint8_t nb_frames;
    uint8_t *frames;
    // hitbox
    uint8_t x1,y1,x2,y2;
};

struct SpriteDef {
    const void * file; 
    const struct StateDef *states; // must use a pointer : will not be able to initialize flexible array member with compound literal
};

struct MapObjectDef {
    int16_t x,y;
    const struct SpriteDef * sprite; // id of spritedef in mapdef->sprites
    uint8_t state_id;
    uint8_t type,group; 
};

struct MapDef {
	const void * tileset; // pointer to source data
  const uint16_t *terrains; // four 4bit terrain_id , from top-left to bottom-right, one per tile
	const void * tilemap; 
	unsigned tilemap_w:12;
	unsigned tilemap_h:12; // in tiles
	uint8_t tilesize; 

	int nb_objects;
	const struct MapObjectDef objects[];
};

