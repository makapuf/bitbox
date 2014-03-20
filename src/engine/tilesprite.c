/* sprite tiled */
/* data format : RLE tilemap. 
- tilemap RLE uint16 
- ref lines uint16[bglines] 
- tileset raw (65k = 32k = 128 tiles / tileset) : uint8_t nb, uint8_t ref
- ref zero = vide
 */

typedef struct {
	uint16_t **tileset; // array of 16x16 tiles.
	uint16_t tilemap_w, tilemap_h; // size in tiles
	uint8_t tilemap[][2]; // rle encoding of nb, tile_id
	uint8_t semitransp_limit; // index of first semi transparent tile. (<id :  non transp ; >= id : semitransparent)
} Tilesprite;

// structure of ram object : 
//   a : ptr to tilesprite, 
//   b : index courant ds tilemap ; 
//   c : tile number (for xy position)

Tilesprite_frame(object *o, int16_t line)
{
	// advance to start of line
	int block=0; uint8_t n** 
	for (
		int block=0, int o->b=0; 
		block < (line/16)*tilemap_w;
		block += ((Tilesprite *)o->a)->tilemap[o->b][0],o->b++
		);
}

Tilesprite_line(object *o)
{
	// we're right at the start, go to next line
	Tilesprite *t = o->a;
	int end_line_block = o->b + t->tilemap_w; //  NO 
	for (;o->b!=end_line_block;o->b++)
	{
		// zero : skip
		// aggregate all a single run of same opacity or divide in loop of N ? 
		// now let's emit each. optimize later as needed

		if (t->tilemap[o->b][1]==0) // skip
			continue;
		else if (t->tilemap[o->b][1]<t->semitransp_limit)
			pre_draw(o, o->x+16*o->b,o->x+16*o->b+16, OPAQUE);
		else  
			pre_draw(o, o->x+16*o->b,o->x+16*o->b+16, TRANSPARENT);
	}
}