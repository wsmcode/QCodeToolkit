#ifndef CODE_TYPES_H
#define CODE_TYPES_H
//#include <QObject>
#include <QString>
#include <QList>
#include <QMap>

// 内容类型枚举
enum class NoteContentType {
    Text,       // 纯文本
    Image,      // 图片
    Markdown,   // Markdown格式
    Code        // 代码段
};

using NType = NoteContentType;

// 单项内容
struct NoteItem { // 默认纯文本
    NoteContentType type;
    QString content;
    QString language;    // 代码语言
    // double 缩放因子
    // 字体大小 // 可以改成整体
    NoteItem() : type(NType::Text), content(""), language(""){}
};
//Q_DECLARE_METATYPE(BriefItem)

//struct NoteTags { // 如果QMap不好用，用这个
//    QString tagGroup;
//    QStringList tags;
//    NoteTags() : tagGroup(""), tags(QStringList()){}
//};
//Q_DECLARE_METATYPE(NoteTags)

struct CodeNote {
    QMap<QString, QStringList> tags; // 标签组，键为组名，值为标签列表
    QList<NoteItem> note;      // 内容

    bool isEmpty() const
    {
        return tags.isEmpty() && note.isEmpty();
    }
};
//Q_DECLARE_METATYPE(CodeNote)

#endif // CODE_TYPES_H
