#include "CommandsListener.h"

#include <SkydimoDriver.h>

int main()
{
    SkydimoDriver driver;
    CommandsListener listener("/tmp/openskydimo.sock", driver);

    listener.Start();
    while (!listener.ShouldStop())
    {
        if (driver.IsReadyToSend())
            driver.SendColors();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}