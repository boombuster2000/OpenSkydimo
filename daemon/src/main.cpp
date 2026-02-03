#include "CommandsListener.h"

#include <SkydimoDriver.h>

int main()
{
    SkydimoDriver driver("/dev/ttyUSB0", 60);
    CommandsListener listener("/tmp/openskydimo.sock", driver);

    driver.OpenSerialConnection();
    driver.Fill(ColorRGB(255, 255, 255));

    listener.Start();
    while (!listener.ShouldStop())
    {
        driver.SendColors();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}