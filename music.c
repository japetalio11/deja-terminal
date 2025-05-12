#include <stdlib.h>

int main() {
    // For WAV files
    //system("aplay music.wav");

    // For MP3 files (if mpg123 is installed)
    system("mpg123 bg-music.mp3");

    return 0;
}
