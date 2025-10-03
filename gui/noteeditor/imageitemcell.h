#ifndef IMAGEITEMCELL_H
#define IMAGEITEMCELL_H

/*****************************************************
*
* @file     imageitemcell.h
* @brief    ImageItemCell类
*
* @description
*           ==== 布局 ====
*           ==== 核心功能 ====
*           ==== 使用说明 ====
*           ==== 注意 ====
*           复制图片需要重写（复制图片到，复制为路径、markdown、html）
*           需要加入放大缩小
*
* @author   无声目
* @date     2025/09/27
* @history
*****************************************************/
#include "itemcell.h"

class QLabel;
class ImageItemCell : public ItemCell
{
    Q_OBJECT
public:
    explicit ImageItemCell(NoteItem item, QWidget *parent = nullptr);
    ~ImageItemCell();

signals:

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void selectImage();
    void removeImage();
//    void copyImage();
    void updateAnimation();
    void onCustomContextMenu(const QPoint &pos);

private:
    void initUI() override;
    void updateContent() override;
    void scaleImages(double scaleFactor = 1.0);
    void scaleMovieFrame();
    void setupMovie(const QString& fileName); // 设置QMovie
    void startDrag(); // 开始拖拽操作
    bool loadImageFromUrl(const QUrl& url); // 从URL加载图片

    QLabel *m_imageLabel = nullptr;
    QMovie *m_movie = nullptr;

    QPixmap m_originalPixmaps;
    double m_scaleFactors = 1.0;
    bool m_isAnimated = false;
    QPoint m_dragStartPosition; // 拖拽起始位置
};

#endif // IMAGEITEMCELL_H
