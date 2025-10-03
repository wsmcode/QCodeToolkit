#include "imageitemcell.h"
#include "stylemanager.h"

#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QDrag>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QMovie>
#include <QSettings>
#include <QStandardPaths>

ImageItemCell::ImageItemCell(NoteItem item, QWidget *parent)
    : ItemCell(item, parent)
{
    initUI();
    setAcceptDrops(true);

    setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(this, &ImageItemCell::customContextMenuRequested,
                     this, &ImageItemCell::onCustomContextMenu);

}

ImageItemCell::~ImageItemCell()
{
    if(m_movie)
    {
        m_movie->stop();
        delete m_movie;
        m_movie = nullptr;
    }
}

void ImageItemCell::initUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(5);

    // 图片标签
    m_imageLabel = new QLabel;
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setObjectName("NoteItemImageLabel");
    StyleManager::getStyleManager()->registerWidget(m_imageLabel);
    m_imageLabel->setMinimumHeight(150);
    m_imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mainLayout->addWidget(m_imageLabel);

    updateContent();
}

void ImageItemCell::updateContent()
{
    // 停止并清理之前的动画
    if(m_movie)
    {
        m_movie->stop();
        delete m_movie;
        m_movie = nullptr;
    }

    if(m_item.content.isEmpty())
    {
        m_originalPixmaps = QPixmap();
        m_imageLabel->setText("右键添加图片");
        m_isAnimated = false;
    }
    else
    {
        // 检查文件是否为GIF动画
        QFileInfo fileInfo(m_item.content);
        QString extension = fileInfo.suffix().toLower();
        m_isAnimated = (extension == "gif");

        if(m_isAnimated) setupMovie(m_item.content);
        else
        {
            QPixmap pixmap;
            if(pixmap.load(m_item.content))
            {
                m_originalPixmaps = pixmap;
                // 缩放图片以适应标签
                scaleImages();
            }
            else
            {
                m_originalPixmaps = QPixmap();
                m_imageLabel->setText(QString("无法加载图片: %1").arg(m_item.content));
            }
        }
    }
}

void ImageItemCell::resizeEvent(QResizeEvent *event)
{
    ItemCell::resizeEvent(event);
    if(!m_isAnimated) scaleImages();
    else if(m_movie) scaleMovieFrame(); // 缩放当前帧
}

void ImageItemCell::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && !m_item.content.isEmpty())
    {
        m_dragStartPosition = event->pos();
    }
    QWidget::mousePressEvent(event);
}

void ImageItemCell::mouseMoveEvent(QMouseEvent *event)
{
    if(!(event->buttons() & Qt::LeftButton)) // 不是鼠标左键
    {
        QWidget::mouseMoveEvent(event);
        return;
    }

    if((event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance())
    {
        QWidget::mouseMoveEvent(event);
        return;
    }

    if(!m_item.content.isEmpty()) startDrag();
}

void ImageItemCell::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasUrls() || event->mimeData()->hasImage())
    {
        event->acceptProposedAction(); // 接受系统建议的拖拽操作（通常是复制）
    }
}

void ImageItemCell::dragMoveEvent(QDragMoveEvent *event)
{
    if(event->mimeData()->hasUrls() || event->mimeData()->hasImage())
    {
        event->acceptProposedAction();
    }
}

void ImageItemCell::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();

    if(mimeData->hasImage())
    {
        // 从剪贴板或其他应用的图片数据
        QPixmap pixmap = qvariant_cast<QPixmap>(mimeData->imageData());
        if(!pixmap.isNull())
        {
            // 保存到临时文件
            QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
            QString fileName = tempDir + "/dropped_image_" +
                              QString::number(QDateTime::currentSecsSinceEpoch()) + ".png";

            if(pixmap.save(fileName, "PNG"))
            {
                m_item.content = fileName;
                updateContent();
                emit contentChanged(m_item);
                event->acceptProposedAction();
            }
        }
    }
    else if(mimeData->hasUrls())
    {
        // 从文件系统拖拽的图片文件
        QList<QUrl> urlList = mimeData->urls();
        if(!urlList.isEmpty())
        {
            QUrl url = urlList.first();
            if(loadImageFromUrl(url))
                event->acceptProposedAction();
        }
    }
}

void ImageItemCell::selectImage()
{
    // 获取应用程序设置
    QSettings settings("QCodeToolkit");

    // 获取上次使用的目录，如果没有则使用系统图片目录
    QString lastDir = settings.value("lastImageDir", "").toString();
    // 确保目录存在
    QDir dir(lastDir);
    if(!dir.exists())
    {
        lastDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    }

    QString fileName = QFileDialog::getOpenFileName(this, "选择图片", lastDir,
                                                   "图片文件 (*.png *.jpg *.jpeg "
                                                   "*.bmp *.gif *.svg "
                                                   "*.webp *.ico)");
    if(!fileName.isEmpty())
    {
        // 保存本次使用的目录
        QFileInfo fileInfo(fileName);
        settings.setValue("lastImageDir", fileInfo.path());

        m_item.content = fileName;
        updateContent();
        emit contentChanged(m_item);
    }
}

void ImageItemCell::removeImage()
{
    m_item.content.clear();
    updateContent();
    emit contentChanged(m_item);
}

//void ImageItemCell::copyImage()
//{
//    if(m_item.content.isEmpty()) return;

//    if(m_isAnimated && m_movie)
//    {
//        // 复制GIF动画的当前帧
//        QPixmap currentFrame = m_movie->currentPixmap();
//        if(!currentFrame.isNull())
//        {
//            QApplication::clipboard()->setPixmap(currentFrame);
//        }
//    }
//    else if(!m_originalPixmaps.isNull())
//    {
//        // 复制静态图片
//        QApplication::clipboard()->setPixmap(m_originalPixmaps);
//    }
//}

void ImageItemCell::updateAnimation()
{
    if(m_movie && m_movie->isValid())
    {
        scaleMovieFrame();
    }
}

void ImageItemCell::onCustomContextMenu(const QPoint &pos)
{
    QMenu menu;

    menu.addAction("选择图片", this, [=](){selectImage();});
    if(!m_item.content.isEmpty())
    {
        menu.addAction("移除图片", this, [=](){removeImage();});
//        menu.addAction("复制图片", this, [=](){copyImage();}, QKeySequence::Copy);
    }

    if(!m_fixed)
    {
        menu.addSeparator();
        menu.addAction("移除项", this, [=](){emit removeRequested();});
    }

    menu.exec(mapToGlobal(pos));
}

void ImageItemCell::scaleImages(double scaleFactor)
{
    m_scaleFactors = scaleFactor;

    if(m_originalPixmaps.isNull()) return;

    // 计算缩放后的尺寸，保持宽高比
    QSize newSize = m_originalPixmaps.size() * scaleFactor;
    if(newSize.width() > m_imageLabel->width())
    {
        newSize.setWidth(m_imageLabel->width());
        newSize.setHeight(m_originalPixmaps.size().height() * m_imageLabel->width() /
                          m_originalPixmaps.size().width());
    }

    QPixmap scaledPix = m_originalPixmaps.scaled(newSize, Qt::KeepAspectRatio, Qt::SmoothTransformation );
    m_imageLabel->setPixmap(scaledPix);
}

void ImageItemCell::scaleMovieFrame()
{
    if(!m_movie || !m_movie->isValid()) return;

    QPixmap currentFrame = m_movie->currentPixmap();
    if(currentFrame.isNull()) return;

    // 计算缩放后的尺寸，保持宽高比
    QSize newSize = currentFrame.size();
    if(newSize.width() > m_imageLabel->width())
    {
        newSize.setWidth(m_imageLabel->width());
        newSize.setHeight(currentFrame.size().height() * m_imageLabel->width() /
                          currentFrame.size().width());
    }

    QPixmap scaledPix = currentFrame.scaled(newSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_imageLabel->setPixmap(scaledPix);
}

void ImageItemCell::setupMovie(const QString &fileName)
{
    m_movie = new QMovie(fileName);
    m_movie->setCacheMode(QMovie::CacheAll);

    // 连接帧变化信号
    QObject::connect(m_movie, &QMovie::frameChanged, this, &ImageItemCell::updateAnimation);
    // 开始播放动画
    m_movie->start();

    // 设置第一帧
    if(m_movie->isValid()) scaleMovieFrame(); // 立即更新第一帧
    else
    {
        m_imageLabel->setText("无法加载GIF动画");
        delete m_movie;
        m_movie = nullptr;
        m_isAnimated = false;
    }
}

void ImageItemCell::startDrag()
{
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    // 设置图片数据
    if(m_isAnimated && m_movie)
    {
        QPixmap dragPixmap = m_movie->currentPixmap();
        if(!dragPixmap.isNull())
            mimeData->setImageData(dragPixmap.toImage());

    }
    else if(!m_originalPixmaps.isNull())
        mimeData->setImageData(m_originalPixmaps.toImage());

    // 设置文件路径
    QList<QUrl> urls;
    urls.append(QUrl::fromLocalFile(m_item.content));
    mimeData->setUrls(urls);

    drag->setMimeData(mimeData);

    // 设置拖拽时的缩略图
    QPixmap dragPixmap;
    if(m_isAnimated && m_movie) dragPixmap = m_movie->currentPixmap();
    else dragPixmap = m_originalPixmaps;

    if(!dragPixmap.isNull())
    {
        // 缩放拖拽图片为合适大小
        QSize dragSize = dragPixmap.size();
        if(dragSize.width() > 200 || dragSize.height() > 200)
        {
            dragSize.scale(200, 200, Qt::KeepAspectRatio);
            dragPixmap = dragPixmap.scaled(dragSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        drag->setPixmap(dragPixmap);
        drag->setHotSpot(QPoint(dragPixmap.width()/2, dragPixmap.height()/2)); // 设置拖拽图片与鼠标指针的相对位置
    }

    drag->exec(Qt::CopyAction);
}

bool ImageItemCell::loadImageFromUrl(const QUrl &url)
{
    if(url.isLocalFile())
    {
        QString filePath = url.toLocalFile();

        // 检查文件是否为支持的图片格式
        QStringList imageExtensions = {"png", "jpg", "jpeg", "bmp", "gif", "svg", "webp", "ico"};
        QFileInfo fileInfo(filePath);
        if(imageExtensions.contains(fileInfo.suffix().toLower()))
        {
            m_item.content = filePath;
            updateContent();
            emit contentChanged(m_item);
            return true;
        }
    }
    return false;
}
