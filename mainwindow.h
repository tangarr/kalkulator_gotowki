#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHash>
#include <QPair>

class QSpinBox;
class QLineEdit;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSpinBoxValueChanged(int value);
    void onPrintPressed();
    void onFocusChanged(QWidget *oldWidget, QWidget *newWidget);

private:
    void recalculateSum();

private:
    Ui::MainWindow *ui;
    QHash<QSpinBox*, QPair<unsigned int, QLineEdit*>> mMapping;

};
#endif // MAINWINDOW_H
