// How to use:
// * call ply_init to reset the song at the start
// * call ply_update in your game_frame (or graph_frame) to run the player
//
// Currently you can use only one song. Soon you will be able to use more...
// To stop playing, stop calling ply_update and reset the oscillators to an
// invalid waveform yourself.
void ply_init();
void ply_update();
