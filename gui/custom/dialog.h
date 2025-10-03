#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

class Dialog : public QDialog
{
    Q_OBJECT
public:
    explicit Dialog(QWidget *parent = nullptr);

signals:

protected:
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    // 鼠标点击事件
    void mousePressEvent(QMouseEvent *event) override;
    // 鼠标移动事件
    void mouseMoveEvent(QMouseEvent *event) override;
    // 鼠标释放事件
    void mouseReleaseEvent(QMouseEvent *event) override;

    QWidget *m_centralWidget;
    QPushButton *m_closePBtn;

private:
    void initDialogUI();
    void bringToFront(); // 将对话框置于最前

    // 鼠标相对窗口位移
    QPoint m_dragPosition;
    bool m_isDragging = false;
};

#endif // DIALOG_H
