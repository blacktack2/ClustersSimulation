#include "main.h"
#include "view/WindowHandler.h"

#include <cstdio>

int main([[maybe_unused]] int argc, [[maybe_unused]] char* args[]) {
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
