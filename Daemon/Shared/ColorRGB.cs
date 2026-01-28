namespace Daemon.Shared;

/// <summary>
/// Represents an RGB color with values from 0-255 for each channel.
/// </summary>
/// <param name="r">Red channel intensity (0-255)</param>
/// <param name="g">Green channel intensity (0-255)</param>
/// <param name="b">Blue channel intensity (0-255)</param>
public readonly struct ColorRGB(byte r, byte g, byte b)
{
    public readonly byte R = r;
    public readonly byte G = g;
    public readonly byte B = b;
}