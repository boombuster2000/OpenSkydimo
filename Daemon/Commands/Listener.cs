using System.IO.Pipes;
using System.Text;
using Daemon.Driver;
using Daemon.Shared;

namespace Daemon.Commands;

public class Listener
{
    private readonly Logger _logger;
    private readonly CancellationTokenSource _cancellationTokenSource;
    private readonly Thread _listenerThread;

    private readonly SkydimoDriver _skydimoDriver = new("/dev/ttyUSB0", 60);

    public Listener()
    {
        _logger = new Logger("Listener");
        _cancellationTokenSource = new CancellationTokenSource();
        
        _listenerThread = new Thread(ListenForConnections)
        {
            IsBackground = true,
            Name = "SkydimoListenerThread"
        };
        
        
        if (!_skydimoDriver.OpenConnection())
            return;
            
        
        _skydimoDriver.StartUpdateLoop();
    }

    public void Start()
    {
        _listenerThread.Start();
    }

    private async void ListenForConnections()
    {
        while (!_cancellationTokenSource.Token.IsCancellationRequested)
        {
            try
            {
                // Create a new server instance for each connection
                var server = new NamedPipeServerStream(
                    "SkydimoDaemon",
                    PipeDirection.InOut,
                    NamedPipeServerStream.MaxAllowedServerInstances,
                    PipeTransmissionMode.Byte,
                    PipeOptions.Asynchronous);

                _logger.Info("Waiting for client connection...");
                await server.WaitForConnectionAsync(_cancellationTokenSource.Token);
                _logger.Info("Client connected!");

                // Handle each client on a separate thread
                var clientThread = new Thread(() => HandleClient(server))
                {
                    IsBackground = true,
                    Name = $"SkydimoClientThread-{Guid.NewGuid()}"
                };
                clientThread.Start();
            }
            catch (OperationCanceledException)
            {
                _logger.Info("Listener stopped");
                break;
            }
            catch (Exception e)
            {
                _logger.Error("Failed to wait for connection", e);
                await Task.Delay(1000); // Prevent tight loop on error
            }
        }
    }

    private void HandleClient(NamedPipeServerStream server)
    {
        try
        {
            using (server)
            using (var reader = new StreamReader(server, Encoding.UTF8))
            using (var writer = new StreamWriter(server, Encoding.UTF8))
            {
                writer.AutoFlush = true;
                while (server.IsConnected && !_cancellationTokenSource.Token.IsCancellationRequested)
                {
                    var message = reader.ReadLine();
                    
                    if (message == null)
                        break; // Client disconnected
                    
                    _logger.Info($"Received command: {message}");
                    
                    // Process the command
                    var response = ProcessCommand(message);
                    
                    // Send response back to client
                    writer.WriteLine(response);
                }
            }
            
            _logger.Info("Client disconnected");
        }
        catch (IOException ex)
        {
            _logger.Error("Client connection error", ex);
        }
        catch (Exception ex)
        {
            _logger.Error("Error handling client", ex);
        }
    }

    private string ProcessCommand(string command)
    {
        var parts = command.Split(' ', StringSplitOptions.RemoveEmptyEntries);
    
        if (parts.Length == 0)
            return "ERROR: Empty command";
    
        var action = parts[0].ToLower();
    
        return action switch
        {
            "fill" when parts.Length >= 2 => HandleFillCommand(parts[1]),
            "status" => "Running",
            "version" => "1.0.0",
            _ => $"ERROR: Unknown command '{action}'"
        };
    }

    private string HandleFillCommand(string colorArg)
    {
        try
        {
            var rgb = colorArg.Split(',');
            if (rgb.Length != 3)
                return "ERROR: Color format should be r,g,b (e.g., 255,0,0)";
        
            var r = byte.Parse(rgb[0]);
            var g = byte.Parse(rgb[1]);
            var b = byte.Parse(rgb[2]);
            ColorRGB color =  new(r, g, b);
        
            // TODO: Apply color to LEDs
            _logger.Info($"Filling LEDs with RGB({r},{g},{b})");

            if (!_skydimoDriver.Fill(color))
                return $"ERROR: Failed to fill with RGB({r},{g},{b})";
            
        
            return $"OK: Filled with RGB({r},{g},{b})";
        }
        catch (Exception ex)
        {
            return $"ERROR: Invalid color format - {ex.Message}";
        }
    }

    public void Stop()
    {
        _cancellationTokenSource.Cancel();
    }
}