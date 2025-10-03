#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include <QFont>
#include <QObject>
#include <QSet>

class FontManager : public QObject
{
    Q_OBJECT
public:
    // 单例模式
    static FontManager *getFontManager()
    {
        static FontManager f;
        return &f;
    }
    // 删除拷贝构造函数和赋值运算符
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;

    void setBaseFont(const QFont &font);
    void setBaseFont(const QString &family, int pointSize);
    QFont baseFont() const;

    void registerWidget(QWidget *widget);
    void unregisterWidget(QWidget *widget);


    void updateAllWidgetsFont();

signals:
    void fontChanged(const QFont &font);

private:
    explicit FontManager(QObject *parent = nullptr);
    ~FontManager();

    QFont m_baseFont;
    QSet<QWidget*> m_registeredWidgets;
};

#endif // FONTMANAGER_H
