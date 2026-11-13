#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>

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
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 文件操作
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionExit_triggered();

    // 编辑操作
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionCut_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionFind_triggered();
    void on_actionReplace_triggered();
    void on_actionSelectAll_triggered();

    // 格式操作
    void on_actionWordWrap_triggered(bool checked);
    void on_actionShowLineNumber_triggered(bool checked); // 必须有
    void on_actionFont_triggered();
    void on_actionFontColor_triggered();
    void on_actionFontBgColor_triggered();
    void on_actionEditorBgColor_triggered();

    // 查看与帮助
    void on_actionToolBar_triggered(bool checked);
    void on_actionStatusBar_triggered(bool checked);
    void on_actionAbout_triggered();

    // 状态更新
    void updateStatusBar();
    void updateEditActions();
    void updateWindowTitle();

private:
    Ui::MainWindow *ui;
    QString m_currentFile;

    bool maybeSave();
    bool saveToFile(const QString &fileName);
    void initStatusBar();
    bool doFindNext(const QString &text, bool caseSensitive, bool searchUp);
};
#endif // MAINWINDOW_H
