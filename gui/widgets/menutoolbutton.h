#ifndef MENUTOOLBUTTON_H
#define MENUTOOLBUTTON_H

#include <QWidget>

class QSvgRenderer;
class QPropertyAnimation;
class MenuToolButton : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity) // 透明度属性，用于动画
public:
    explicit MenuToolButton(QWidget *parent = nullptr);
    explicit MenuToolButton(const QString &imagePath, QWidget *parent = nullptr);
    explicit MenuToolButton(const QString &imagePath, const QString &toolTip, QWidget *parent = nullptr);

    // 设置按钮图片
    void setImage(const QString &image);
    // 设置按钮选中状态
    void setChecked(bool checked);
    // 获取按钮选中状态
    bool isChecked() const;

    // 设置点击动画持续时间
    void setClickAnimationDuration(int ms);
    // 设置悬停动画持续时间
    void setHoverAnimationDuration(int ms);

    // 设置父控件方向（水平或垂直），用于自动调整尺寸
    void setParentOrientation(Qt::Orientation orientation);
    // 获取父控件方向
    Qt::Orientation parentOrientation() const;

signals:
    void clicked();      // 点击信号
    void toggled(bool);  // 切换状态信号

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    // 透明度属性相关
    qreal opacity() const;
    void setOpacity(qreal opacity);

private slots:
    void onEnterAnimationFinished();
    void onLeaveAnimationFinished();
    void onClickAnimationFinished();

private:
    void init();
    void startClickAnimation();
    void updatePixmapSize();
    void updateButtonSize(); // 根据父控件方向更新按钮尺寸
    void calculateSizes();   // 计算各种状态下的尺寸

    // 状态标志
    bool m_isHovered = false; // 鼠标悬停状态
    bool m_pressed = false;   // 鼠标按下状态
    bool m_checked = false;   // 选中状态

    // 尺寸相关
    int m_normalSize = 0;     // 正常状态尺寸（正方形边长）
    int m_hoverSize = 0;      // 悬停状态尺寸（正方形边长）
    int m_currentSize = 0;    // 当前尺寸（正方形边长）

    // 图片资源
    QString m_imagePath;      // 图片路径
//    QPixmap m_pixmap;         // 图片对象
    QSvgRenderer *m_svgRenderer = nullptr;  // 替换为SVG渲染器

    // 动画资源
    qreal m_opacity = 1.0;    // 当前透明度
    QPropertyAnimation *m_enterAnimation = nullptr;  // 进入动画
    QPropertyAnimation *m_leaveAnimation = nullptr;  // 离开动画
    QPropertyAnimation *m_clickAnimation = nullptr;  // 点击动画

    // 动画持续时间
    int m_clickAnimationDuration = 150;  // 点击动画时长(毫秒)
    int m_hoverAnimationDuration = 200;  // 悬停动画时长(毫秒)

    // 父控件方向相关
    Qt::Orientation m_parentOrientation = Qt::Horizontal; // 默认水平方向
    int m_sizeRatio = 60; // 尺寸比例（相对于父控件尺寸的百分比）
    qreal m_hoverScale = 1.3; // 悬停时的缩放比例
};

#endif // MENUTOOLBUTTON_H
