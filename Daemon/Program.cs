using Daemon.Commands;
using Daemon.Driver;

namespace Daemon;

internal static class Program
{
    private static void Main(string[] args)
    {
        var commandsListener = new Listener();
        
        commandsListener.Start();
        
        Console.ReadLine();
        
    }
}