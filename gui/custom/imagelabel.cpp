#include "imagelabel.h"

#include <QEvent>
#include <QFile>
#include <QPainter>
#include <QRegularExpression>
#include <QStyleOptionButton>

ImageLabel::ImageLabel(QWidget *parent) : QLabel(parent)
{
    setAlignment(Qt::AlignCenter);
    setFrameShape(QFrame::Box);
    setFixedSize(m_imageSize * 1.25);
}

void ImageLabel::setPixmap(const QPixmap &pixmap)
{
    QLabel::setPixmap(pixmap);
    if(this->pixmap()->isNull()) m_hasPixmap = false;
    else m_hasPixmap = true;
}

void ImageLabel::paintEvent(QPaintEvent *event)
{
    m_isUpdate = true;
    if(m_hasPixmap)
    {
        QLabel::paintEvent(event);
        m_isUpdate = false;  // 确保在返回前设置 m_isUpdate
        return;
    }
    QPainter painter(this);

    // 只绘制样式表的背景和边框，不绘制图片
    // 创建一个临时的样式表，移除图片相关属性
    QString originalStyleSheet = this->styleSheet();
    QString tempStyleSheet = originalStyleSheet;

    // 移除图片相关的样式属性
    tempStyleSheet.remove(QRegularExpression("image\\s*:\\s*[^;]*;",
                                             QRegularExpression::CaseInsensitiveOption));
    tempStyleSheet.remove(QRegularExpression("background-image\\s*:\\s*[^;]*;",
                                             QRegularExpression::CaseInsensitiveOption));

    // 应用临时样式表
    if(tempStyleSheet != originalStyleSheet) this->setStyleSheet(tempStyleSheet);

    // 绘制默认样式（边框、背景等）
    QStyleOption opt;
    opt.initFrom(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    // 恢复原始样式表
    if(tempStyleSheet != originalStyleSheet) this->setStyleSheet(originalStyleSheet);

    // 如果需要更新图片缓存
    if(m_pixmapDirty) updatePixmapFromStyle();

    // 绘制图片
    if(m_hasSvg && m_svgRenderer.isValid())
    {
        // 计算 SVG 绘制位置和大小（保持宽高比）
        QSize svgSize = m_svgRenderer.defaultSize();
        svgSize.scale(m_imageSize, Qt::KeepAspectRatio);

        int x = (width() - svgSize.width()) / 2;
        int y = (height() - svgSize.height()) / 2;

        m_svgRenderer.render(&painter, QRect(x, y, svgSize.width(), svgSize.height()));
    }
    else if(!m_cachedPixmap.isNull())
    {
        int x = (width() - m_cachedPixmap.width()) / 2;
        int y = (height() - m_cachedPixmap.height()) / 2;
        painter.drawPixmap(x, y, m_cachedPixmap);
    }

    m_isUpdate = false;
}

void ImageLabel::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::StyleChange)
    {
        if(!m_isUpdate)
        {
            m_pixmapDirty = true;
            update();
        }
    }
    QLabel::changeEvent(event);
}

QSize ImageLabel::imageSize() const
{
    return m_imageSize;
}

void ImageLabel::setImageSize(const QSize &size)
{
    if(m_imageSize == size) return;

    m_imageSize = size;
    m_pixmapDirty = true;
    update();
    emit imageSizeChanged(size);
}

void ImageLabel::updatePixmapFromStyle()
{
    QString imageUrl = extractImageUrlFromStyleSheet();
    m_hasSvg = false;

    if(!imageUrl.isEmpty())
    {
        // 移除可能的引号
        imageUrl.remove('\"');
        imageUrl.remove('\'');

        // 检查是否为 SVG 文件
        if(isSvgFile(imageUrl))
        {
            // 加载 SVG
            if(imageUrl.startsWith(":/")) m_hasSvg = m_svgRenderer.load(imageUrl); // 资源文件
            else m_hasSvg = m_svgRenderer.load(imageUrl); // 文件路径

            if(!m_hasSvg) m_cachedPixmap = QPixmap(); // 清空图片
        }
        else
        {
            // 加载图片
            if(imageUrl.startsWith(":/")) m_cachedPixmap = QPixmap(imageUrl); // 资源文件
            else m_cachedPixmap = QPixmap(imageUrl); // 文件路径

            // 缩放图片
            if(!m_cachedPixmap.isNull() && m_imageSize.isValid())
            {
                m_cachedPixmap = m_cachedPixmap.scaled(m_imageSize,
                                                       Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }
        }
    }
    else m_cachedPixmap = QPixmap(); // 清空图片

    m_pixmapDirty = false;
}

QString ImageLabel::extractImageUrlFromStyleSheet() const
{
    // 获取当前样式表
    QString styleSheet = this->styleSheet();

    // 使用正则表达式匹配 image 属性
    QRegularExpression regex("image\\s*:\\s*url\\(([^)]+)\\)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = regex.match(styleSheet);

    if(match.hasMatch()) return match.captured(1).trimmed();

    // 如果没有找到 image 属性，尝试匹配 background-image
    regex.setPattern("background-image\\s*:\\s*url\\(([^)]+)\\)");
    match = regex.match(styleSheet);

    if(match.hasMatch()) return match.captured(1).trimmed();

    return QString();
}

bool ImageLabel::isSvgFile(const QString &filePath) const
{
    return filePath.endsWith(".svg", Qt::CaseInsensitive);
}
