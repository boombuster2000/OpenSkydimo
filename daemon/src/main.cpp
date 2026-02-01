#include <openskydimo/logger.h>

int main()
{
    const openskydimo::Logger logger("daemon");
    logger.Log("Starting");
}