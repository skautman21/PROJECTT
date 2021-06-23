#ifndef ZXPUSHBUTTON_H
#define ZXPUSHBUTTON_H

#include <QPushButton>
#include <QWidget>

class ZXPushButton : public QPushButton
{
    Q_OBJECT
public:
    ZXPushButton(int row, int col, QWidget *parent = nullptr);

    int row() const { return _key_row; }
    int col() const { return _key_col; }
Q_SIGNALS:
    int pressed(int row, int col);
    int released(int row, int col);

private:
    int _key_row, _key_col;
public Q_SLOTS:
    void on_pressed();
    void on_released();
    void on_toggled(bool v);

};

#endif // ZXPUSHBUTTON_H
