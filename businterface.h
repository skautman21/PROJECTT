#ifndef BUSINTERFACE_H
#define BUSINTERFACE_H

#include <QObject>
#include "ramdevice.h"
#include "romdevice.h"
#include "portfe.h"
#include "port1f.h"
#include "port7ffd.h"

class BusInterface : public QObject
{
    Q_OBJECT
public:
    explicit BusInterface(QObject *parent = nullptr);

    virtual uint8_t mem_read8(uint32_t addr) = 0;
    virtual void mem_write8(uint32_t addr, uint8_t value) = 0;

    virtual uint8_t io_read8(uint32_t addr) = 0;
    virtual void io_write8(uint32_t addr, uint8_t value) = 0;

    int border() const { return portfe.border(); }

    void key_press(int row, int col);
    void key_release(int row, int col);

    void kj_button_press(int btn) { port1f.press_button(btn); }
    void kj_button_release(int btn) { port1f.release_button(btn); }

    virtual const uint8_t * framebuffer() const = 0;

    virtual void reset() {};

signals:

protected:
    PortFE portfe;
    PORT1F port1f;
};

#endif // BUSINTERFACE_H
