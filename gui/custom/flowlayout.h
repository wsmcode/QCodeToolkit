#ifndef FLOWLAYOUT_H
#define FLOWLAYOUT_H

/*****************************************************
*
* @file     flowlayout.h
* @brief    FlowLayout流式布局
*
* @description
*           ==== 使用说明 ====
*           Qt官方文档
*           https://doc.qt.io/archives/qt-5.15/qtwidgets-layouts-flowlayout-example.html
*           ==== 注意 ====
*
* @author   Qt
* @date     2025/09/07
* @history
*****************************************************/

#include <QLayout>
#include <QWidget>
#include <QStyle>

class FlowLayout : public QLayout
{
public:
    explicit FlowLayout(QWidget *parent, int margin = -1, int hSpacing = -1, int vSpacing = -1);
    explicit FlowLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);
    ~FlowLayout();

    void addItem(QLayoutItem *item) override;
    int horizontalSpacing() const;
    int verticalSpacing() const;
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int) const override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect &rect) override;
    QSize sizeHint() const override;
    QLayoutItem *takeAt(int index) override;

private:
    int doLayout(const QRect &rect, bool testOnly) const;
    int smartSpacing(QStyle::PixelMetric pm) const;

    QList<QLayoutItem *> itemList;
    int m_hSpace;
    int m_vSpace;
};

#endif // FLOWLAYOUT_H
