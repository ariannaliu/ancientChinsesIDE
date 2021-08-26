#include "GUI.h"
#include "ui_GUI.h"
#include "number.h"
#include "Compiler.h"
#include "assember.h"
#include "linker_l.h"
#include "simulator.h"
#include "toMIPS.h"
#include <QDir>
#include <QDebug>
#include <QDockWidget>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QFontDialog>
#include <QFont>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QIcon>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPainter>
#include <QPushButton>
#include <QSplitter>
#include <QTextBlock>
#include <QTextStream>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QWidget>
#include <QtWidgets>


std::map<std::string, std::string> simple_classic = {
    {"数", "數"},{"赋", "賦"},{"关", "関"},{"减", "減"},{"七", "柒"},{"长", "長"},
    {"一", "壹"},{"二", "贰"},{"三", "叁"},{"四", "肆"},{"五", "伍"},{"六", "陆"},
    {"八", "捌"},{"九", "玖"},{"十", "拾"},{"百", "佰"},{"千", "仟"},{"万", "萬"},
    {"闻", "聞"},{"亏", "虧"},{"当", "當"},{"宽", "寬"},{"兔", "兔"},{"现", "現"},
    {"继", "繼"},{"离", "離"},{"负", "負"},{"鸡", "雞"},
    {"毕", "畢"},{"损", "損"},{"术", "術"},{"为", "為"},
    {"鸭", "鴨"},{"猫", "貓"},{"猪", "豬"},{"鱼", "魚"},
    {"车", "車"},{"马", "馬"},{"东", "東"},{"儿", "兒"},
};


myjz_519::myjz_519(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::myjz_519)
{
    ui->setupUi(this);
    this->setWindowState(Qt::WindowMaximized);
    //this->setWindowTitle(QString::fromUtf8("九章算术文言文编辑器[*]"));
    this->setWindowIcon(QIcon("Resources\\nine.png"));
    CreateUi();
}

myjz_519::~myjz_519()
{
    delete ui;
}

void myjz_519::CreateUi(){

    /*****************************************************************
     ***********************Right Layout******************************
     *****************************************************************/
    QSplitter * rightSplitter = new QSplitter(Qt::Vertical, 0);

    /* Text Editor (up) */
    textedit = new CodeEditor;
    textedit->setStyleSheet("QPlainTextEdit{font-size:25px; background:write url(Resources//simple.png)}");
    //setCentralWidget(textedit);
    highlighter = new Highlighter(textedit->document());//设置语法高亮
    //createActions();
    createStatusBar();

    readSettings();

    connect(textedit->document(), &QTextDocument::contentsChanged,
            this, &myjz_519::documentWasModified);

#ifndef QT_NO_SESSIONMANAGER
    QGuiApplication::setFallbackSessionManagementEnabled(false);
    connect(qApp, &QGuiApplication::commitDataRequest,
            this, &myjz_519::commitData);
#endif

    setCurrentFile(QString());
    setUnifiedTitleAndToolBarOnMac(true);


    /* Output Window (down) */
    //QDockWidget * dockLeft = new QDockWidget(QString::fromUtf8("项目"), this);
    QDockWidget* dockBottom = new QDockWidget(QString::fromUtf8("九章输出"), this);
    dockBottom->setMinimumHeight(80);
    dockBottom->setFeatures(QDockWidget::NoDockWidgetFeatures);

    fakeTextEdit = new QTextEdit();
    fakeTextEdit->setReadOnly(true);
    dockBottom->setWidget(fakeTextEdit);

    /* Right Layout (whole) */
    rightSplitter->addWidget(textedit);
    rightSplitter->addWidget(dockBottom);
    rightSplitter->setStretchFactor(0, 3);
    rightSplitter->setStretchFactor(1, 1);

    inputLine = new QLineEdit;
    inputButton = new QPushButton;
    QHBoxLayout * inputLayout = new QHBoxLayout;
    inputButton->setText("输入");
    inputButton->setDisabled(true);
    connect(inputButton, &QPushButton::clicked, this, &myjz_519::inputClicked);

    inputLayout->addWidget(inputLine);
    inputLayout->addWidget(inputButton);

    QVBoxLayout * rightLayout = new QVBoxLayout;
    rightLayout->addWidget(rightSplitter);
    rightLayout->addLayout(inputLayout);
    //rightLayout->setMargin(0);


    /*****************************************************************
     ***********************Left Layout*******************************
     *****************************************************************/
    QSplitter *leftSpliter = new QSplitter(Qt::Vertical, 0);

    /* Run Button (down) */
    QPushButton *btnRun=new QPushButton(this);
    //btnRun->setStyleSheet("QPushButton{font-size:25px}");//设定字体大小
    btnRun->setIcon(QIcon("Resources\\run0.png"));
    btnRun->setIconSize(QSize(50,50));
    //btnRun->setText("RUN");
    btnRun->setMaximumSize(300,85);
    btnRun->setMinimumHeight(50);

    connect(btnRun, &QPushButton::clicked, this, &myjz_519::RunClicked);

    /* Tree Widget (up) */
    treeWidget = new QTreeWidget;
    TreeWidget(treeWidget);


    /* Left Layout (whole) */
    leftSpliter->addWidget(treeWidget);
    leftSpliter->addWidget(btnRun);
    leftSpliter->setCollapsible(1, false);

    QVBoxLayout * leftLayout = new QVBoxLayout;
    leftLayout->addWidget(leftSpliter);
//    leftLayout->addWidget(treeWidget);
//    leftLayout->addWidget(btnRun);
    //leftLayout->setMargin(5);


    /*****************************************************************
     ***********************Main Layout*******************************
     *****************************************************************/
    QHBoxLayout * mainLayout = new QHBoxLayout;
    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(rightLayout);
    mainLayout->setStretch(0,1);
    mainLayout->setStretch(1,4);

    QWidget * widget = new QWidget();
    this->setCentralWidget(widget);
    widget->setLayout(mainLayout);


    /*****************************************************************
     ***********************Main Menu*********************************
     *****************************************************************/
    createMenu();
}



/*****************************************************************
 *****************************************************************
 ***********************Tree Widget*******************************
 *****************************************************************
 *****************************************************************/
void myjz_519::TreeWidget(QTreeWidget *tree) // Tree Widget总方程
{
    initTreeWidget();
    setFocusPolicy(Qt::ClickFocus);
    tree->setHeaderLabel("项目");
    connect(tree,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(onDoubleClicked(QTreeWidgetItem*, int)));
    creaTreeMenu();

}

void myjz_519::onDoubleClicked(QTreeWidgetItem* , int)// Show Selected File
{
    QTreeWidgetItem *currentItem = treeWidget->currentItem();

    if(currentItem->parent()){//若双击的是子节点，把对应文件的内容显示在editor上

        if (maybeSave()) {
            QString dir_str = currentItem->parent()->data(1,1).toString();
            QString currentItemInfo = currentItem->text(0);
            dir_str = dir_str + "/" + currentItemInfo;
            loadFile(dir_str);
        }
        return;
    }

    //若双击的是父节点，加载它名下所有文件作为子节点
    QString dir_str = currentItem->data(1,1).toString();
    QDir *dir=new QDir(dir_str);
    QStringList filter;
    QList<QFileInfo> *fileInfo=new QList<QFileInfo>(dir->entryInfoList(filter));

    //获取该项目下已有的所有子节点
    QList<QString> allTheItem;
    for(int i = 0;i < currentItem->childCount();i++)
    {
        QTreeWidgetItem *child = currentItem->child(i);//获取子节点
        QString text =child->text(0);
        allTheItem.append(text);
    }

    for(int i = 2;i<fileInfo->count(); i++)
    {
        QString newFileName = fileInfo->at(i).fileName();

        if (!allTheItem.contains(newFileName)) {
            if (!newFileName.contains(".jz")) continue;//仅显示九章文件
            QTreeWidgetItem *file0 = new QTreeWidgetItem(currentItem);
            file0->setText(0, newFileName);
            qDebug() << newFileName; //双击父节点，读取它名下的子节点，更新tree（显示该项目下所有文件的文件名）
        }
    }
}

void myjz_519::contextMenuEvent(QContextMenuEvent *e) //设定鼠标点击回馈
{
    if(treeWidget->hasFocus())
    {
        QString dir_str = treeWidget->currentItem()->data(1,1).toString();
        if (dir_str.contains("/")) {
            treeMenu->move(cursor().pos());
            treeMenu->show();
        }
        else {
            itemMenu->move(cursor().pos());
            itemMenu->show();
        }
    }
    e->accept();
}

void myjz_519::initTreeWidget() // 初始化Tree Widget
{
    QTreeWidgetItem *pro0 = new QTreeWidgetItem(treeWidget);
    pro0->setText(0, "新项目");

    QString dir_str = QCoreApplication::applicationDirPath();
//    dir_str = dir_str.mid(0, dir_str.lastIndexOf("/"));
    dir_str = dir_str + "/code/新项目";

    //qDebug()<<"Debug Action" << dir_str;
    pro0->setData(1, 1, QString(dir_str));

    // 若九章测试项目尚不存在，新建项目
    QDir dir;
    if (!dir.exists(dir_str))
    {
        bool res = dir.mkpath(dir_str);
        qDebug() << "Init-新建目录：新项目" << res;// 若九章测试项目尚不存在，新建项目（显示true表示成功）
    }

    // 若九章测试文件尚不存在，新建文件
    QTreeWidgetItem *file0 = new QTreeWidgetItem(pro0);
    file0->setText(0, "未命名.jzss");
    //file0->setData(1,1,QString("0"));
    dir_str = dir_str + "/未命名.jzss";
    if (!dir.exists(dir_str)) saveFile(dir_str);
}


/*****************************************************************
 ***********************Tree Menu*********************************
 *****************************************************************/
void myjz_519::creaTreeMenu(){
    //创建右键（project项目）弹出的菜单栏
    treeMenu = new QMenu(this);

    run_action = new QAction(QIcon("Resources\\run0.png"),"运行(&R)",this);
    add_project = new QAction("创建新项目",this);
    add_file = new QAction("添加新文件",this);
    rename_action = new QAction("重命名(&M)",this);
    close_action = new QAction("关闭项目(&C)",this);
    del_action = new QAction(QIcon("Resources\\transparent.png"),"删除(&D)",this);

    treeMenu->addAction(run_action);
    treeMenu->addSeparator();
    treeMenu->addAction(add_project);
    treeMenu->addAction(add_file);
    treeMenu->addAction(rename_action);
    treeMenu->addAction(close_action);
    treeMenu->addSeparator();
    treeMenu->addAction(del_action);

    //检测点击事件
    //connect(newfile, &QAction::triggered, this, &myjz_519::newFile); "新建(&N)"
    connect(run_action, SIGNAL(triggered(bool)) , this, SLOT(onRunPro(bool)));
    connect(add_project, SIGNAL(triggered(bool)) , this, SLOT(onAddPro(bool)));
    connect(add_file, SIGNAL(triggered(bool)) , this, SLOT(onAddFile(bool)));
    connect(rename_action, SIGNAL(triggered(bool)) , this, SLOT(onRename(bool)));
    connect(close_action, SIGNAL(triggered(bool)) , this, SLOT(onCloseProFile(bool)));
    connect(del_action, SIGNAL(triggered(bool)) , this, SLOT(onDelProFile(bool)));

    //创建右键（file文件）弹出的菜单栏
    itemMenu = new QMenu(this);

    itemMenu->addAction(add_project);
    itemMenu->addAction(add_file);
    itemMenu->addAction(rename_action);
    itemMenu->addSeparator();
    itemMenu->addAction(del_action);
}

void myjz_519::onRunPro(bool){
    RunClicked();
}

void myjz_519::onAddPro(bool)
{
    bool ok;
    QString projectDirectory = QFileDialog::getExistingDirectory(this,tr("选择新建项目所属的文件夹"));
    if (projectDirectory == NULL) return;
    QString ProjectName = QInputDialog::getText(this, "输入" , "新建项目名:" , QLineEdit::Normal,  "", &ok, Qt::WindowCloseButtonHint);
    if (ProjectName == NULL) return;
    //qDebug()<<"Add Project";

    // 在 Tree Widget 中显示新文件夹
    QTreeWidgetItem *ProjectItem = new QTreeWidgetItem(treeWidget);
    ProjectItem->setText(0, ProjectName);

    // 获取准新建文件夹的绝对路径
    QString dir_str = projectDirectory + "/" + ProjectName;
    ProjectItem->setData(1,1,QString(dir_str));

    // 检查该路径是否存在，若不存在则新建
    QDir dir;
    if (!dir.exists(dir_str))
    {
        bool res = dir.mkpath(dir_str);
        qDebug() << "NewPro-新建目录是否成功" << res;//检验是否成功新建了项目（显示true表示成功）
    }
}

void myjz_519::onAddFile(bool)
{
    QTreeWidgetItem *currentItem = treeWidget->currentItem(); //获取当前被点击的节点

    if(!currentItem) //未选中项目
    {
        QMessageBox::information(this, "Information" , "Please Choose Project");
        return ;
    }
    if(currentItem->parent()) //对选中项目或文件一视同仁，均在项目目录下添加新文件
    {
        bool ok;
        QInputDialog *dialog  = new  QInputDialog(this);//  #生成一个输入框
        QString newFileName = dialog->getText(this, "输入" , "新建文件名:" , QLineEdit::Normal, "", &ok, Qt::WindowCloseButtonHint);
        if (newFileName == NULL) return;

        QTreeWidgetItem *fileItem = new QTreeWidgetItem(currentItem->parent());
        fileItem->setText(0 , newFileName);
        //fileItem->setData(1,1,QString("0"));

        QString dir_str = currentItem->parent()->data(1,1).toString();
        dir_str = dir_str + "/" + newFileName;

        if (maybeSave()) {
            textedit->clear();
            saveFile(dir_str);
        }
        //qDebug()<<"Add File (Click File)" << dir_str;

        return ;
    }

    bool ok;
    QString newFileName = QInputDialog::getText(this, "输入" , "新建文件名:" , QLineEdit::Normal,  "", &ok, Qt::WindowCloseButtonHint);
    if (newFileName == NULL) return;
    QTreeWidgetItem *fileItem = new QTreeWidgetItem(currentItem);
    fileItem->setText(0 , newFileName);

    QString dir_str = currentItem->data(1,1).toString();
    dir_str = dir_str + "/" + newFileName;
    if (maybeSave()) {
        textedit->clear();
        saveFile(dir_str);
    }
    //qDebug()<<"Add File (Click Pro)" << dir_str;

    return ;
}

void myjz_519::onRename(bool){
    QTreeWidgetItem *currentItem = treeWidget->currentItem(); //获取当前被点击的节点

    if(!currentItem) //未选中Item
    {
        QMessageBox::information(this, "Information" , "Please Choose Project");
        return ;
    }

    bool ok; // 读取用户输入的新名字
    QString oldName = currentItem->text(0);
    QString newFileName = QInputDialog::getText( this, "重命名" , "输入新名字:" , QLineEdit::Normal, oldName, &ok, Qt::WindowCloseButtonHint);
    if (newFileName == NULL) return;

    if(currentItem->parent()) //对选中项目或文件重新命名
    {
        QString dir_str = currentItem->parent()->data(1,1).toString();
        QString new_dir_str = dir_str + "/" + newFileName;
        dir_str = dir_str + "/" + currentItem->text(0);
        if (maybeSave()) {
            bool check = QFile::rename(dir_str, new_dir_str);
            qDebug() << "重命名:" << check << new_dir_str;
            if (!check) return;
            currentItem->setText(0, newFileName);
        }
        return;
    }

    QString dir_str = currentItem->data(1,1).toString();
    QString new_dir_str = dir_str.mid(0, dir_str.lastIndexOf("/"));
    new_dir_str = new_dir_str + "/" + newFileName;

    if (maybeSave()) {
        bool check = QFile::rename(dir_str, new_dir_str);
        qDebug() << "重命名:" << check << new_dir_str;
        if (!check) return;
        currentItem->setText(0, newFileName);
        currentItem->setData(1, 1, new_dir_str);
    }

    return;
}

void myjz_519::onCloseProFile(bool)
{
    QTreeWidgetItem *currentItem = treeWidget->currentItem();
    if(currentItem)
    {
        QMessageBox mess(QMessageBox::Information,tr("提示"),tr("您确定要关闭当前项目吗？"));
        QPushButton *okbutton = mess.addButton(tr("确定"),QMessageBox::AcceptRole);
        mess.addButton(tr("取消"),QMessageBox::AcceptRole);
        QIcon *icon = new QIcon("Resources\\nine.png");
        mess.setWindowIcon(*icon);
        mess.exec();

        if(mess.clickedButton()==okbutton) {
            treeWidget->takeTopLevelItem(treeWidget->indexOfTopLevelItem(treeWidget->currentItem()));
            qDebug()<<"Close-选中项目已关闭";//选中项目已关闭
        }
    }
}

void myjz_519::onDelProFile(bool)
{//qDebug()<<"Delete Action";
    QTreeWidgetItem *currentItem = treeWidget->currentItem();
    //QString dir_str = checkself(currentItem);
    QString dir_str;
    if(currentItem)
    {
        if(currentItem->parent())
        {
            QMessageBox mess(QMessageBox::Information,tr("提示"),tr("您确定要删除当前文件吗？"));
            QPushButton *okbutton = mess.addButton(tr("确定"),QMessageBox::AcceptRole);
            mess.addButton(tr("取消"),QMessageBox::AcceptRole);
            QIcon *icon = new QIcon("Resources\\nine.png");
            mess.setWindowIcon(*icon);
            mess.exec();

            if(mess.clickedButton()==okbutton) {
                dir_str = currentItem->parent()->data(1,1).toString();
                QString currentItemInfo = currentItem->text(0);
                dir_str = dir_str + "/" + currentItemInfo;
                currentItem->parent()->removeChild(currentItem);
            }
        }
        else
        {
            QMessageBox mess(QMessageBox::Information,tr("提示"),tr("您确定要删除当前项目吗？"));
            QPushButton *okbutton = mess.addButton(tr("确定"),QMessageBox::AcceptRole);
            mess.addButton(tr("取消"),QMessageBox::AcceptRole);
            QIcon *icon = new QIcon("Resources\\nine.png");
            mess.setWindowIcon(*icon);
            mess.exec();

            if(mess.clickedButton()==okbutton) {
                dir_str = currentItem->data(1,1).toString();
                treeWidget->takeTopLevelItem(treeWidget->indexOfTopLevelItem(treeWidget->currentItem()));
            }
        }
        DeleteFileOrFolder(dir_str);
        newFile();
    }
}

bool myjz_519::DeleteFileOrFolder(const QString &strPath)//要删除的文件夹或文件的路径
{
    if (strPath.isEmpty() || !QDir().exists(strPath))//是否传入了空的路径||路径是否存在
        return false;

    QFileInfo FileInfo(strPath);

    if (FileInfo.isFile()){    //如果是文件
        QFile::remove(strPath);
    }
    else if (FileInfo.isDir())//如果是文件夹
    {
        QDir qDir(strPath);
        qDir.removeRecursively();
    }
    qDebug() << "Delete-删除文件：" << strPath;//文件已删除（显示删除前的绝对路径）
    return true;
}



/*****************************************************************
 *****************************************************************
 **********************Push Button********************************
 *****************************************************************
 *****************************************************************/
void myjz_519::RunClicked(){
    if (!maybeSave()) return;
// 路径预处理：得到当前项目的绝对路径和其名下的所有文件名
    QTreeWidgetItem *currentItem = treeWidget->currentItem();
    if (currentItem->parent()) currentItem = currentItem->parent();
    QString dir_str = currentItem->data(1,1).toString();
    QString runFolderName = currentItem->text(0) + "-temp";

    QDir *dir = new QDir(dir_str);
    QStringList filter;
    QList<QFileInfo> *fileInfo = new QList<QFileInfo>(dir->entryInfoList(filter));

// 检查运行路径是否存在，若不存在则新建
    runFolderName = "/" + runFolderName;
    QString dir_run = dir_str + runFolderName;
    QDir dirRun;
    if (!dirRun.exists(dir_run))
    {
        bool res = dirRun.mkpath(dir_run);
        qDebug() << "NewFolder-新建运行目录是否成功" << res;//检验是否成功新建了运行文件夹（显示true表示成功）
    }

// 路径预处理：读取项目中的所有文件的绝对路径和文件内容，并添加到vectors
    vector<std::string> editorFilePath;
    vector<std::wstring> editorFileContent;
    editorFilePath.push_back("");
    QString tem = "";
    std::wstring blank = tem.toStdWString();
    editorFileContent.push_back(blank);

    bool jzssExist = false;

    for(int i = 2; i < fileInfo->count(); i++)
    {
        qDebug()<< "Run-读取文件：" << fileInfo->at(i).filePath();//运行时依次读取文件（显示每个文件的绝对路径）
        QString eachFilePath = fileInfo->at(i).filePath();//at(i).fileName();

        if (!eachFilePath.contains(".jz")) continue;

        if (eachFilePath.contains(".jzss")){// 主文件（放在第一位）
            jzssExist = true;
            QFile file(eachFilePath);// 根据已知路径打开文件
            if (!file.open(QFile::ReadOnly | QFile::Text)) {
                QMessageBox::warning(this, tr("九章"),
                                     tr("Cannot read file %1:\n%2.")
                                     .arg(QDir::toNativeSeparators(eachFilePath), file.errorString()));
                return;
            }

            QTextStream in(&file);
            QString allFileInfo = in.readAll();

            eachFilePath = eachFilePath.mid(0, eachFilePath.lastIndexOf("/")) + runFolderName + "/" +fileInfo->at(i).fileName();
            eachFilePath = eachFilePath.mid(0, eachFilePath.lastIndexOf("."));
            qDebug() << "转化成vector所需格式" << eachFilePath;
            std::string str_filePath = eachFilePath.toStdString();// 转化成vector所需格式
            std::wstring wstr_file = allFileInfo.toStdWString();
            str_filePath = StringToUtf(str_filePath);
            editorFilePath[0] = str_filePath;// 把所得信息放入vector
            editorFileContent[0] = wstr_file;
        }

        else{// 非主文件
            QFile file(eachFilePath);// 根据已知路径打开文件
            if (!file.open(QFile::ReadOnly | QFile::Text)) {
                QMessageBox::warning(this, tr("九章"),
                                     tr("Cannot read file %1:\n%2.")
                                     .arg(QDir::toNativeSeparators(eachFilePath), file.errorString()));
                return;
            }

            QTextStream in(&file);
            //QTextStream out(&file);
            QString allFileInfo = in.readAll();

            eachFilePath = eachFilePath.mid(0, eachFilePath.lastIndexOf("/")) + runFolderName + "/" +fileInfo->at(i).fileName();
            eachFilePath = eachFilePath.mid(0, eachFilePath.lastIndexOf("."));
            qDebug() << "转化成vector所需格式" << eachFilePath;
            std::string str_filePath = eachFilePath.toStdString();// 转化成vector所需格式
            std::wstring wstr_file = allFileInfo.toStdWString();
            str_filePath = StringToUtf(str_filePath);
            editorFilePath.push_back(str_filePath);// 把所得信息放入vector
            editorFileContent.push_back(wstr_file);
        }
    }

    if (jzssExist == false) {
        editorFilePath.erase(editorFilePath.begin());
        editorFileContent.erase(editorFileContent.begin());
    }

// Compiler每个文件
    try{
        vector<string> compilerFile = Compiler(editorFilePath, editorFileContent);

        int count = compilerFile.size();
        for (int i = 0; i < count; i++){
          QString resultGot = QString::fromStdString(compilerFile[i]);
        }
        vector<string> linkerFile = assembler(compilerFile);

        string simulatorAddr = linker_l(linkerFile);

        simulator(simulatorAddr, this);
    }
    catch (string message) {
        fakeTextEdit->insertPlainText(('\n'+message+'\n').c_str());
    }

}

void myjz_519::inputClicked(){
    QString inputInfo = inputLine->text();
    std::wstring str_STL = inputInfo.toStdWString();
    inputNum = chineseToNum(str_STL);

    std::string a;
    QString b;
    if (inputNum == 2147483640){
        fakeTextEdit->insertPlainText("错误：输入内容不规范，请输入中文格式的数字(例如：二百零一)\n" );
    }else{
        a = std::to_string(inputNum);
        b = QString(QString::fromUtf8(a.c_str()));
    }

//     更新GUI界面信息
    inputLine->clear();
    fakeTextEdit->insertPlainText("输入：" + inputInfo + "\n");
    //fakeTextEdit->insertPlainText("输入：" + b + "\n");
    //qDebug() << "inputClicked:" << inputInfo;
}



/*****************************************************************
 *****************************************************************
 ***********************Edit Function*****************************
 *****************************************************************
 *****************************************************************/
void myjz_519::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
    qDebug() << "Close-关闭九章编辑器";//九章编辑器已关闭
}

void myjz_519::newFile()
{
    if (maybeSave()) {
        textedit->clear();
        setCurrentFile(QString());
    }
}

void myjz_519::openProject()
{
    if (maybeSave()) {
        QString proName = QFileDialog::getExistingDirectory(this);
        if (proName == NULL) return;
        qDebug() << "Open-打开项目" << proName;//文件菜单中打开已存在项目（显示项目路径）

        QTreeWidgetItem *pro0 = new QTreeWidgetItem(treeWidget);
        pro0->setData(1,1,QString(proName));

        proName = proName.mid(proName.lastIndexOf("/")+1);
        pro0->setText(0, proName);
    }
}

bool myjz_519::save()
{
    if (curFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

bool myjz_519::saveAs()
{
    QFileDialog dialog(this, QString::fromUtf8("另存为"),"untitled.jz","九章(*.jz);;九章算术(*.jzss);;AllFile(*.*)");
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec() != QDialog::Accepted)
        return false;
    return saveFile(dialog.selectedFiles().first());
}

void myjz_519::about()
{// QString::fromUtf8("关于")
   QMessageBox::about(this, tr("关于"),
            QString::fromUtf8("九章算术文言文编辑器"));
}

void myjz_519::documentWasModified()
{
    setWindowModified(textedit->document()->isModified());
}

void myjz_519::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void myjz_519::readSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }
}

void myjz_519::writeSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());
}

bool myjz_519::maybeSave()
{
    if (!textedit->document()->isModified())
        return true;
    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("警告"),
                               tr("当前文件有未保存的更改。\n"
                                  "您希望保存当前更改吗？"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        return save();
    case QMessageBox::Cancel:
        return false;
    default:
        textedit->clear();
        setCurrentFile(QString());
        break;
    }
    return true;
}

void myjz_519::loadFile(const QString &fileName)
{
    qDebug() << "Load-加载文件："<<fileName;//双击加载文件（显示文件的绝对路径）
    //statusBar()->showMessage(tr("File loaded:")+&fileName, 2000);
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QTextStream in(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif

    textedit->setPlainText(in.readAll());

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
    QString shownName = curFile.mid(curFile.lastIndexOf("/")+1) +
            "[*]" + tr(" ——九章算术文言文编辑器");
    setWindowTitle(shownName);
    fakeTextEdit->clear();
    fakeTextEdit->insertPlainText("正在编辑："+fileName + "\n"); //将路径显示到plainTextEdit中
}

bool myjz_519::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName),
                                  file.errorString()));
        return false;
    }

    QTextStream out(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    out << textedit->toPlainText();
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
    QString shownName = curFile.mid(curFile.lastIndexOf("/")+1) +
            "[*]" + tr(" ——九章算术文言文编辑器");
    setWindowTitle(shownName);
    return true;
}

void myjz_519::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    textedit->document()->setModified(false);
    setWindowModified(false);

    QString shownName = curFile;
    if (curFile.isEmpty())
        shownName = "untitled.jz[*]" + tr(" ——九章算术文言文编辑器");
    setWindowTitle(shownName);
}

QString myjz_519::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

#ifndef QT_NO_SESSIONMANAGER
void myjz_519::commitData(QSessionManager &manager)
{
    if (manager.allowsInteraction()) {
        if (!maybeSave())
            manager.cancel();
    } else {
        // Non-interactive: save without asking
        if (textedit->document()->isModified())
            save();
    }
}
#endif



/*****************************************************************
 ***********************Font & Find Text**************************
 *****************************************************************/
void myjz_519::AutoTransfer(){
    textedit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

void myjz_519::SetFont(){
    //textedit->setPlainText(QString::fromUtf8("设置字体"));
    bool ok;
    QFont font = QFontDialog::getFont(&ok, this);
    if(ok){
        textedit->setFont(font);
    }

}

void myjz_519::FindTextW(){
    finddlg->show();
}

void myjz_519::Results_Show(){
    QString string = findedit->text();
    //从内容后面开始查找
    bool isfind = textedit->find(string, QTextDocument::FindBackward);
    if(isfind){
        //高亮
        QPalette palette = textedit->palette();
        palette.setColor(QPalette::Highlight,palette.color(QPalette::Active,QPalette::Highlight));
        textedit->setPalette(palette);
    }else{
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("没有查找到该内容"),QMessageBox::Ok);
    }
}



/*****************************************************************
 *****************************************************************
 ***********************Edit Area Menu****************************
 *****************************************************************
 ****************************************************************/
void myjz_519::createMenu()
{
    //menu bar*********************************
    menuBar = new QMenuBar;

    /*****************************************************************
     ***********************File文件*************************************
     ****************************************************************/
    fileMenu = new QMenu(QString::fromUtf8("文件(&F)"));

    newfile = new QAction(QString::fromUtf8("  新建(&N)"), this);
    connect(newfile, &QAction::triggered, this, &myjz_519::newFile);

    openfile = new QAction(QString::fromUtf8("  打开(&O)"), this);
    connect(openfile, &QAction::triggered, this, &myjz_519::openProject);

    savefile= new QAction(QString::fromUtf8("  保存(&S)"), this);
    connect(savefile, &QAction::triggered, this, &myjz_519::save);

    saveasfile = new QAction(QString::fromUtf8("  另存为(&A)"), this);
    connect(saveasfile, &QAction::triggered, this, &myjz_519::saveAs);

    out = new QAction(QString::fromUtf8("  退出(&X)"), this);
    connect(out, &QAction::triggered, this, &myjz_519::close);

    fileMenu->addAction(newfile);
    fileMenu->addAction(openfile);
    fileMenu->addAction(savefile);
    fileMenu->addAction(saveasfile);
    fileMenu->addAction(out);


/*****************************************************************
 ***********************Edit*************************************
 ****************************************************************/
    editorMenu = new QMenu(QString::fromUtf8("编辑(&E)"));
    Cut = new QAction(QString::fromUtf8("  剪切(&T)"), this);
    connect(Cut, &QAction::triggered, textedit, &QPlainTextEdit::cut);

    Copy = new QAction(QString::fromUtf8("  拷贝(&C)"), this);
    connect(Copy, &QAction::triggered, textedit, &QPlainTextEdit::copy);

    Paste = new QAction(QString::fromUtf8("  粘贴(&P)"), this);
    connect(Paste, &QAction::triggered, textedit, &QPlainTextEdit::paste);

    editorMenu->addAction(Cut);
    editorMenu->addAction(Copy);
    editorMenu->addAction(Paste);

/*****************************************************************
 ***********************Format************************************
 *****************************************************************/
    formatMenu = new QMenu(QString::fromUtf8("格式(&O)"));//geshi
    Auto = new QAction(QString::fromUtf8("  自动换行(&W)"), this); //change line automatially
    connect(Auto, &QAction::triggered, this, &myjz_519::AutoTransfer);

    Font = new QAction(QString::fromUtf8("  字体(&F)"), this); // the font of words
    connect(Font, &QAction::triggered, this, &myjz_519::SetFont);

    formatMenu->addAction(Auto);
    formatMenu->addAction(Font);

/*****************************************************************
 ***********************Select************************************
 *****************************************************************/
    selectMenu = new QMenu(QString::fromUtf8("查看(&V)"));
    select = new QAction(QString::fromUtf8("  查找(&F)"), this);
    connect(select, &QAction::triggered, this, &myjz_519::FindTextW);

    render = new QAction(QString::fromUtf8("  渲染"), this);
    connect(render, &QAction::triggered, this, &myjz_519::render_show);

    selectMenu->addAction(select);
    selectMenu->addAction(render);
/*****************************************************************
 ***********************Help*************************************
 ****************************************************************/
    helpMenu = new QMenu(QString::fromUtf8("帮助(&H)"));
    About = new QAction(QString::fromUtf8("  关于(&A)"), this);
    connect(About, &QAction::triggered, this, &myjz_519::about);
    helpMenu->addAction(About);

    menuBar->addMenu(fileMenu);
    menuBar->addMenu(editorMenu);
    menuBar->addMenu(formatMenu);
    menuBar->addMenu(selectMenu);
    menuBar->addMenu(helpMenu);

    this ->setMenuBar(menuBar);

    //查找弹跳窗口，原先被隐藏
    QHBoxLayout *h1 = new QHBoxLayout;
    finddlg = new QDialog(this); //查找窗口
    finddlg->setWindowTitle("查找窗口");
    finddlg->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    QLabel *label = new QLabel(QString::fromUtf8("查找内容:"));
    findedit = new QLineEdit; //查找的内容
    QPushButton *selBtn = new QPushButton(QString::fromUtf8("查找"));
    connect(selBtn, &QPushButton::clicked, this, &myjz_519::Results_Show);
    h1->addWidget(label);
    h1->addWidget(findedit);
    h1->addWidget(selBtn);
    finddlg->setLayout(h1);
    finddlg->hide();

    //渲染文本
    QHBoxLayout *h2 = new QHBoxLayout;
    renderWin = new QDialog(this); //查找窗口
    renderWin->setWindowTitle("渲染文本");
    renderWin->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    renderLbl = new QTextEdit();
    renderLbl->setReadOnly(true);
    h2->addWidget(renderLbl);
    renderWin->setLayout(h2);
    renderWin->hide();
}


/*************************************************************************************
 *************************************************************************************
 *************************************************************************************
 ******************************Class Code Editor**************************************
 *************************************************************************************
 *************************************************************************************
 *************************************************************************************
 */
CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}



int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}



void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}


void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}


void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}


void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        //QColor lineColor = QColor(227, 186, 122).lighter(120);
        QColor lineColor = QColor(180, 180, 180).lighter(120);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}


void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

//![extraAreaPaintEvent_0]

//![extraAreaPaintEvent_1]
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();
//![extraAreaPaintEvent_1]

//![extraAreaPaintEvent_2]
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void myjz_519::edit_output(std::string output_string){
    QString qstring = QString(QString::fromUtf8(output_string.c_str()));
    qDebug() << qstring;
    fakeTextEdit->insertPlainText(qstring);
}

string StringToUtf(string strValue)
{
    int nwLen = MultiByteToWideChar(CP_UTF8, 0, strValue.c_str(), -1, NULL, 0);
    wchar_t * pwBuf = new wchar_t[nwLen + 1];//加上末尾'\0'
    memset(pwBuf, 0, nwLen * 2 + 2);
    MultiByteToWideChar(CP_UTF8, 0, strValue.c_str(), strValue.length(), pwBuf, nwLen);
    int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL);
    char * pBuf = new char[nLen + 1];
    memset(pBuf, 0, nLen + 1);
    WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);
    std::string retStr = pBuf;
    delete []pBuf;
    delete []pwBuf;
    pBuf = NULL;
    pwBuf = NULL;
    return retStr;
}


void myjz_519::render_show(){
    //renderWin->setSizeGripEnabled(false);
    QString renderText = textedit->toPlainText();
    renderText.remove(QString("。"), Qt::CaseInsensitive);
//    renderText.remove(QString("“"), Qt::CaseInsensitive);
//    renderText.remove(QString("”"), Qt::CaseInsensitive);
    renderText = renderText.simplified();
    string str_renderText = renderText.toStdString();
    string result;
    QString Qtarget;
    string target;
    string classic_target;

    for (int chiNum = 0; chiNum < str_renderText.size(); chiNum++){
//        target = str_renderText.substr(chiNum,1);
        Qtarget = renderText[chiNum];
        target = Qtarget.toStdString();
        if (simple_classic.count(target)!=0){
            classic_target = simple_classic[target];
            result += classic_target;
        }
        else if (target == " "){
            result += "\n";
        }else{
            result += target;
        }
    }

    renderLbl->setStyleSheet("QTextEdit{background:write url(Resources//renBack.png)}");

    renderLbl->setText(QString::fromUtf8(result.c_str()));
    renderLbl->setFont(QFont("隶书",22));
    renderWin->resize(920,800);
    renderWin->show();

}
