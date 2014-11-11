// test simple by default
#include <string.h>
#include "simple.h"

const char *lorem = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec ut malesuada tellus. Curabitur commodo eleifend suscipit. Nunc sit amet mauris ut purus lobortis luctus. Pellentesque sit amet dolor quam. Sed venenatis laoreet tortor. Curabitur volutpat molestie ligula, vel consequat dolor elementum ut. Nullam vel neque eu ex molestie sagittis eu at mauris.\n"
  "In nec porta est. Sed bibendum aliquet orci. Maecenas elit ligula, imperdiet venenatis sollicitudin eu, sollicitudin et eros. Duis suscipit erat lacus, vel dignissim mi suscipit sed. Nunc congue enim quam, finibus gravida dui tempus nec. Nam sed facilisis diam. Nunc euismod justo orci, sed blandit augue mollis vitae. Vivamus vel tincidunt mi. Aliquam non diam at risus convallis mattis. Proin ac molestie nisi. Integer vehicula magna eget porta gravida. Aliquam rutrum elementum ipsum ac pellentesque. Nulla in rhoncus diam. Suspendisse et interdum elit.\n"
  "Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Nullam vestibulum nunc quam, eu ullamcorper leo euismod ut. Proin leo urna, viverra sit amet nibh sed, rhoncus malesuada quam. Vestibulum consequat nunc ac libero posuere porta. Proin scelerisque quam urna, at rutrum ligula placerat id. Vivamus commodo maximus odio, ut tempus est lacinia nec. Donec posuere, diam eu iaculis varius, ex massa gravida nisl, non commodo ligula ligula at ante. Mauris at lacinia urna. Morbi ullamcorper blandit ante ultricies tincidunt. Suspendisse potenti. Nam tincidunt nisl id turpis molestie, sit amet pellentesque ante hendrerit. Proin ac velit quis metus semper iaculis quis nec turpis. Interdum et malesuada fames ac ante ipsum primis in faucibus. Phasellus luctus tellus in rhoncus ultricies.\n"
  "Quisque eu dui leo. Nam tempus nec sem quis placerat. In ornare elementum posuere. Curabitur congue dapibus diam. Aliquam gravida nibh consectetur elit porttitor, nec rutrum neque pretium. Nam venenatis, leo in sagittis fermentum, augue dolor aliquet sem, id commodo nulla lacus sed elit. Morbi pulvinar ac diam ultricies facilisis. Nulla feugiat odio non tempus congue. Duis tempor commodo ligula vitae elementum. Sed pellentesque lorem semper, semper enim sed, volutpat urna. Donec orci leo, sodales ut tristique at, vulputate at elit. Vivamus eleifend enim at dolor bibendum, quis hendrerit nulla viverra. In eu nunc eget mauris lacinia placerat. \n";

void game_init() {
    //  draw ascii set
    for (int i=0;i<256;i++) {
        vram[(i/16)*SCREEN_W+i%16]=i;
    }

    // print text (crudely, no \n or \t interpreting)
    strcpy(&vram[18*SCREEN_W],lorem);
    strcpy(&vram[5*SCREEN_W+60],"Hello Bitbox simple text !");

}


void game_frame() {
}
