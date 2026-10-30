#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , currentInput("0")
    , firstOperand("")
    , pendingOperator("")
    , waitingForOperand(true)
{
    ui->setupUi(this);
    this->setStyleSheet(R"(
        QMainWindow, QWidget#centralwidget {
            background-color: #ffffff;
        }
        QLineEdit {
            font-size: 36px;
            background-color: #ffffff;
            border: none;
            padding-right: 10px;
        }
        QPushButton {
            font-size: 20px;
            background-color: #f9f9f9;
            border: 1px solid #e0e0e0;
        }
        QPushButton:hover {
            background-color: #e6e6e6;
        }
        QPushButton:pressed {
            background-color: #d4d4d4;
        }
        QPushButton#btn_equal {
            background-color: #cce4f7;
            border: 1px solid #99c2e5;
            color: #000000;
        }
        QPushButton#btn_equal:hover {
            background-color: #b3d7f2;
        }
    )");

    // 绑定按钮点击事件（使用 lambda 表达式减少代码重复）
    // 数字 0-9
    QList<QPushButton*> numBtns = {ui->btn_0, ui->btn_1, ui->btn_2, ui->btn_3, ui->btn_4,
                                   ui->btn_5, ui->btn_6, ui->btn_7, ui->btn_8, ui->btn_9};
    for (int i = 0; i < numBtns.size(); ++i) {
        connect(numBtns[i], &QPushButton::clicked, this, [=]() { onDigitClicked(i); });
    }

    // 运算符
    connect(ui->btn_add, &QPushButton::clicked, this, [=]() { onOperatorClicked("+"); });
    connect(ui->btn_sub, &QPushButton::clicked, this, [=]() { onOperatorClicked("-"); });
    connect(ui->btn_mul, &QPushButton::clicked, this, [=]() { onOperatorClicked("*"); });
    connect(ui->btn_div, &QPushButton::clicked, this, [=]() { onOperatorClicked("/"); });

    // 功能键
    connect(ui->btn_dot, &QPushButton::clicked, this, &MainWindow::onDotClicked);
    connect(ui->btn_equal, &QPushButton::clicked, this, &MainWindow::onEqualClicked);
    connect(ui->btn_back, &QPushButton::clicked, this, &MainWindow::onBackspaceClicked);
    connect(ui->btn_clear, &QPushButton::clicked, this, &MainWindow::onClearAllClicked);
    connect(ui->btn_ce, &QPushButton::clicked, this, &MainWindow::onClearClicked);

    // 特殊运算
    connect(ui->btn_negate, &QPushButton::clicked, this, [=]() { onUnaryOperatorClicked("negate"); });
    connect(ui->btn_reciprocal, &QPushButton::clicked, this, [=]() { onUnaryOperatorClicked("reciprocal"); });
    connect(ui->btn_square, &QPushButton::clicked, this, [=]() { onUnaryOperatorClicked("square"); });
    connect(ui->btn_sqrt, &QPushButton::clicked, this, [=]() { onUnaryOperatorClicked("sqrt"); });
    connect(ui->btn_percent, &QPushButton::clicked, this, &MainWindow::onPercentClicked);

    updateDisplay();
}

MainWindow::~MainWindow()
{
    delete ui;
}

//键盘事件处理
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();

    // 数字键 0-9
    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
        onDigitClicked(key - Qt::Key_0);
    }
    // 小数点
    else if (key == Qt::Key_Period) {
        onDotClicked();
    }
    // 运算符
    else if (key == Qt::Key_Plus) {
        onOperatorClicked("+");
    }
    else if (key == Qt::Key_Minus) {
        onOperatorClicked("-");
    }
    else if (key == Qt::Key_Asterisk) {
        onOperatorClicked("*");
    }
    else if (key == Qt::Key_Slash) {
        onOperatorClicked("/");
    }
    // 等号与回车
    else if (key == Qt::Key_Equal || key == Qt::Key_Enter || key == Qt::Key_Return) {
        onEqualClicked();
    }
    // 退格
    else if (key == Qt::Key_Backspace) {
        onBackspaceClicked();
    }
    // 清除（Esc 键）
    else if (key == Qt::Key_Escape) {
        onClearAllClicked();
    }
    else {
        QMainWindow::keyPressEvent(event); // 其他按键交给父类处理
    }
}

//核心逻辑

// 数字输入
void MainWindow::onDigitClicked(int digit)
{
    if (waitingForOperand) {
        currentInput = QString::number(digit);
        waitingForOperand = false;
    } else {
        if (currentInput == "0") {
            currentInput = QString::number(digit);
        } else {
            currentInput += QString::number(digit);
        }
    }
    updateDisplay();
}

// 小数点输入（防止重复输入小数点）
void MainWindow::onDotClicked()
{
    if (waitingForOperand) {
        currentInput = "0.";
        waitingForOperand = false;
    } else if (!currentInput.contains(".")) {
        currentInput += ".";
    }
    updateDisplay();
}

// 运算符输入（处理连续运算符）
void MainWindow::onOperatorClicked(QString op)
{
    if (!pendingOperator.isEmpty() && !waitingForOperand) {
        // 如果已有待处理运算符，先计算前一个结果
        calculate();
    }

    if (pendingOperator.isEmpty() || !waitingForOperand) {
        firstOperand = currentInput;
        pendingOperator = op;
        waitingForOperand = true;
    } else {
        // 如果连续输入运算符，则替换前一个运算符
        pendingOperator = op;
    }
    updateDisplay();
}

// 等号计算
void MainWindow::onEqualClicked()
{
    if (pendingOperator.isEmpty()) return;
    calculate();
    pendingOperator = "";
    waitingForOperand = true;
    updateDisplay();
}

// 退格
void MainWindow::onBackspaceClicked()
{
    if (waitingForOperand) return;

    currentInput.chop(1); // 删除最后一个字符
    if (currentInput.isEmpty() || currentInput == "-") {
        currentInput = "0";
        waitingForOperand = true;
    }
    updateDisplay();
}

// 清除当前输入（CE）
void MainWindow::onClearClicked()
{
    currentInput = "0";
    waitingForOperand = true;
    updateDisplay();
}

// 全部清除（C）
void MainWindow::onClearAllClicked()
{
    currentInput = "0";
    firstOperand = "";
    pendingOperator = "";
    waitingForOperand = true;
    updateDisplay();
}

// 一元运算（+/-、1/x、x²、√x）
void MainWindow::onUnaryOperatorClicked(QString op)
{
    double value = currentInput.toDouble();
    if (op == "negate") {
        value = -value;
    } else if (op == "reciprocal") {
        if (value == 0) { handleError("除数不能为零"); return; }
        value = 1.0 / value;
    } else if (op == "square") {
        value = value * value;
    } else if (op == "sqrt") {
        if (value < 0) { handleError("负数不能开平方"); return; }
        value = std::sqrt(value);
    }
    currentInput = QString::number(value, 'g', 12); // 保留精度
    waitingForOperand = true;
    updateDisplay();
}

// 百分号
void MainWindow::onPercentClicked()
{
    double value = currentInput.toDouble();
    currentInput = QString::number(value / 100.0, 'g', 12);
    waitingForOperand = true;
    updateDisplay();
}

// 计算核心逻辑
void MainWindow::calculate()
{
    double op1 = firstOperand.toDouble();
    double op2 = currentInput.toDouble();
    double result = 0.0;

    if (pendingOperator == "+") {
        result = op1 + op2;
    } else if (pendingOperator == "-") {
        result = op1 - op2;
    } else if (pendingOperator == "*") {
        result = op1 * op2;
    } else if (pendingOperator == "/") {
        if (op2 == 0.0) { handleError("除数不能为零"); return; }
        result = op1 / op2;
    } else {
        return;
    }

    currentInput = QString::number(result, 'g', 12); // 自动去除多余的小数点和零
    firstOperand = currentInput;
    waitingForOperand = true;
}

// 更新界面显示
void MainWindow::updateDisplay()
{
    QString text;

    if (pendingOperator.isEmpty()) {
        // 如果没有任何待处理的运算符，只显示当前数字
        text = currentInput;
    } else {
        // 如果有运算符，先显示第一个操作数和运算符（例如 "5 +"）
        text = firstOperand + " " + pendingOperator;

        // 只有当第二个操作数已经开始输入了（waitingForOperand 为 false），才把第二个数拼上去
        if (!waitingForOperand) {
            text += " " + currentInput;
        }
    }

    ui->display->setText(text);
}

// 错误处理
void MainWindow::handleError(const QString &msg)
{
    currentInput = "Error";
    firstOperand = "";
    pendingOperator = "";
    waitingForOperand = true;
    ui->display->setText(msg);
}
