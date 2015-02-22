/** Initialize the player to play the given song.
 * - Oscillators are reset to silence
 * - Song position is reset to 0
 * - Tracks and instruments are loaded from the song.
 *
 * Pass NULL to stop playing and not load a new song.
 */
void ply_init(const uint8_t songlen, const unsigned char* song);

/** Update the player status. This must be called once per frame as long as the
 * song is running.
 */
void ply_update(); // Call this once per frame.
