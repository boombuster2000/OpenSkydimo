using System.IO.Ports;

namespace Daemon.Driver;

public class SkydimoDriver
{
    private const int HeaderSize = 6;
    
    private readonly SerialPort _serialPort;
    private byte[] _ledBuffer;
    private readonly ColorRGB[] _currerntColors;

    public int LedCount { get; }

    public SkydimoDriver(string portName, int ledCount, int baudRate = 115200)
    {
        LedCount = ledCount;
        
        _currerntColors = new ColorRGB[LedCount];
        
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
}