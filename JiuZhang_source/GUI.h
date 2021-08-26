#ifndef MYJZ_519_H
#define MYJZ_519_H

#include <QtWidgets/QMainWindow>
#include <qmenubar.h>
#include <qmenu.h>
#include <qaction.h>
#include <qtextedit.h>
#include <qdialog.h>
#include <highlighter.h>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QObject>
#include <QTreeWidget>
#include <QWidget>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QTreeWidgetItem>
#include <QLabel>
#include <QPushButton>

void selectedPro(QString s);
QT_BEGIN_NAMESPACE
namespace Ui { class myjz_519; }
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class QSessionManager;
QT_END_NAMESPACE

class CodeEditor;
class LineNumberArea;
class TreeWidget;

class myjz_519 : public QMainWindow
{
    Q_OBJECT

public:
    myjz_519(QWidget *parent = nullptr);
    ~myjz_519();

    int inputNum;
    void edit_output(std::string output_string);

    void loadFile(const QString &fileName);//加载文件内容到编辑器

    void TreeWidget(QTreeWidget *treeWidget);
    void creaTreeMenu();
    void contextMenuEvent(QContextMenuEvent *event) override;
    QTextEdit * fakeTextEdit;
    QPushButton * inputButton;

protected:
    void closeEvent(QCloseEvent *event) override;


private:
    void CreateUi();
    void createMenu();

    void createActions();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    bool maybeSave();
    bool saveFile(const QString &fileName);//保存文件
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);

    // Code Editor
    void IsEmpty(); //判断文本编辑器是否为空
    bool file_save(); //文件保存


private slots:
    // Code Editor
    void newFile();
    void openProject(); // 打开已存在的九章项目文件夹
    bool save();
    bool saveAs();
    void about();
    void documentWasModified();
    //void open(); 不再需要了，被openProject代替
#ifndef QT_NO_SESSIONMANAGER
    void commitData(QSessionManager &);
#endif

    // Other Editor Functions
    void AutoTransfer();
    void SetFont();

    void FindTextW();

    void Results_Show();  //显示查找结果
    void render_show();   //渲染
    // Run Project
    void RunClicked();
    void inputClicked();

    // Tree Widget
    void onRunPro(bool); //运行项目

    void onAddPro(bool); //添加项目

    void onAddFile(bool); //添加文件

    void onRename(bool); //重命名项目或文件

    void onCloseProFile(bool); //关闭项目、文件

    void onDelProFile(bool); //删除项目、文件

    void onDoubleClicked(QTreeWidgetItem*,int);

    bool DeleteFileOrFolder(const QString &strPath);
    //QString checkself(QTreeWidgetItem *item);


private:
    Ui::myjz_519 *ui;
    QMenuBar *menuBar;   //file menue
/***********************************************/
    QMenu *fileMenu;
    QAction *newfile;
    QAction *openfile;
    QAction *savefile;
    QAction *saveasfile;
    QAction *out;

/***********************************************/
    QMenu *editorMenu;
    QAction *Cut;
    QAction *Copy;
    QAction *Paste;

/***********************************************/
    QMenu *formatMenu;//geshi

    QAction *Auto; //change line automatially
    QAction *Font; // the font of words

/***********************************************/
    QMenu *selectMenu;
    QAction *select;
    QAction *render;
/***********************************************/
    QMenu *helpMenu;
    QAction *About;

/***********************************************/
    //QPlainTextEdit *textedit;
    CodeEditor * textedit;
    //QTextEdit *textedit;
    QLineEdit * inputLine;
    Highlighter *highlighter;//设置语法高亮

/*************New File**********************************/
    bool isSaved;  //是否保存
    QString filename; //文件路径
    QString curFile;//当前文件目录


    //内容查找
    QDialog *finddlg; //查找窗口
    QLineEdit *findedit; //查找的内容

    QDialog *renderWin; // 渲染窗口
    QTextEdit *renderLbl;


    //编译按钮
    QPushButton *runThisFile; //编译文件

/*************Tree Widget**********************************/
    QTreeWidget *treeWidget;

    void initTreeWidget();

    QMenu *treeMenu;
    QMenu *itemMenu;

    QAction *run_action;
    QAction *add_project;
    QAction *add_file;
    QAction *rename_action;
    QAction *close_action;
    QAction *del_action;

};






/*************************************************************************************
 *************************************************************************************
 *************************************************************************************
 *************************************************************************************
 ************************************害怕**********************************************
 *************************************************************************************
 *************************************************************************************
 *************************************************************************************
 *************************************************************************************
 */
class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    CodeEditor(QWidget *parent = 0);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);

private:
    QWidget *lineNumberArea;
};

//![codeeditordefinition]
//![extraarea]

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CodeEditor *editor) : QWidget(editor) {
        codeEditor = editor;
    }

    QSize sizeHint() const override {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor *codeEditor;
};

#include <string>
std::string StringToUtf(std::string strValue);

#endif // MYJZ_519_H
