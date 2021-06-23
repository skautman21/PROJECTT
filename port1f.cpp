#include "port1f.h"

PORT1F::PORT1F()
{

}

uint8_t PORT1F::read8(uint32_t address)
{
    Q_UNUSED(address);
    return _port_1f_data;
}

void PORT1F::write8(uint32_t address, uint8_t value)
{
    Q_UNUSED(address);
    Q_UNUSED(value;)
}

void PORT1F::press_button(int btn)
{
    _port_1f_data |= (1 << btn);

}

void PORT1F::release_button(int btn)
{
   _port_1f_data &= ~(1 << btn);
}
