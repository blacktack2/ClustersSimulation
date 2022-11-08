#include "main.h"
#include "view/WindowHandler.h"
#include "view/Logger.h"

#include <cstdio>

int main([[maybe_unused]] int argc, [[maybe_unused]] char* args[]) {
    if (!Logger::getLogger().isValid())
        return -1;
    Logger::getLogger().logMessage("Begin execution");
    {
        WindowHandler windowHandler;
        windowHandler.setSize(800, 600);

        if (windowHandler.init()) {
            windowHandler.mainloop();
        } else {
            Logger::getLogger().logError("Failed to initialize window handler");
            Logger::getLogger().logMessage("End execution");
            return -1;
        }
    }
    Logger::getLogger().logMessage("End execution");

    return 0;
}
