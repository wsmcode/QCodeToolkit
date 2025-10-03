#ifndef ROTATIONBUTTON_H
#define ROTATIONBUTTON_H

#include <QPushButton>

class RotationButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(int rotation READ rotation WRITE setRotation) // 添加自定义属性支持旋转动画
public:
    explicit RotationButton(QWidget *parent = nullptr);

signals:

protected:
    void paintEvent(QPaintEvent *event) override;

    // 旋转属性相关方法
    int rotation() const;
    void setRotation(int rotation);

private:
    int m_rotation = 0;  // 旋转角度
};

#endif // ROTATIONBUTTON_H
