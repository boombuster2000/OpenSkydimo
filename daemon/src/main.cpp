#include <SkydimoDriver.h>

int main()
{
    SkydimoDriver driver("/dev/ttyUSB0", 60);

    driver.OpenSerialConnection();

    driver.Fill(ColorRGB(255, 255, 255));
    driver.SendColors();
}