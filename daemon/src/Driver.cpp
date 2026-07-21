#include "Driver.h"

#include <fcntl.h>
#include <format>
#include <termios.h>
#include <unistd.h>

Driver::~Driver()
{
    StopAndCleanup();
}

void Driver::LoadConfigAndStart()
{
    try
    {
        m_configHandler.Load();
    }
    catch (const std::runtime_error& e)
    {
        m_logger->error("Failed to load config: {}", e.what());
        return;
    }

    std::string port;
    int ledCount = 0;

    try
    {
        port = m_configHandler.config.value("port", "");
    }
    catch (const nlohmann::json::exception& e)
    {
        m_logger->error("Config file contains invalid port value: {}", e.what());
        return;
    }

    try
    {
        ledCount = m_configHandler.config.value("led-count", 0);
    }
    catch (const nlohmann::json::exception& e)
    {
        m_logger->error("Config file contains invalid led-count value: {}", e.what());
        return;
    }

    if (port.empty())
    {
        m_logger->info("No port configured yet; skipping auto-connect.");
        return;
    }
    if (auto [code, message] = SetSerialPort(port); code != 0)
    {
        m_logger->error("{}", message);
        return;
    }

    if (ledCount <= 0)
    {
        m_logger->info("No LED count configured yet; skipping auto-connect.");
        return;
    }

    if (auto [code, message] = SetLedCount(ledCount); code != 0)
    {
        m_logger->error("{}", message);
        return;
    }

    if (auto [code, message] = OpenSerialConnection(); code != 0)
    {
        m_logger->error("{}", message);
        return;
    }

    if (!m_configHandler.config.contains("last-effect") || m_configHandler.config["last-effect"].is_null())
    {
        m_logger->info("No previous effect to restore.");
        return;
    }

    const auto& lastEffect = m_configHandler.config["last-effect"];
    try
    {
        const Effect type = lastEffect.at("type").get<Effect>();
        const auto params = lastEffect.value("params", nlohmann::json{});
        if (auto [applyCode, applyMessage] = ApplyEffect(type, params, false); applyCode != 0)
            m_logger->warn("Failed to restore last effect on startup: {}", applyMessage);
    }
    catch (const nlohmann::json::exception& e)
    {
        m_logger->warn("Skipping malformed lastEffect on startup: {}", e.what());
    }
}

Response Driver::ApplyEffect(const Effect effect, const nlohmann::json& params, const bool saveToFile)
{
    Response result;

    switch (effect)
    {
    case Effect::FILL: {
        try
        {
            result = Fill(params.get<ColorRGB>());
        }
        catch (const nlohmann::json::exception& e)
        {
            m_logger->warn("Invalid parameters for FILL effect: {}", e.what());
            return MakeWarning(2, "Invalid effect parameters.");
        }

        break;
    }
    default:
        m_logger->warn("Received unknown effect.");
        return MakeWarning(2, "Received unknown effect.");
    }

    if (result.code == 0)
    {
        m_configHandler.config["last-effect"] = {{"type", effect}, {"params", params}};

        try
        {
            if (saveToFile)
            {
                m_configHandler.Save();
                m_logger->info("Updated last effect in config file.");
            }
        }
        catch (const std::runtime_error& e)
        {
            m_logger->warn("Effect applied but failed to persist to config: {}", e.what());
            return MakeWarning(2, "Effect applied but failed to persist to config.");
        }
    }

    return result;
}

void Driver::StopAndCleanup()
{
    m_isConnectionOpened = false;

    if (m_sendThread.joinable())
        m_sendThread.join();

    if (m_serialPort >= 0)
    {
        close(m_serialPort);
        m_serialPort = -1;
    }
}

std::optional<Response> Driver::RequireStopped(const char* action) const
{
    if (m_isConnectionOpened)
    {
        m_logger->warn("Tried to {} whilst connection is opened.", action);
        return MakeWarning(2, "connection must be closed first, run openskydimo stop");
    }

    return std::nullopt;
}

Response Driver::SetRefreshRate(const int hz)
{
    if (auto response = RequireStopped("set refresh rate"))
        return *response;

    if (hz <= 0)
        return MakeError(1, "refresh rate must be > 0Hz");

    if (hz >= 1'000'000)
        return MakeError(1, "refresh rate must be less than 1,000,000Hz.");

    m_logger->info("Setting refresh rate to {} Hz", hz);
    m_sendInterval = std::chrono::microseconds(1'000'000 / hz);

    return MakeOk();
}

Response Driver::SetSerialPort(const std::string& portName)
{
    if (auto response = RequireStopped("set serial port"))
        return *response;

    m_logger->info("Setting serial port to '{}'", portName);
    m_portName = portName;
    m_configHandler.config["port"] = portName;

    try
    {
        m_configHandler.Save();
        m_logger->info("Updated port in config file.");
    }
    catch (const std::runtime_error& e)
    {
        m_logger->warn("Port set but failed to persist to config: {}", e.what());
    }

    return MakeOk();
}

Response Driver::SetBaudRate(const int baudRate)
{
    if (auto response = RequireStopped("set baud rate"))
        return *response;

    m_logger->info("Setting baud rate to {}", baudRate);
    m_baudRate = baudRate;
    return MakeOk();
}

Response Driver::SetLedCount(const int ledCount)
{
    if (auto response = RequireStopped("set LED count"))
        return *response;

    if (ledCount < 0 || ledCount > 1024)
    {
        m_logger->warn("Tried to set LED count to {}", ledCount);
        return MakeError(1, std::format("LED count must be between 0 and 1024, got {}", ledCount));
    }

    m_logger->info("Setting LED count to {}", ledCount);
    m_ledCount = ledCount;
    m_configHandler.config["led-count"] = ledCount;

    try
    {
        m_configHandler.Save();
        m_logger->info("Updated led-count in config file.");
    }
    catch (const std::runtime_error& e)
    {
        m_logger->warn("led-count set but failed to persist to config: {}", e.what());
    }

    AddHeaderToBuffer();
    return MakeOk();
}

Response Driver::OpenSerialConnection()
{
    if (auto response = RequireStopped("open serial connection"))
        return *response;

    StopAndCleanup();

    m_logger->info("Opening serial connection on port '{}'", m_portName);

    if (m_portName.empty())
        return MakeError(1, "no serial port set", "openskydimo set port <path>");

    if (m_ledCount == 0)
        return MakeError(1, "LED count not set", "openskydimo set count <n>");

    m_serialPort = open(m_portName.c_str(), O_RDWR | O_NOCTTY);
    if (m_serialPort < 0)
        return MakeError(1, std::format("unable to open serial port '{}'", m_portName));

    m_logger->debug("Serial port '{}' opened, configuring tty attributes", m_portName);

    termios tty{};
    if (tcgetattr(m_serialPort, &tty) != 0)
    {
        close(m_serialPort);
        m_serialPort = -1;
        return MakeError(1, "unable to get tty attributes");
    }

    // Configure basic settings
    tty.c_cflag &= ~PARENB; // No parity
    tty.c_cflag &= ~CSTOPB; // 1 stop bit

    tty.c_cflag &= ~CSIZE; // First clear the data-bits set
    tty.c_cflag |= CS8;    // 8 data bits (DataBits = 8)

    tty.c_cflag &= ~CRTSCTS;       // No hardware flow control (Handshake.None)
    tty.c_cflag |= CREAD | CLOCAL; // Enable receiver, ignore modem control lines

    // Configure input flags
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // No software flow control
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
    tty.c_oflag &= ~OPOST;
    tty.c_oflag &= ~ONLCR;
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHONL | ISIG);

    speed_t baudRate;
    switch (m_baudRate)
    {
    case 9600:
        baudRate = B9600;
        break;
    case 19200:
        baudRate = B19200;
        break;
    case 38400:
        baudRate = B38400;
        break;
    case 57600:
        baudRate = B57600;
        break;
    case 115200:
        baudRate = B115200;
        break;
    case 230400:
        baudRate = B230400;
        break;
    default:
        close(m_serialPort);
        m_serialPort = -1;
        return MakeError(1, std::format("unsupported baud rate: {}", m_baudRate));
    }

    cfsetispeed(&tty, baudRate);
    cfsetospeed(&tty, baudRate);
    m_logger->debug("Baud rate set to {}", m_baudRate);

    if (tcsetattr(m_serialPort, TCSANOW, &tty) != 0)
    {
        close(m_serialPort);
        m_serialPort = -1;
        return MakeError(1, "unable to get tty attributes");
    }

    m_logger->info("Serial connection established on '{}' at {} baud, ready to send to {} LEDs", m_portName, m_baudRate,
                   m_ledCount);

    m_isConnectionOpened = true;
    m_sendThread = std::thread(&Driver::SendLoop, this);
    m_logger->info("Send thread started at ~{} Hz", 1'000'000 / m_sendInterval.count());

    return MakeOk();
}

Response Driver::CloseSerialConnection()
{
    if (m_serialPort < 0)
    {
        m_logger->warn("CloseSerialConnection called but no connection is open");
        return MakeWarning(2, "no serial connection is open to close");
    }

    m_logger->info("Closing serial connection on '{}'", m_portName);
    StopAndCleanup();
    m_logger->info("Serial connection closed");

    return MakeOk();
}

void Driver::SendLoop()
{
    using clock = std::chrono::steady_clock;

    while (m_isConnectionOpened)
    {
        const auto next = clock::now() + m_sendInterval;

        try
        {
            std::lock_guard lock(m_bufferMutex);
            SendColors();
        }
        catch (const SerialWriteException& e)
        {
            m_logger->error("Send loop write error: {}", e.what());
            m_isConnectionOpened = false; // Stop rather than spam errors
            break;
        }

        std::this_thread::sleep_until(next);
    }
}

void Driver::SendColors() const
{
    m_logger->debug("Sending {} bytes to '{}'", m_buffer.size(), m_portName);

    size_t totalWritten = 0;
    const auto* data = reinterpret_cast<const char*>(m_buffer.data());

    while (totalWritten < m_buffer.size())
    {
        const ssize_t bytesWritten = write(m_serialPort, data + totalWritten, m_buffer.size() - totalWritten);

        if (bytesWritten < 0)
        {
            if (errno == EINTR)
                continue;

            throw SerialWriteException(
                std::format("Failed to write to serial port '{}': {} (errno: {})", m_portName, strerror(errno), errno));
        }

        if (bytesWritten == 0)
            throw SerialWriteException(std::format("write() returned 0 unexpectedly on '{}'", m_portName));

        totalWritten += static_cast<size_t>(bytesWritten);
    }

    m_logger->debug("Sent {} bytes successfully", totalWritten);
}

Response Driver::Fill(const ColorRGB color)
{
    if (!m_isConnectionOpened)
        return MakeError(1, "driver not started", "openskydimo start");

    m_logger->info("Filling {} LEDs with RGB({}, {}, {})", m_ledCount, color.r, color.g, color.b);

    {
        std::lock_guard lock(m_bufferMutex);
        int offset = m_headerSize;
        for (int i = 0; i < m_ledCount; i++)
        {
            m_buffer[offset++] = color.r;
            m_buffer[offset++] = color.g;
            m_buffer[offset++] = color.b;
        }
    }

    m_logger->debug("Buffer updated with new fill colour");
    return MakeOk();
}
void Driver::AddHeaderToBuffer()
{
    const size_t bufferSize = m_headerSize + (m_ledCount * 3);
    m_logger->debug("Resizing buffer to {} bytes ({} header + {} LEDs x 3 channels)", bufferSize, m_headerSize,
                    m_ledCount);

    m_buffer.resize(bufferSize);
    m_buffer[0] = static_cast<std::byte>('A');
    m_buffer[1] = static_cast<std::byte>('d');
    m_buffer[2] = static_cast<std::byte>('a');
    m_buffer[3] = static_cast<std::byte>(0);
    m_buffer[4] = static_cast<std::byte>(0);
    m_buffer[5] = static_cast<std::byte>(std::min(m_ledCount, 255));
}
