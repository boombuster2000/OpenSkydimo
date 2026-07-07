#include "Driver.h"

#include <fcntl.h>
#include <format>
#include <termios.h>
#include <unistd.h>

Driver::~Driver()
{
    if (m_serialPort >= 0)
    {
        close(m_serialPort);
        m_serialPort = -1;
    }
}

Response Driver::SetRefreshRate(const int hz)
{
    if (hz <= 0)
        return {1, "Refresh rate must be > 0 Hz"};

    logger->info("Setting refresh rate to {} Hz", hz);
    m_sendInterval = std::chrono::microseconds(1'000'000 / hz);

    return {0, "OK"};
}

Response Driver::SetSerialPort(const std::string& portName)
{
    logger->info("Setting serial port to '{}'", portName);
    m_portName = portName;

    return {0, "OK"};
}

Response Driver::SetBaudRate(const int baudRate)
{
    logger->info("Setting baud rate to {}", baudRate);
    m_baudRate = baudRate;
    return {0, "OK"};
}

Response Driver::SetLedCount(const int ledCount)
{
    logger->info("Setting LED count to {}", ledCount);
    m_ledCount = ledCount;
    AddHeaderToBuffer();
    return {0, "OK"};
}

Response Driver::OpenSerialConnection()
{
    if (m_running)
    {
        logger->warn("Tried to open serial connection but already running.");
        return {2, "Tried to open serial connection but already running."};
    }

    logger->info("Opening serial connection on port '{}'", m_portName);

    if (m_portName.empty())
        return {1, "No serial port set. Run 'openskydimo set port <path>' before starting "
                   "(e.g. set port /dev/ttyUSB0)."};

    if (m_ledCount == 0)
        return {1, "LED count has not been set. Run 'openskydimo set count <n>' before starting."};

    m_serialPort = open(m_portName.c_str(), O_RDWR | O_NOCTTY);
    if (m_serialPort < 0)
        return {1, std::format("Unable to open serial port '{}'", m_portName)};

    logger->debug("Serial port '{}' opened, configuring tty attributes", m_portName);

    termios tty{};
    if (tcgetattr(m_serialPort, &tty) != 0)
    {
        close(m_serialPort);
        m_serialPort = -1;
        return {1, "Unable to get tty attributes"};
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
        return {1, std::format("Unsupported baud rate: {}", m_baudRate)};
    }

    cfsetispeed(&tty, baudRate);
    cfsetospeed(&tty, baudRate);
    logger->debug("Baud rate set to {}", m_baudRate);

    if (tcsetattr(m_serialPort, TCSANOW, &tty) != 0)
    {
        close(m_serialPort);
        m_serialPort = -1;
        return {1, "Unable to set tty attributes"};
    }

    logger->info("Serial connection established on '{}' at {} baud, ready to send to {} LEDs", m_portName, m_baudRate,
                 m_ledCount);

    m_running = true;
    m_sendThread = std::thread(&Driver::SendLoop, this);
    logger->info("Send thread started at ~{} Hz", 1'000'000 / m_sendInterval.count());

    return {0, "OK"};
}

Response Driver::CloseSerialConnection()
{
    if (m_serialPort < 0)
    {
        logger->warn("CloseSerialConnection called but no connection is open");
        return {2, "CloseSerialConnection called but no connection is open"};
    }

    // Signal and join the thread before closing the port
    logger->info("Stopping send thread");
    m_running = false;
    if (m_sendThread.joinable())
        m_sendThread.join();
    logger->info("Send thread stopped");

    logger->info("Closing serial connection on '{}'", m_portName);
    close(m_serialPort);
    m_serialPort = -1;
    logger->info("Serial connection closed");

    return {0, "OK"};
}

void Driver::SendLoop()
{
    using clock = std::chrono::steady_clock;

    while (m_running)
    {
        const auto next = clock::now() + m_sendInterval;

        try
        {
            std::lock_guard lock(m_bufferMutex);
            SendColors();
        }
        catch (const SerialWriteException& e)
        {
            logger->error("Send loop write error: {}", e.what());
            m_running = false; // Stop rather than spam errors
            break;
        }

        std::this_thread::sleep_until(next);
    }
}

void Driver::SendColors() const
{
    logger->debug("Sending {} bytes to '{}'", m_buffer.size(), m_portName);

    const ssize_t bytesWritten = write(m_serialPort, m_buffer.data(), m_buffer.size());

    if (bytesWritten < 0)
        throw SerialWriteException(
            std::format("Failed to write to serial port '{}': {} (errno: {})", m_portName, strerror(errno), errno));

    if (static_cast<size_t>(bytesWritten) != m_buffer.size())
        throw SerialWriteException(
            std::format("Incomplete write to '{}': {}/{} bytes", m_portName, bytesWritten, m_buffer.size()));

    logger->debug("Sent {} bytes successfully", bytesWritten);
}

Response Driver::Fill(const ColorRGB color)
{
    if (!m_running)
        return {1, "Driver has not been started. Run 'openskydimo start' to start it."};

    logger->info("Filling {} LEDs with RGB({}, {}, {})", m_ledCount, color.r, color.g, color.b);

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

    logger->debug("Buffer updated with new fill colour");
    return {0, "OK"};
}
void Driver::AddHeaderToBuffer()
{
    const size_t bufferSize = m_headerSize + (m_ledCount * 3);
    logger->debug("Resizing buffer to {} bytes ({} header + {} LEDs x 3 channels)", bufferSize, m_headerSize,
                  m_ledCount);

    m_buffer.resize(bufferSize);
    m_buffer[0] = static_cast<std::byte>('A');
    m_buffer[1] = static_cast<std::byte>('d');
    m_buffer[2] = static_cast<std::byte>('a');
    m_buffer[3] = static_cast<std::byte>(0);
    m_buffer[4] = static_cast<std::byte>(0);
    m_buffer[5] = static_cast<std::byte>(std::min(m_ledCount, 255));
}
