#include "main.h"
#include "WindowHandler.h"

#include <cstdio>
#include <fstream>


int main(int argc, char* args[]) {
    {
        WindowHandler windowHandler;
        windowHandler.setSize(800, 600);

        if (windowHandler.init()) {
            windowHandler.mainloop();
        } else {
            fprintf(stderr, "Failed to create window handler!");
            return 1;
        }
    }

    return 0;
}
