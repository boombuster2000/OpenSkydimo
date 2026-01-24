using Daemon.Driver;

namespace Daemon;

internal static class Program
{
    private static void Main(string[] args)
    {
        using var skydimoDriver = new SkydimoDriver("/dev/ttyUSB0", 60);
        
        if (!skydimoDriver.OpenConnection())
            return;
        
        skydimoDriver.StartUpdateLoop();
        
        skydimoDriver.Fill(new ColorRGB(255,0, 255));
        
        
        Console.ReadLine();
        
    }
}