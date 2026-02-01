using System.IO.Pipes;
using System.Text;

namespace Client;

internal static class Program
{
    private static async Task<int> Main(string[] args)
    {
        if (args.Length == 0)
        {
            ShowUsage();
            return 1;
        }

        try
        {
            // Join all arguments into a single command string
            var command = string.Join(" ", args);
            
            Console.WriteLine($"Sending command: {command}");
            
            var response = await SendCommandAsync(command);
            
            Console.WriteLine($"Response: {response}");
            return 0;
        }
        catch (OperationCanceledException)
        {
            await Console.Error.WriteLineAsync("Error: Could not connect to Skydimo daemon. Is it running?");
            return 2;
        }
        catch (Exception ex)
        {
            await Console.Error.WriteLineAsync($"Error: {ex.Message}");
            return 3;
        }
    }

    private static async Task<string> SendCommandAsync(string command, int timeoutMs = 5000)
    {
        await using var client = new NamedPipeClientStream(
            ".",
            "SkydimoDaemon",
            PipeDirection.InOut,
            PipeOptions.Asynchronous);
    
        // Connect with timeout
        using var cts = new CancellationTokenSource(timeoutMs);
        await client.ConnectAsync(cts.Token);
        
        await using var writer = new StreamWriter(client, Encoding.UTF8, leaveOpen: true);
        writer.AutoFlush = true;
        using var reader = new StreamReader(client, Encoding.UTF8, leaveOpen: true);
    
        // Send command
        await writer.WriteLineAsync(command);
    
        // Flush to ensure the command is sent
        await writer.FlushAsync(cts.Token);
            
        // Read response
        var response = await reader.ReadLineAsync(cts.Token);
        return response ?? "No response";
    }

    private static void ShowUsage()
    {
        Console.WriteLine("Skydimo LED Controller Client");
        Console.WriteLine();
        Console.WriteLine("Usage: skydimo <command> [arguments]");
        Console.WriteLine();
        Console.WriteLine("Commands:");
        Console.WriteLine("  fill <r,g,b>       Fill all LEDs with color (e.g., fill 255,0,0)");
        Console.WriteLine("  status             Get daemon status");
        Console.WriteLine("  version            Get daemon version");
        Console.WriteLine();
        Console.WriteLine("Examples:");
        Console.WriteLine("  skydimo fill 255,0,0");
        Console.WriteLine("  skydimo status");
    }
}