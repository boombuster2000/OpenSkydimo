using System.IO.Ports;
using System.Net.Http.Headers;

namespace Daemon.Driver;

public class SkydimoDriver : IDisposable
{
    private const int HeaderSize = 6;
    private readonly Logger _logger;
    
    private readonly SerialPort _serialPort;
    private readonly byte[] _ledBuffer;
    private readonly ColorRGB[] _currentColors;
    private readonly Lock _lock = new Lock();
    
    private Thread? _updateThread;
    private volatile bool _isRunning;

    private int LedCount { get; }

    public SkydimoDriver(string portName, int ledCount, int baudRate = 115200)
    {
        if (string.IsNullOrWhiteSpace(portName))
            throw new ArgumentNullException(nameof(portName), "Port name must be provided.");
        
        if  (ledCount is <= 0 or > 255)
            throw new ArgumentOutOfRangeException(nameof(ledCount), "LedCount must be between 1 and 255.");
        
        LedCount = ledCount;
        _logger = new Logger();

        _currentColors = new ColorRGB[LedCount];
        
        // Each LED requires 3 bytes (r,g,b)
        var bufferSize = HeaderSize + (LedCount * 3);
        _ledBuffer = new byte[bufferSize];
        
        _serialPort = new SerialPort(portName, baudRate)
        {
            DataBits = 8,
            Parity = Parity.None,
            StopBits = StopBits.One,
            Handshake = Handshake.None,
            ReadTimeout = 1000,
            WriteTimeout = 1000
        };
        
        AddHeaderToBuffer();
    }

    private void AddHeaderToBuffer()
    {
        // "Ada" - Magic identifier word
        _ledBuffer[0] = (byte)'A';
        _ledBuffer[1] = (byte)'d';
        _ledBuffer[2] = (byte)'a';
        
        _ledBuffer[3] = 0;
        _ledBuffer[4] = 0;
        
        _ledBuffer[5] = (byte)(Math.Min(LedCount, 255)); // Max 255 LEDs
    }
    
    public bool OpenConnection()
    {
        try
        {
            if (_serialPort.IsOpen)
            {
                _logger.Info($"Serial port {_serialPort.PortName} is already open");
                return true;
            }
        
            _logger.Info($"Opening serial port {_serialPort.PortName} (Baud: {_serialPort.BaudRate}, LEDs: {LedCount})");
            _serialPort.Open();
            _logger.Info($"Successfully opened serial port {_serialPort.PortName}");
        
            return true;
        }
        catch (UnauthorizedAccessException ex)
        {
            _logger.Error($"Access denied to serial port {_serialPort.PortName}. Port may be in use by another application", ex);
            return false;
        }
        catch (IOException ex)
        {
            _logger.Error($"I/O error opening serial port {_serialPort.PortName}. Check if device is connected", ex);
            return false;
        }
        catch (Exception ex)
        {
            _logger.Error($"Unexpected error opening serial port {_serialPort.PortName}", ex);
            return false;
        }
    }

    public void StartUpdateLoop()
    {
        if (_isRunning)
        {
            _logger.Warning("Update loop is already running");
            return;
        }

        _isRunning = true;
        _updateThread = new Thread(LedUpdateLoop)
        {
            IsBackground = true,
            Name = "SkydimoUpdateThread"
        };
        _updateThread.Start();
        
        _logger.Info("Started LED update loop");
    }

    private void StopUpdateLoop()
    {
        if (!_isRunning) return;

        _isRunning = false;
        _updateThread?.Join();
        _logger.Info("Stopped LED update loop");
    }

    private void LedUpdateLoop()
    {
        _logger.Info("LED update loop started");


        while (_isRunning)
        {
            if (_serialPort.IsOpen)
                SendCurrentColors();


            Thread.Sleep(100);
        }


        _logger.Info("LED update loop stopped");
    }

    private bool SendCurrentColors()
    {
        lock (_lock)
        {
            try
            {
                if (!_serialPort.IsOpen)
                {
                    _logger.Error($"Cannot update LEDs: Serial port {_serialPort.PortName} is not open");
                    return false;
                }

                _serialPort.Write(_ledBuffer, 0, _ledBuffer.Length);
                return true;
            }
            catch (Exception ex)
            {
                _logger.Error($"Failed to send LED data to {_serialPort.PortName}: {ex.Message}");
                return false;
            }
        }
    }

    private bool SetLedColors(ColorRGB[] colors)
    {
        if (colors.Length != LedCount)
        {
            _logger.Error($"LED count mismatch. Expected {LedCount}, got {colors.Length}");
            return false;
        }

        lock (_lock)
        {
            Array.Copy(colors, _currentColors, LedCount);
        
            var offset = HeaderSize;
            for (var i = 0; i < colors.Length; i++)
            {
                _ledBuffer[offset++] = colors[i].R;
                _ledBuffer[offset++] = colors[i].G;
                _ledBuffer[offset++] = colors[i].B;
            }
        }

        return true;
    }

    public bool Fill(ColorRGB color)
    {
        var colors = new ColorRGB[LedCount];
        
        for (var i = 0; i < LedCount; i++)
            colors[i] = color;
        
        return SetLedColors(colors);
    }
    
    public void Dispose()
    {
        StopUpdateLoop();
        
        if (_serialPort.IsOpen)
        {
            _logger.Info($"Closing serial port {_serialPort.PortName}");
            _serialPort.Close();
            _logger.Info($"Successfully closed serial port {_serialPort.PortName}");
        }
        
        _serialPort.Dispose();
        GC.SuppressFinalize(this);
    }
}