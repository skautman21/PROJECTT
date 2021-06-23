#ifndef SCREENWIDGET_H
#define SCREENWIDGET_H

#include <QWidget>
#include "businterface.h"

class ScreenWidget : public QWidget
{
    Q_OBJECT
public:
    static constexpr int SCREEN_WIDTH = 256;
    static constexpr int SCREEN_HEIGHT = 192;
    static constexpr int SCREEN_ATTR_WIDTH = 32;
    static constexpr int SCREEN_ATTR_HEIGHT = 24;
    static constexpr int BORDER_MIN_WIDTH = 32;
    static constexpr int BORDER_MIN_HEIGHT = 24;

    explicit ScreenWidget(QWidget *parent = nullptr);

    void setBusInterface(const BusInterface *bi);

public slots:
    void toggleFlash() { flash_state = !flash_state; }
    void setPalChange(int index) {PalChange = index;}
signals:

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    const BusInterface * _bi { nullptr };
    bool flash_state { false };
    int PalChange {0};

};

#endif // SCREENWIDGET_H
