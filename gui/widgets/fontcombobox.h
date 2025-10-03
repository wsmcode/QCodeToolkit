#ifndef FONTCOMBOBOX_H
#define FONTCOMBOBOX_H

#include <QTimer>
#include <combobox.h>

class FontComboBox : public ComboBox
{
    Q_OBJECT
public:
    explicit FontComboBox(QWidget *parent = nullptr);

signals:

protected:
    void showPopup() override;
    void hidePopup() override;

private slots:
    void loadFonts();
    void clearFonts();

private:
    void initTimer();

    QTimer m_clearTimer;
    bool m_fontsLoaded = false;

};

#endif // FONTCOMBOBOX_H
