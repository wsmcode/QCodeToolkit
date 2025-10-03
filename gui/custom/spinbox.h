#ifndef SPINBOX_H
#define SPINBOX_H

#include <QSpinBox>

class SpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit SpinBox(QWidget *parent = nullptr);

signals:

protected:
    void wheelEvent(QWheelEvent *event) override;
};

#endif // SPINBOX_H
