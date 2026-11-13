#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QFontDialog>
#include <QColorDialog>
#include <QCloseEvent>
#include <QClipboard>
#include <QApplication>
#include <QTextCursor>
#include <QLabel>
#include <QToolBar>
#include <QIcon>
#include <QStyle>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setStyleSheet(
            "QToolButton:checked {"
            "   background-color: #0078D7;"
            "   color: white;"
            "   border: 1px solid #005A9E;"
            "   border-radius: 3px;"
            "}"
            "QMenu::item:checked {"
            "   background-color: #0078D7;"
            "   color: white;"
            "}"
            "QMenu::item:selected {"
            "   background-color: #E0E0E0;"
            "   color: black;"
            "}"
        );
    ui->actionNew->setIcon(QIcon(":/icon/actionNew.png"));
    ui->actionOpen->setIcon(QIcon(":/icon/actionOpen.png"));
    ui->actionSave->setIcon(QIcon(":/icon/actionSave.png"));
    ui->actionSaveAs->setIcon(QIcon(":/icon/actionSaveAs.png"));
    ui->actionExit->setIcon(QIcon(":/icon/actionExit.png"));

    ui->actionUndo->setIcon(QIcon(":/icon/actionUndo.png"));
    ui->actionRedo->setIcon(QIcon(":/icon/actionRedo.png"));
    ui->actionCut->setIcon(QIcon(":/icon/actionCut.png"));
    ui->actionCopy->setIcon(QIcon(":/icon/actionCopy.png"));
    ui->actionPaste->setIcon(QIcon(":/icon/actionPaste.png"));
    ui->actionFind->setIcon(QIcon(":/icon/actionFind.png"));
    ui->actionReplace->setIcon(QIcon(":/icon/actionReplace.png"));
    ui->actionSelectAll->setIcon(QIcon(":/icon/actionSelectAll.png"));

    ui->actionWordWrap->setIcon(QIcon(":/icon/actionWordWrap.png"));
    ui->actionShowLineNumber->setIcon(QIcon(":/icon/actionShowLineNumber.png"));
    ui->actionFont->setIcon(QIcon(":/icon/actionFont.png"));

    ui->actionStatusBar->setIcon(QIcon(":/icon/actionStatusBar.png"));
    ui->ToolBar->setIcon(QIcon(":/icon/ToolBar.png"));

    ui->actionWordWrap->setCheckable(true);
      ui->actionWordWrap->setChecked(true);

      ui->actionShowLineNumber->setCheckable(true);
      ui->actionShowLineNumber->setChecked(false);

      // 如果工具栏开关和状态栏开关也需要高亮
      if(ui->ToolBar) {
          ui->ToolBar->setCheckable(true);
          ui->ToolBar->setChecked(true);
      }
      if(ui->actionStatusBar) {
          ui->actionStatusBar->setCheckable(true);
          ui->actionStatusBar->setChecked(true);
      }

    // 查找并初始化工具栏/状态栏开关
    QList<QAction*> allActions = this->findChildren<QAction*>();
    for(QAction *act : allActions) {
        if(act->objectName().contains("ToolBar", Qt::CaseInsensitive) || act->text().contains("工具栏")) {
            act->setCheckable(true);
            act->setChecked(true);
        }
        if(act->objectName().contains("StatusBar", Qt::CaseInsensitive) || act->text().contains("状态栏")) {
            act->setCheckable(true);
            act->setChecked(true);
        }
    }

    // 1. 初始化窗口标题
    setWindowTitle("新建文本文件[*] - 编辑器");
    ui->textEdit->document()->setModified(false);

    // 2. 初始化状态栏
    initStatusBar();

    // 3. 连接文档修改信号，控制标题上的 * 号
    connect(ui->textEdit->document(), &QTextDocument::modificationChanged,
            this, &QWidget::setWindowModified);

    // 4. 连接各种信号，实时更新 Action 的使能状态
    connect(ui->textEdit, &QPlainTextEdit::copyAvailable, this, &MainWindow::updateEditActions);
    connect(ui->textEdit, &QPlainTextEdit::undoAvailable, this, &MainWindow::updateEditActions);
    connect(ui->textEdit, &QPlainTextEdit::redoAvailable, this, &MainWindow::updateEditActions);
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &MainWindow::updateEditActions);

    // 5. 连接光标移动信号，更新状态栏的行列号
    connect(ui->textEdit, &QPlainTextEdit::cursorPositionChanged, this, &MainWindow::updateStatusBar);
    connect(ui->textEdit->document(), &QTextDocument::blockCountChanged, this, &MainWindow::updateStatusBar);

    // 6. 初始化按钮状态
    updateEditActions();
    updateStatusBar();

    // 7. 默认开启自动换行
    ui->textEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ==================== 辅助函数 ====================

void MainWindow::initStatusBar()
{
    QLabel *labelLength = new QLabel("length: 0", this);
    QLabel *labelLines = new QLabel("lines: 1", this);
    QLabel *labelPos = new QLabel("Ln: 1  Col: 1", this);
    QLabel *labelInfo = new QLabel("黄梓镰", this);

    statusBar()->addWidget(labelLength);
    statusBar()->addWidget(labelLines);
    statusBar()->addPermanentWidget(labelPos);
    statusBar()->addPermanentWidget(labelInfo);
}

void MainWindow::updateWindowTitle()
{
    QString name = m_currentFile.isEmpty() ? "未命名" : QFileInfo(m_currentFile).fileName();
    setWindowTitle(name + "[*] - 编辑器");
    setWindowModified(ui->textEdit->document()->isModified());
}

void MainWindow::updateEditActions()
{
    bool hasSelection = ui->textEdit->textCursor().hasSelection();
    ui->actionCut->setEnabled(hasSelection);
    ui->actionCopy->setEnabled(hasSelection);
    ui->actionPaste->setEnabled(!QApplication::clipboard()->text().isEmpty());
    ui->actionUndo->setEnabled(ui->textEdit->document()->isUndoAvailable());
    ui->actionRedo->setEnabled(ui->textEdit->document()->isRedoAvailable());
}

void MainWindow::updateStatusBar()
{
    QTextCursor cursor = ui->textEdit->textCursor();
    int line = cursor.blockNumber() + 1;
    int col = cursor.columnNumber() + 1;
    int len = ui->textEdit->toPlainText().length();

    QList<QLabel*> labels = statusBar()->findChildren<QLabel*>();
    if(labels.size() >= 3) {
        labels[0]->setText(QString("length: %1").arg(len));
        labels[1]->setText(QString("lines: %1").arg(ui->textEdit->blockCount()));
        labels[2]->setText(QString("Ln: %1  Col: %2").arg(line).arg(col));
    }
}

bool MainWindow::maybeSave()
{
    if (!ui->textEdit->document()->isModified()) return true;

    auto ret = QMessageBox::warning(this, "未保存的修改", "文档已修改，是否保存？",
                                    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save) {
        if (m_currentFile.isEmpty()) {
            on_actionSaveAs_triggered();
            return !m_currentFile.isEmpty();
        } else {
            return saveToFile(m_currentFile);
        }
    } else if (ret == QMessageBox::Discard) {
        return true;
    }
    return false;
}

bool MainWindow::saveToFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "保存失败", file.errorString());
        return false;
    }
    QTextStream out(&file);
    out << ui->textEdit->toPlainText();
    file.close();

    m_currentFile = fileName;
    ui->textEdit->document()->setModified(false);
    updateWindowTitle();
    return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::on_actionNew_triggered()
{
    if (!maybeSave()) return;
    ui->textEdit->clear();
    m_currentFile.clear();
    ui->textEdit->document()->setModified(false);
    updateWindowTitle();
}

void MainWindow::on_actionOpen_triggered()
{
    if (!maybeSave()) return;
    QString path = QFileDialog::getOpenFileName(this, "打开文件", "", "文本文件 (*.txt);;所有文件 (*.*)");
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "打开失败", file.errorString());
        return;
    }
    QTextStream in(&file);
    ui->textEdit->setPlainText(in.readAll());
    file.close();

    m_currentFile = path;
    ui->textEdit->document()->setModified(false);
    updateWindowTitle();
}

void MainWindow::on_actionSave_triggered()
{
    if (m_currentFile.isEmpty()) {
        on_actionSaveAs_triggered();
    } else {
        saveToFile(m_currentFile);
    }
}

void MainWindow::on_actionSaveAs_triggered()
{
    QString path = QFileDialog::getSaveFileName(this, "另存为", m_currentFile, "文本文件 (*.txt);;所有文件 (*.*)");
    if (path.isEmpty()) return;
    saveToFile(path);
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

// ==================== 编辑操作 ====================

void MainWindow::on_actionUndo_triggered() { ui->textEdit->undo(); }
void MainWindow::on_actionRedo_triggered() { ui->textEdit->redo(); }
void MainWindow::on_actionCut_triggered() { ui->textEdit->cut(); }
void MainWindow::on_actionCopy_triggered() { ui->textEdit->copy(); }
void MainWindow::on_actionPaste_triggered() { ui->textEdit->paste(); }
void MainWindow::on_actionSelectAll_triggered() { ui->textEdit->selectAll(); }

// ==================== 查找与替换辅助 ====================

bool MainWindow::doFindNext(const QString &text, bool caseSensitive, bool searchUp)
{
    if (text.isEmpty()) return false;

    QTextDocument::FindFlags flags;
    if (caseSensitive) flags |= QTextDocument::FindCaseSensitively;
    if (searchUp) flags |= QTextDocument::FindBackward;

    bool found = ui->textEdit->find(text, flags);

    if (!found) {
        QTextCursor cursor = ui->textEdit->textCursor();
        if (searchUp) cursor.movePosition(QTextCursor::End);
        else cursor.movePosition(QTextCursor::Start);
        ui->textEdit->setTextCursor(cursor);
        found = ui->textEdit->find(text, flags);
    }

    if (!found) {
        QMessageBox::information(this, "查找", "未找到匹配项。");
    }
    return found;
}

void MainWindow::on_actionFind_triggered()
{
    QDialog *findDialog = new QDialog(this);
    findDialog->setWindowTitle("查找");
    findDialog->setAttribute(Qt::WA_DeleteOnClose);

    QLineEdit *findLineEdit = new QLineEdit(findDialog);
    QPushButton *btnFindNext = new QPushButton("查找下一个(F)", findDialog);
    btnFindNext->setDefault(true);

    QCheckBox *checkCase = new QCheckBox("区分大小写", findDialog);
    QRadioButton *radioUp = new QRadioButton("向上(U)", findDialog);
    QRadioButton *radioDown = new QRadioButton("向下(D)", findDialog);
    radioDown->setChecked(true);

    QGroupBox *groupBox = new QGroupBox("方向", findDialog);
    QHBoxLayout *hLayoutGroup = new QHBoxLayout(groupBox);
    hLayoutGroup->addWidget(radioUp);
    hLayoutGroup->addWidget(radioDown);

    QPushButton *btnCancel = new QPushButton("取消", findDialog);

    QGridLayout *layout = new QGridLayout(findDialog);
    layout->addWidget(new QLabel("查找目标:"), 0, 0);
    layout->addWidget(findLineEdit, 0, 1);
    layout->addWidget(btnFindNext, 0, 2);
    layout->addWidget(groupBox, 1, 1, 2, 1);
    layout->addWidget(checkCase, 2, 0);
    layout->addWidget(btnCancel, 2, 2);

    connect(btnFindNext, &QPushButton::clicked, this, [=]() {
        doFindNext(findLineEdit->text(), checkCase->isChecked(), radioUp->isChecked());
    });
    connect(btnCancel, &QPushButton::clicked, findDialog, &QDialog::close);

    findDialog->exec();
}

void MainWindow::on_actionReplace_triggered()
{
    QDialog *replaceDialog = new QDialog(this);
    replaceDialog->setWindowTitle("替换");
    replaceDialog->setAttribute(Qt::WA_DeleteOnClose);

    QLineEdit *findLineEdit = new QLineEdit(replaceDialog);
    QLineEdit *replaceLineEdit = new QLineEdit(replaceDialog);

    QPushButton *btnFindNext = new QPushButton("查找下一个(F)", replaceDialog);
    QPushButton *btnReplace = new QPushButton("替换(R)", replaceDialog);
    QPushButton *btnReplaceAll = new QPushButton("全部替换(A)", replaceDialog);

    QCheckBox *checkCase = new QCheckBox("区分大小写", replaceDialog);
    QRadioButton *radioUp = new QRadioButton("向上(U)", replaceDialog);
    QRadioButton *radioDown = new QRadioButton("向下(D)", replaceDialog);
    radioDown->setChecked(true);

    QGroupBox *groupBox = new QGroupBox("方向", replaceDialog);
    QHBoxLayout *hLayoutGroup = new QHBoxLayout(groupBox);
    hLayoutGroup->addWidget(radioUp);
    hLayoutGroup->addWidget(radioDown);

    QPushButton *btnCancel = new QPushButton("取消", replaceDialog);

    QGridLayout *layout = new QGridLayout(replaceDialog);
    layout->addWidget(new QLabel("查找目标:"), 0, 0);
    layout->addWidget(findLineEdit, 0, 1);
    layout->addWidget(btnFindNext, 0, 2);
    layout->addWidget(new QLabel("替换为:"), 1, 0);
    layout->addWidget(replaceLineEdit, 1, 1);
    layout->addWidget(btnReplace, 1, 2);
    layout->addWidget(groupBox, 2, 1);
    layout->addWidget(checkCase, 3, 0);
    layout->addWidget(btnReplaceAll, 3, 2);
    layout->addWidget(btnCancel, 4, 2);

    connect(btnFindNext, &QPushButton::clicked, this, [=]() {
        doFindNext(findLineEdit->text(), checkCase->isChecked(), radioUp->isChecked());
    });

    connect(btnReplace, &QPushButton::clicked, this, [=]() {
        if (ui->textEdit->textCursor().hasSelection()) {
            ui->textEdit->insertPlainText(replaceLineEdit->text());
        }
        doFindNext(findLineEdit->text(), checkCase->isChecked(), radioUp->isChecked());
    });

    connect(btnReplaceAll, &QPushButton::clicked, this, [=]() {
        QString target = findLineEdit->text();
        QString replacement = replaceLineEdit->text();
        if (target.isEmpty()) return;

        QTextDocument::FindFlags flags;
        if (checkCase->isChecked()) flags |= QTextDocument::FindCaseSensitively;
        if (radioUp->isChecked()) flags |= QTextDocument::FindBackward;

        QTextCursor startCursor = ui->textEdit->textCursor();
        startCursor.movePosition(QTextCursor::Start);
        ui->textEdit->setTextCursor(startCursor);

        QTextCursor editBlockCursor(ui->textEdit->document());
        editBlockCursor.beginEditBlock();
        int count = 0;

        while (ui->textEdit->find(target, flags)) {
            ui->textEdit->insertPlainText(replacement);
            count++;
            if (count > 10000) {
                QMessageBox::warning(this, "警告", "替换次数过多，可能存在死循环！已强制停止。");
                break;
            }
        }
        editBlockCursor.endEditBlock();

        QMessageBox::information(this, "替换", QString("共替换了 %1 处。").arg(count));
    });

    connect(btnCancel, &QPushButton::clicked, replaceDialog, &QDialog::close);

    replaceDialog->exec();
}

// ==================== 格式操作 ====================

void MainWindow::on_actionWordWrap_triggered(bool checked)
{
    ui->textEdit->setLineWrapMode(checked ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
}

void MainWindow::on_actionShowLineNumber_triggered(bool checked)
{
    if(checked) {
        QMessageBox::information(this, "提示", "已开启行号显示（当前版本为纯文本控件，左侧行号需用派生类实现选做功能，但右侧状态栏已实时显示行列号）。");
    }
}

void MainWindow::on_actionFont_triggered()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, ui->textEdit->font(), this);
    if (ok) {
        ui->textEdit->selectAll();
        ui->textEdit->setFont(font);
        ui->textEdit->moveCursor(QTextCursor::End);
    }
}

void MainWindow::on_actionFontColor_triggered()
{
    QColor color = QColorDialog::getColor(Qt::black, this, "选择字体颜色");
    if (color.isValid()) {
        QTextCharFormat fmt;
        fmt.setForeground(color); // 设置前景色（字体颜色）

        QTextCursor cursor = ui->textEdit->textCursor();
        if (cursor.hasSelection()) {
            // 如果选中了文字，只改变选中的文字颜色
            cursor.mergeCharFormat(fmt);
        } else {
            // 如果没有选中，改变接下来输入的文字颜色
            ui->textEdit->setCurrentCharFormat(fmt);
        }
    }
}

void MainWindow::on_actionFontBgColor_triggered()
{
    QColor color = QColorDialog::getColor(Qt::white, this, "选择字体背景色");
    if (color.isValid()) {
        QTextCharFormat fmt;
        fmt.setBackground(color);

        QTextCursor cursor = ui->textEdit->textCursor();
        if (cursor.hasSelection()) {
            cursor.mergeCharFormat(fmt);
        } else {
            ui->textEdit->setCurrentCharFormat(fmt);
        }
    }
}

void MainWindow::on_actionEditorBgColor_triggered()
{
    // 获取当前编辑器底色
    QColor color = QColorDialog::getColor(ui->textEdit->palette().base().color(), this, "选择编辑器背景色");
    if (color.isValid()) {
        ui->textEdit->setStyleSheet(QString("QPlainTextEdit { background-color: %1; }").arg(color.name()));
    }
}

// ==================== 查看与帮助 ====================

void MainWindow::on_actionToolBar_triggered(bool checked)
{
    QList<QToolBar*> toolbars = this->findChildren<QToolBar*>();
    for(QToolBar *tb : toolbars) {
        tb->setVisible(checked);
    }
}

void MainWindow::on_actionStatusBar_triggered(bool checked)
{
    statusBar()->setVisible(checked);
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "关于",
        "<h2>文本编辑器</h2>"
        "<p>开发者：黄梓镰</p>"
        "<p>学号：2024414290213</p>"
        "<p>email: 1793482317@qq.com</p>");
}
