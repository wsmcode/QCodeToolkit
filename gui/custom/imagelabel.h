#ifndef IMAGELABEL_H
#define IMAGELABEL_H

#include <QLabel>
#include <QSvgRenderer>

class ImageLabel : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(QSize imageSize READ imageSize WRITE setImageSize NOTIFY imageSizeChanged)
public:
    explicit ImageLabel(QWidget *parent = nullptr);

    void setPixmap(const QPixmap &pixmap);

signals:
    void imageSizeChanged(const QSize &size);

protected:
    void paintEvent(QPaintEvent *event) override;
    void changeEvent(QEvent *event) override;

    QSize imageSize() const;
    void setImageSize(const QSize &size);

private:
    void updatePixmapFromStyle();
    QString extractImageUrlFromStyleSheet() const;
    bool isSvgFile(const QString &filePath) const;

    QSize m_imageSize = QSize(40, 40);
    QPixmap m_cachedPixmap;
    QSvgRenderer m_svgRenderer;
    bool m_pixmapDirty = true;
    bool m_hasSvg = false;

    bool m_hasPixmap = false;
    bool m_isUpdate = false;
};

#endif // IMAGELABEL_H
