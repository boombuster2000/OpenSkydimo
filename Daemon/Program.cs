using Daemon.Driver;

namespace Daemon;

class Program
{
    static void Main(string[] args)
    {
        var skydimoDriver = new SkydimoDriver("/dev/ttyUSB0", 60);
        skydimoDriver.OpenConnection();
        
    }
}