using Daemon.Driver;

namespace Daemon;

internal static class Program
{
    private static void Main(string[] args)
    {
        var skydimoDriver = new SkydimoDriver("/dev/ttyUSB0", 60);
        skydimoDriver.OpenConnection();
        
        
        
        skydimoDriver.Dispose();
    }
}