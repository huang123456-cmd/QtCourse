#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // 键盘事件处理
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    // 数字与小数点
    void onDigitClicked(int digit);
    void onDotClicked();
    // 运算符与等号
    void onOperatorClicked(QString op);
    void onEqualClicked();
    // 编辑与清除
    void onBackspaceClicked();
    void onClearClicked();
    void onClearAllClicked();
    // 特殊运算
    void onUnaryOperatorClicked(QString op);
    void onPercentClicked();

private:
    Ui::MainWindow *ui;

    // 计算器核心状态
    QString currentInput;    // 当前正在输入的数字字符串
    QString firstOperand;    // 第一个操作数
    QString pendingOperator; // 待执行的运算符
    bool waitingForOperand;  // 是否等待输入新操作数（用于连续运算和计算后继续输入）

    // 辅助函数
    void calculate();
    void updateDisplay();
    void handleError(const QString &msg);
};
#endif // MAINWINDOW_H
