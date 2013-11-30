#ifndef BTCGUITEST_H
#define BTCGUITEST_H


#include <QWidget>

namespace Ui
{
    class btcguitest;
}


class btcguitest : public QWidget
{
public:
    btcguitest();

    explicit btcguitest(QWidget *parent = 0);
    ~btcguitest();

private:
    Q_OBJECT
    Ui::btcguitest *ui;

private slots:
    void on_testButton_clicked();
};

#endif // BTCGUITEST_H
