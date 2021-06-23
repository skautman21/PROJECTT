#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QDebug>
#include <QKeyEvent>
#include <QMap>

#include "businterface48.h"
#include "businterface128.h"
#include "romdevice.h"

enum {
    CURSOR_IF = 0,
    KEMPSTON_IF = 1,
    ZXINTERFACE2_IF = 2,
};

#if defined (WIN32)
static constexpr int ESC_SCANCODE = 1;
static constexpr int F12_SCANCODE = 88;
static constexpr int UP_SCANCODE = 328;
static constexpr int DOWN_SCANCODE = 336;
static constexpr int LEFT_SCANCODE = 331;
static constexpr int RIGHT_SCANCODE = 333;
static constexpr int LCTRL_SCANCODE = 29;
static constexpr int SPACE_SCANCODE = 57;


static constexpr int UP_SCANCODE_W = 17;
static constexpr int DOWN_SCANCODE_S = 31;
static constexpr int LEFT_SCANCODE_A = 30;
static constexpr int RIGHT_SCANCODE_D = 32;
static constexpr int RCTRL_SCANCODE = 285;
#else
static constexpr int ESC_SCANCODE = 9;
static constexpr int F12_SCANCODE = 96;
static constexpr int UP_SCANCODE = 111;
static constexpr int DOWN_SCANCODE = 116;
static constexpr int LEFT_SCANCODE = 113;
static constexpr int RIGHT_SCANCODE = 114;
static constexpr int LCTRL_SCANCODE = 37;
#endif

static constexpr int PAIR(int a, int b) { return a * 100 + b; }
static constexpr int FIRST(int v) { return v / 100; }
static constexpr int SECOND (int v) { return v % 100; }

static const QMap<int, int> s_key_mapping {
        { 2,  PAIR(0, 11) },
        { 3,  PAIR(1, 11) },
        { 4,  PAIR(2, 11) },
        { 5,  PAIR(3, 11) },
        { 6,  PAIR(4, 11) },
        { 7,  PAIR(4, 12) },
        { 8,  PAIR(3, 12) },
        { 9,  PAIR(2, 12) },
        { 10, PAIR(1, 12) },
        { 11, PAIR(0, 12) },

        { 16, PAIR(0, 10) },
        { 17, PAIR(1, 10) },
        { 18, PAIR(2, 10) },
        { 19, PAIR(3, 10) },
        { 20, PAIR(4, 10) },
        { 21, PAIR(4, 13) },
        { 22, PAIR(3, 13) },
        { 23, PAIR(2, 13) },
        { 24, PAIR(1, 13) },
        { 25, PAIR(0, 13) },

        { 30, PAIR(0, 9) },
        { 31, PAIR(1, 9) },
        { 32, PAIR(2, 9) },
        { 33, PAIR(3, 9) },
        { 34, PAIR(4, 9) },
        { 35, PAIR(4, 14) },
        { 36, PAIR(3, 14) },
        { 37, PAIR(2, 14) },
        { 38, PAIR(1, 14) },
        { 28, PAIR(0, 14) },

        { 42, PAIR(0, 8) },
        { 44, PAIR(1, 8) },
        { 45, PAIR(2, 8) },
        { 46, PAIR(3, 8) },
        { 47, PAIR(4, 8) },
        { 48, PAIR(4, 15) },
        { 49, PAIR(3, 15) },
        { 50, PAIR(2, 15) },
        { 54, PAIR(1, 15) },
        { 57, PAIR(0, 15) },

        {331, PAIR(4,11) },
        {336, PAIR(4,12) },
        {328, PAIR(3,12) },
        {333, PAIR(2,12) },
        { 29, PAIR(0,12) },
};
static uint8_t s_mem_read(void *context, uint16_t address)
{
    BusInterface * bi = reinterpret_cast<BusInterface*>(context);
    return bi->mem_read8(address);

}

static void s_mem_write(void *context, uint16_t address, uint8_t value)
{
    BusInterface * bi = reinterpret_cast<BusInterface*>(context);
    return bi->mem_write8(address, value);
}

static uint8_t s_port_read(void *context, uint16_t address)
{
    BusInterface * bi = reinterpret_cast<BusInterface*>(context);
    return bi->io_read8(address);
}
static void s_port_write(void *context, uint16_t address, uint8_t value)
{
    BusInterface * bi = reinterpret_cast<BusInterface*>(context);
    return bi->io_write8(address, value);
}

static uint32_t s_int_data(void *context)
{
    Q_UNUSED(context);
    return 0xC3000000; //JP #0
}

static void s_halt (void * context, uint8_t state)
{
    Q_UNUSED(context);
    Q_UNUSED(state);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->cbShowControls->setChecked(false);
    ui->twControls->setVisible(false);

    bus = new BusInterface128();
    ui->screen->setBusInterface(bus);

    connect(ui->keyboard,
            SIGNAL(key_pressed(int,int)),
            this,
            SLOT(on_key_pressed(int,int))
            );
    connect(ui->keyboard,
            SIGNAL(key_released(int,int)),
            this,
            SLOT(on_key_released(int,int))
            );
    connect(ui->btnReset,
            SIGNAL(clicked()),
            this,
            SLOT(reset()));

    connect(ui->pbUp, SIGNAL(pressed()), this, SLOT(upPressed()));
    connect(ui->pbDown, SIGNAL(pressed()), this, SLOT(downPressed()));
    connect(ui->pbLeft, SIGNAL(pressed()), this, SLOT(leftPressed()));
    connect(ui->pbRight, SIGNAL(pressed()), this, SLOT(rightPressed()));
    connect(ui->pbFire, SIGNAL(pressed()), this, SLOT(firePressed()));
    connect(ui->pbUp, SIGNAL(released()), this, SLOT(upReleased()));
    connect(ui->pbDown, SIGNAL(released()), this, SLOT(downReleased()));
    connect(ui->pbLeft, SIGNAL(released()), this, SLOT(leftReleased()));
    connect(ui->pbRight, SIGNAL(released()), this, SLOT(rightReleased()));
    connect(ui->pbFire, SIGNAL(released()), this, SLOT(fireReleased()));

    bus->io_write8(0xfe, 1);

    cpustate.context = bus;
    cpustate.read = s_mem_read;
    cpustate.write = s_mem_write;
    cpustate.in = s_port_read;
    cpustate.out = s_port_write;
    cpustate.int_data = s_int_data;
    cpustate.halt = s_halt;

    reset();

//    for (uint32_t addr = 16384; addr < 16384 + 32*192 + 32*24; ++addr )
//    {
//        bus.mem_write8(addr, addr & 0xff);
//    }

    frame_timer = new QTimer();
    connect(frame_timer,
            SIGNAL(timeout()),
            this,
            SLOT(frameRefresh()));
    frame_timer->start(1000 / 50);

    flash_timer = new QTimer();
    connect(flash_timer,
            SIGNAL(timeout()),
            ui->screen,
            SLOT(toggleFlash()));
    flash_timer->start(320);
}

MainWindow::~MainWindow()
{
    delete frame_timer;
    delete flash_timer;
    delete ui;
}

#pragma pack(push, 1)
struct SNAHeader
{
    uint8_t     I;
    uint16_t    HL_, DE_,
                BC_, AF_;
    uint16_t    HL, DE, BC, IY, IX;
    uint8_t     IFF2;
    uint8_t     R;
    uint16_t     AF, SP;
    uint8_t     IM;
    uint8_t     BDR;

};
#pragma pack(pop)

void MainWindow::load_sna(const QString &filename)
{
    QFile sna(filename);

    if (sna.open(QIODevice::ReadOnly)){
        QByteArray buffer;
        buffer = sna.readAll();
        SNAHeader * sna_hdr = reinterpret_cast<SNAHeader *>(buffer.data());
        uint8_t * sna_memory = reinterpret_cast<uint8_t *>(buffer.data()) + sizeof (SNAHeader);
        cpustate.state.i = sna_hdr->I;
        cpustate.state.hl_.value_uint16 = sna_hdr->HL_;
        cpustate.state.de_.value_uint16 = sna_hdr->DE_;
        cpustate.state.bc_.value_uint16 = sna_hdr->BC_;
        cpustate.state.af_.value_uint16 = sna_hdr->AF_;
        cpustate.state.hl.value_uint16 = sna_hdr->HL;
        cpustate.state.de.value_uint16 = sna_hdr->DE;
        cpustate.state.bc.value_uint16 = sna_hdr->BC;
        cpustate.state.af.value_uint16 = sna_hdr->AF;
        cpustate.state.iy.value_uint16 = sna_hdr->IY;
        cpustate.state.ix.value_uint16 = sna_hdr->IX;
        cpustate.state.internal.iff2 = sna_hdr->IFF2 >> 2;
        cpustate.state.r = sna_hdr->R;
        cpustate.state.af.value_uint16 = sna_hdr->AF;
        cpustate.state.sp = sna_hdr->SP;
        cpustate.state.internal.im = sna_hdr->IM;
        bus->io_write8(0xfe, sna_hdr->BDR);
        for (int off = 0; off < 49152; ++off) {
            bus->mem_write8(16384 + off, sna_memory[off]);
        }

    }
}

void MainWindow::upPressed(bool PlayerS)
{
    switch (ui->cbJoystickInterface->currentIndex()) {
    case CURSOR_IF: bus->key_press(3, 12);break;
    case KEMPSTON_IF: bus->kj_button_press(PORT1F::KJ_UP);break;
    case ZXINTERFACE2_IF: if (PlayerS) bus->key_press(3, 11); else bus->key_press(1,12);break;
    }
}

void MainWindow::downPressed(bool PlayerS)
{
    switch (ui->cbJoystickInterface->currentIndex()) {
    case CURSOR_IF: bus->key_press(4, 12);break;
    case KEMPSTON_IF: bus->kj_button_press(PORT1F::KJ_DOWN);break;
    case ZXINTERFACE2_IF: if (PlayerS) bus->key_press(2, 11); else bus->key_press(2,12);break;
    }
}

void MainWindow::leftPressed(bool PlayerS)
{
    switch (ui->cbJoystickInterface->currentIndex()) {
    case CURSOR_IF: bus->key_press(4, 11);break;
    case KEMPSTON_IF: bus->kj_button_press(PORT1F::KJ_LEFT);break;
    case ZXINTERFACE2_IF: if (PlayerS) bus->key_press(0, 11); else bus->key_press(4,12);break;
    }
}

void MainWindow::rightPressed(bool PlayerS)
{
    switch (ui->cbJoystickInterface->currentIndex()) {
    case CURSOR_IF: bus->key_press(2, 12);break;
    case KEMPSTON_IF: bus->kj_button_press(PORT1F::KJ_RIGHT);break;
    case ZXINTERFACE2_IF: if (PlayerS) bus->key_press(1, 11); else bus->key_press(3,12);break;
    }
}

void MainWindow::firePressed(bool PlayerS)
{
    switch (ui->cbJoystickInterface->currentIndex()) {
    case CURSOR_IF: bus->key_press(0, 12);break;
    case KEMPSTON_IF: bus->kj_button_press(PORT1F::KJ_FIRE);break;
    case ZXINTERFACE2_IF: if (PlayerS) bus->key_press(4, 11); else bus->key_press(0,12);break;
    }
}

void MainWindow::keyPressed(int sc)
{
    auto elem = s_key_mapping.find(sc);
    if (elem != s_key_mapping.end()){
        int row = FIRST(elem.value());
        int col = SECOND(elem.value());
        bus->key_press(row, col);
    }
}

void MainWindow::upReleased(bool PlayerS)
{
    switch (ui->cbJoystickInterface->currentIndex()) {
    case CURSOR_IF: bus->key_release(3, 12);break;
    case KEMPSTON_IF: bus->kj_button_release(PORT1F::KJ_UP);break;
    case ZXINTERFACE2_IF: if (PlayerS) bus->key_release(3, 11); else bus->key_release(1,12);break;
    }
}

void MainWindow::downReleased(bool PlayerS)
{
    switch (ui->cbJoystickInterface->currentIndex()) {
    case CURSOR_IF: bus->key_release(4, 12);break;
    case KEMPSTON_IF: bus->kj_button_release(PORT1F::KJ_DOWN);break;
    case ZXINTERFACE2_IF: if (PlayerS) bus->key_release(2, 11); else bus->key_release(2,12);break;
    }
}

void MainWindow::leftReleased(bool PlayerS)
{
    switch (ui->cbJoystickInterface->currentIndex()) {
    case CURSOR_IF: bus->key_release(4, 11);break;
    case KEMPSTON_IF: bus->kj_button_release(PORT1F::KJ_LEFT);break;
    case ZXINTERFACE2_IF: if (PlayerS) bus->key_release(0, 11); else bus->key_release(4,12);break;
    }
}

void MainWindow::rightReleased(bool PlayerS)
{
    switch (ui->cbJoystickInterface->currentIndex()) {
    case CURSOR_IF: bus->key_release(2, 12);break;
    case KEMPSTON_IF: bus->kj_button_release(PORT1F::KJ_RIGHT);break;
    case ZXINTERFACE2_IF: if (PlayerS) bus->key_release(1, 11); else bus->key_release(3,12);break;
    }
}

void MainWindow::fireReleased(bool PlayerS)
{
    switch (ui->cbJoystickInterface->currentIndex()) {
    case CURSOR_IF: bus->key_release(0, 12);break;
    case KEMPSTON_IF: bus->kj_button_release(PORT1F::KJ_FIRE);break;
    case ZXINTERFACE2_IF: if (PlayerS) bus->key_release(4, 11); else bus->key_release(0,12);break;
    }
}

void MainWindow::keyReleased(int sc)
{
    auto elem = s_key_mapping.find(sc);
    if (elem != s_key_mapping.end()){
        int row = FIRST(elem.value());
        int col = SECOND(elem.value());
        bus->key_release(row, col);
    }
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object);
//    if (event->type() == QEvent::FocusOut and
//        object == ui->screen and
//        ui->cbCaptureKeyboard->isChecked()) {
//        ui->screen->setFocus();
//    }
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(event);
  //      qDebug() << "Key pressed: "  << ke->nativeScanCode();
        auto sc = ke->nativeScanCode();

        switch (sc) {
        case ESC_SCANCODE:  reset();break;
        case F12_SCANCODE:  z80_nmi(&cpustate);break;
        case UP_SCANCODE:   upPressed();break;
        case DOWN_SCANCODE: downPressed();break;
        case LEFT_SCANCODE: leftPressed();break;
        case RIGHT_SCANCODE:rightPressed();break;
        case RCTRL_SCANCODE:
        case LCTRL_SCANCODE:firePressed();break;


        case UP_SCANCODE_W:   upPressed(true);break;
        case DOWN_SCANCODE_S: downPressed(true);break;
        case LEFT_SCANCODE_A: leftPressed(true);break;
        case RIGHT_SCANCODE_D:rightPressed(true);break;
        case SPACE_SCANCODE:firePressed(true);break;
        default:            keyPressed(sc);
        }
    }
    if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(event);
        qDebug() << "Key released: "  << ke->nativeScanCode();
        auto sc = ke->nativeScanCode();
        switch (sc) {
        case UP_SCANCODE:upReleased();break;
        case DOWN_SCANCODE:downReleased();break;
        case LEFT_SCANCODE:leftReleased();break;
        case RIGHT_SCANCODE:rightReleased();break;
        case RCTRL_SCANCODE:
        case LCTRL_SCANCODE:fireReleased();break;

        case UP_SCANCODE_W:    upReleased(true);break;
        case DOWN_SCANCODE_S:  downReleased(true);break;
        case LEFT_SCANCODE_A:  leftReleased(true);break;
        case RIGHT_SCANCODE_D: rightReleased(true);break;
        case SPACE_SCANCODE:   fireReleased(true);break;
        default: keyReleased(sc);
        }
    }
    return false;
}

void MainWindow::frameRefresh()
{
    z80_run(&cpustate, 70000 - 28);
    z80_int(&cpustate, 1);
    z80_run(&cpustate, 28);
    z80_int(&cpustate, 0);
    ui->screen->repaint();
}


void MainWindow::on_cbShowControls_stateChanged(int state)
{
    if (state == Qt::Checked) {
        ui->twControls->setVisible(true);
    } else {
        ui->twControls->setVisible(false);
    }
}

void MainWindow::reset()
{
    z80_reset(&cpustate);
    bus->reset();
}

void MainWindow::on_key_pressed(int row, int col)
{
    qDebug() << "Key pressed: " << row << " " << col;
    bus->key_press(row, col);
}

void MainWindow::on_key_released(int row, int col)
{
    qDebug() << "Key released: " << row << " " << col;
    bus->key_release(row, col);
}

void MainWindow::on_cbCaptureKeyboard_stateChanged(int state)
{
    if (state == Qt::Checked) {

        installEventFilter(this);
    } else {

        installEventFilter(nullptr);
    }
}

void MainWindow::on_btnTest_clicked()
{
    load_sna("sna/river.sna");
}

void MainWindow::on_actionSpectrum_48k_triggered()
{
    ui->actionSpectrum_48k->setChecked(true);
    ui->actionSpectrum_128k->setChecked(false);
    auto new_bi = new BusInterface48();
    auto old_bi = bus;
    ui->screen->setBusInterface(new_bi);
    cpustate.context = new_bi;
    bus = new_bi;
    delete old_bi;
    reset();
}

void MainWindow::on_actionSpectrum_128k_triggered()
{
    ui->actionSpectrum_48k->setChecked(false);
    ui->actionSpectrum_128k->setChecked(true);
    auto new_bi = new BusInterface128();
    auto old_bi = bus;
    ui->screen->setBusInterface(new_bi);
    cpustate.context = new_bi;
    bus = new_bi;
    delete old_bi;
    reset();
}

void MainWindow::on_cbPalChange_currentIndexChanged(int index)
{
    if (index == 0)
        ui->screen->setPalChange(0);
    else
        ui->screen->setPalChange(16);
}


void MainWindow::on_actionLoad_a_screenshot_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Выберите файл", "", "*.scr");

    QFile scr(fileName);
    if (scr.open(QIODevice::ReadOnly)){
        QByteArray buffer;
        buffer = scr.readAll();
        uint8_t * scr_memory = reinterpret_cast<uint8_t *>(buffer.data());
        for(int off = 0; off < 6912; ++off){
            bus->mem_write8(16384 + off, scr_memory[off]);
        }
    }
}

void MainWindow::on_actionSave_a_screenshot_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Выберетие папку и название файла", "", "*.scr");

    QFile scr(fileName);
    if (scr.open(QIODevice::ReadWrite)){
        QDataStream out(&scr);
        for (int off = 0; off < 6912; ++off){
            out << bus->mem_read8(16384 + off);
        }
        scr.close();
    }
}


void MainWindow::on_actionReset_triggered()
{
    reset();
}


void MainWindow::on_actionNMI_triggered()
{
    z80_nmi(&cpustate);
}


void MainWindow::on_actionE_xit_triggered()
{
    QApplication::quit();
}


void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "About", "Это приложение было создано Артемием Крысиным");
}

