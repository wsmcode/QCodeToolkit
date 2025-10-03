#ifndef SEARCHBOX_H
#define SEARCHBOX_H

#include <QLineEdit>

class QListWidget;
class QStringListModel;
class PopoverWidget;
class SearchBox : public QLineEdit
{
    Q_OBJECT
public:
    explicit SearchBox(QWidget *parent = nullptr);
    ~SearchBox();

signals:
    void openNote(const QString &fullPath);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *) override;

private slots:
    void onTextChanged(const QString &text);
    void onPopoverActivated(const QString &text);
//    void onSuggestionItemClicked(QListWidgetItem *item);
//    void performSearch();

private:
    void loadSearchHistory();
    void saveSearchHistory();

    QString formatSuggestion(int nodeId, const QString &projectName) const;
    bool isHistoryItem(const QString &text) const;

    PopoverWidget *m_popover;
    QStringList m_searchHistory;

    QMap<QString, int> m_nodeIdFromSuggestion;
};

#endif // SEARCHBOX_H
