#include "screenwidget.h"
#include <QPainter>
#include <algorithm>

//COLOR MODEL - GRB
static const QColor s_palette[32] {

            QColor(0, 0, 0),
            QColor(0, 0, 192),
            QColor(192, 0, 0),
            QColor(192, 0, 192),
            QColor(0, 192, 0),
            QColor(0,192,192),
            QColor(192,192,0),
            QColor(192,192,192),

            QColor(0, 0, 0),
            QColor(96, 96, 255),
            QColor(255, 96, 96),
            QColor(255, 96, 255),
            QColor(96, 255, 96),
            QColor(96,255,255),
            QColor(255,255,96),
            QColor(255,255,255),



            QColor(0, 0, 0),
            QColor(192, 96, 192),
            QColor(192, 0, 96),
            QColor(192, 192, 192),
            QColor(192, 192, 50),
            QColor(192,192,192),
            QColor(192,10,0),
            QColor(192,5,192),

            QColor(0, 0, 0),
            QColor(96, 96, 255),
            QColor(255, 96, 96),
            QColor(255, 96, 255),
            QColor(96, 255, 96),
            QColor(96,255,255),
            QColor(60, 255, 96),
            QColor(96,255,96)
};

ScreenWidget::ScreenWidget(QWidget *parent) : QWidget(parent)
{
    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
}

void ScreenWidget::setBusInterface(const BusInterface *bi)
{
    _bi = bi;
}

void ScreenWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.fillRect(rect(), s_palette[_bi->border()+PalChange]);

    // Каждый пиксель представляет собой квдарат со стороной n
    int nw, nh, n;

    int ox, oy;

    nw = (width() - 2 * BORDER_MIN_HEIGHT) / SCREEN_WIDTH;
    nh = (height() - 2 * BORDER_MIN_HEIGHT) / SCREEN_HEIGHT;
    n = std::min(nw, nh);
    if (n < 1) n = 1;

    ox = (width() - SCREEN_WIDTH * n) / 2;
    oy = (height() - SCREEN_HEIGHT * n) / 2;

    auto fb = _bi->framebuffer();
    const uint8_t * pixel_data = &fb[0];
    const uint8_t * attr_data = &fb[SCREEN_WIDTH * SCREEN_HEIGHT / 8];

    int x,y,ax,ay;
    for (y = 0; y < SCREEN_HEIGHT; ++y) {
        for (ax = 0; ax < SCREEN_WIDTH / 8; ++ax) {
            ay = y / 8;

            uint8_t attr = attr_data[ay * SCREEN_WIDTH / 8 + ax];
            uint8_t ink = attr & 0b00000111;
            uint8_t paper = (attr & 0b00111000) >> 3;
            uint8_t bright = (attr & 0b01000000) >> 6;
            uint8_t flash = (attr & 0b10000000) >> 7;

            int block_addr = y * SCREEN_WIDTH / 8 + ax;
            int a4a0    =   block_addr & 0b0000000000011111;
            int a10a8   =   (block_addr & 0b0000011100000000) >> 8;
            int a7a5    =   (block_addr & 0b0000000011100000) >> 5;
            int a12a11  =   (block_addr & 0b0001100000000000) >> 11;
            int video_addr = a4a0 + (a10a8 << 5) + (a7a5 << 8) + (a12a11 << 11);
            uint8_t data = pixel_data[video_addr];
            for (int px = 0; px < 8; ++px) {
                x = ax * 8 + px;

                QRect pr( ox + x * n,
                          oy + y * n,
                          n, n);
                bool pixel = (data & (0b10000000 >> px));
                if (pixel xor (flash_state && flash)) {// ink
                    p.fillRect(pr, s_palette[(ink + 8 * bright)+PalChange]);
                } else { //paper
                    p.fillRect(pr, s_palette[paper+PalChange]);
                }
            }
        }
    }
}
