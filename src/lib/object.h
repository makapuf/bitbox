#pragma once

#include <stdint.h>
#include "kernel.h"

extern int screen_width;


// forwards
struct Object;

// FLASH DATA

// Object -----------------------------------------------------------------------------------------------


/*
 "objects" are used on the whole game for :
 - level enemies (standard enemies)
 - static enemies (towers) shooting at the player
 - static, destroyable elements on the level
 - undestroyable obstacles
 - bonus elements (static, no damage, brings a special gift or money) - if bonus encapsulated in something to shoot, encapsulate enemies
 - capsule elements (damage if touched, killed instantly, spawns bonus if die)
 - bonus : kill on touch, no damage if touched, bonus elements

    (All of those are initialized from the level data as the player progresses in the level with subobjects descriptors)

 - enemies bullets (auto-aiming, static direction or homing missile)
 Bosses are special
*/



struct Object; // forward

// object types : flash storage mostly  (Can/will be inherited with composition.)
// could however be in RAM
typedef struct ObjectType {
    char *name; // for debug

    int16_t dx,dy; // offset from parent layer for display
    int16_t zlevel; // higher = top. impacts rendering order. subs must be higher.
     // XXX IN Subobject ?? in Object ? should not change within object life.
    int16_t hitbox[4];

    uint16_t tilesize; // 8 or 16, square (always 8?)
    uint16_t tilemap_width, tilemap_height; // dimensions of a frame

    uint16_t nframes;
    uint16_t *frames; // ptr to tilemaps
    void (*draw)(struct Object*); // draw itself
    // draw virtual func ?
} ObjectType;

// live Object (in ram)
typedef struct Object {
    // -- Common attributes
    const ObjectType *type; // generic pointer. To be cast back to real pointer type. pointer to subelement ?

    unsigned int start_tick; // tick when brought to life
    int last_updated;  // last updated frame. Allows not to update N times if several children.

    int x,y; // updated by tick().
    int anim_frame; // XXX ptr to first tile id to run to

    struct Object *parent; // subobject of X (to RAM data)

    struct Object *displaylist_next; // next object to display (used as display list). NULL if last

    // -- Display runtime specific

    // -- Behaviour / Game specific
} Object;


//int object_create(const SubObject *sub,Object* parent, bool keep_sub);
// find an empty space, insert enemy object in it. returns new id or MAX_ENEMY if no more room.
// also create subobjects.

// bullets or other friendly objects are created in another list (simpler, no Z ; no subs)

void object_disable(Object *);
void object_disable_children(Object *);

void object_blit16(Object *);
void object_blit16_opaque(Object *);
void object_blit16_bg(Object *o); // fixed size, no transp tile

void object_blit8(Object *);
void object_blit8_opaque(Object *o);


/** Tests that an object does collide or not on any object on this zlevel. returns the first to collide*/

void displaylist_insert(Object *o);
void displaylist_remove(Object *o);
/**
 clean display list of dead objects, all at once.
*/
void displaylist_clean();
void displaylist_draw(); // draws whole scene. never called by game, always by kernel.
Object * displaylist_collide(const Object *o,int z_start,int z_end); // zmin,zmax, included
void displaylist_print();
// used for scanning through displaylist
extern Object *displaylist_top;



#define TILESIZE16 16
inline int min(int a, int b)
{
    return a<b ? a:b;
}


inline int max(int a, int b)
{
    return a>b ? a:b;
}
