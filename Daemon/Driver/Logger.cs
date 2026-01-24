namespace Daemon.Driver;

public class Logger
{
    public enum LogLevel
    {
        Info,
        Warning,
        Error,
    }

    private readonly Lock _lockObj = new();
    
    private readonly Dictionary<LogLevel, bool> _isLogLevelEnabled = new()
    {
        { LogLevel.Info, true },
        {LogLevel.Warning, true },
        { LogLevel.Error, true },
    };
    
    private readonly Dictionary<LogLevel, ConsoleColor> _logLevelColors = new()
    {
        { LogLevel.Info, ConsoleColor.Green },
        { LogLevel.Warning, ConsoleColor.Yellow },
        { LogLevel.Error, ConsoleColor.Red },
    };
    
    private static string Timestamp => DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss");

    public void SetLogLevel(LogLevel level, bool enabled)
    {
        lock (_lockObj)
        {
            _isLogLevelEnabled[level] = enabled;
        }
    }

    public void Log(LogLevel level, string message)
    {
        lock (_lockObj)
        {
            if (!_isLogLevelEnabled[level]) return;
            var originalColor = Console.ForegroundColor;
            
            // Timestamp in gray
            Console.ForegroundColor = ConsoleColor.Gray;
            Console.Write($"[{Timestamp}] ");
            
            // Log level in its specific color
            Console.ForegroundColor = _logLevelColors[level];
            Console.Write($"[{level.ToString().ToUpper()}] ");
            
            // Message in default color
            Console.ForegroundColor = originalColor;
            Console.WriteLine($"- {message}");
            
            Console.ForegroundColor = originalColor;
        }
    }

    
    public void Info(string message) => Log(LogLevel.Info, message);
    public void Warning(string message) => Log(LogLevel.Warning, message);
    public void Error(string message) => Log(LogLevel.Error, message);
    public void Error(string message, Exception ex) => 
        Log(LogLevel.Error, $"{message} - Exception: {ex.Message}\n{ex.StackTrace}");
}