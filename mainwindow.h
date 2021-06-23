#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "businterface.h"
#include "emulation/CPU/Z80.h"
#include <QTimer>
#include <QFileDialog>
#include <QMessageBox>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void load_sna(const QString& filename);

    void keyPressed(int sc);

    void keyReleased(int sc);

public slots:
    void upPressed(bool PlayerS = false);

    void leftPressed(bool PlayerS = false);

    void rightPressed(bool PlayerS = false);

    void firePressed(bool PlayerS = false);

    void downPressed(bool PlayerS = false);

    void upReleased(bool PlayerS = false);

    void leftReleased(bool PlayerS = false);

    void downReleased(bool PlayerS = false);

    void rightReleased(bool PlayerS = false);

    void fireReleased(bool PlayerS = false);





protected:
    virtual bool eventFilter(QObject *object, QEvent *event) override;


private slots:
    void frameRefresh();
    void on_cbShowControls_stateChanged(int state);

    void reset();

    void on_key_pressed(int row, int col);
    void on_key_released(int row, int col);

    void on_cbCaptureKeyboard_stateChanged(int state);

    void on_btnTest_clicked();    
    void on_actionSpectrum_48k_triggered();

    void on_actionSpectrum_128k_triggered();

    void on_cbPalChange_currentIndexChanged(int index);

    void on_actionSave_a_screenshot_triggered();

    void on_actionLoad_a_screenshot_triggered();

    void on_actionReset_triggered();

    void on_actionNMI_triggered();

    void on_actionE_xit_triggered();

    void on_actionAbout_triggered();

private:
    Ui::MainWindow *ui;

    BusInterface * bus { nullptr };

    // redcode/Z80
    Z80 cpustate {};
    QTimer *frame_timer;
    QTimer *flash_timer;
};
#endif // MAINWINDOW_H
