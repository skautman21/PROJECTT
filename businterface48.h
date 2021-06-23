#ifndef BUSINTERFACE48_H
#define BUSINTERFACE48_H

#include "businterface.h"

class BusInterface48 : public BusInterface
{
    Q_OBJECT
public:
    BusInterface48();

    virtual uint8_t mem_read8(uint32_t addr);
    virtual void mem_write8(uint32_t addr, uint8_t value);

    virtual uint8_t io_read8(uint32_t addr);
    virtual void io_write8(uint32_t addr, uint8_t value);

    virtual const uint8_t * framebuffer() const {return ram.getBuffer(16384); }

private:
    ROMDevice rom { "rom/48.rom" };
    RAMDevice ram { 16 };
};

#endif // BUSINTERFACE48_H
