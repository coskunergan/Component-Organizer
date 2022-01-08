/*********************************************************************
Component Organizer
Copyright (C) Mário Ribeiro (mario.ribas@gmail.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

#include "optionsdialog.h"
#include "ui_optionsdialog.h"

#include "co.h"
#include "component.h"
#include "package.h"
#include "container.h"
#include "manufacturer.h"
#include "label.h"
#include "pminitablewidget.h"

#include "mainwindow.h"

#include "componentdialog.h"
#include "componentdetails.h"
#include "ui_componentdialog.h"

#include "co_defs.h"
#include "stock.h"
#include "stocktable.h"

#include <QListWidgetItem>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>

#include <QDebug>

#include <QAxObject>

OptionsDialog::OptionsDialog(CO *co, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog),
    m_co(co)
{
    ui->setupUi(this);

    setup();
}

OptionsDialog::~OptionsDialog()
{
    delete m_manufacturerTable;
    delete m_secondaryLabelTable;
    delete m_primaryLabelTable;
    delete m_packageTable;
    delete m_containerTable;
    delete ui;
}

void OptionsDialog::setup()
{
    setWindowTitle(tr("Settings"));
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    MainWindow *w = (MainWindow *) parentWidget();
    ui->saveLayout_checkBox->setChecked(w->settings().saveDimensions);
    ui->markLowStock_checkBox->setChecked(w->settings().markLowStock);
    ui->showContainers_checkBox->setChecked(w->settings().showContainers);

    QListWidgetItem *general = new QListWidgetItem(tr("General"));
    general->setIcon(QIcon(":/img/config.png"));
    ui->options_listWidget->addItem(general);

    QListWidgetItem *containers = new QListWidgetItem(tr("Containers"));
    containers->setIcon(QIcon(":/img/container.png"));
    ui->options_listWidget->addItem(containers);

    QListWidgetItem *packages = new QListWidgetItem(tr("Packages"));
    packages->setIcon(QIcon(":/img/package.png"));
    ui->options_listWidget->addItem(packages);

    QListWidgetItem *labels = new QListWidgetItem(tr("Labels"));
    labels->setIcon(QIcon(":/img/label.png"));
    ui->options_listWidget->addItem(labels);

    QListWidgetItem *manufacturers = new QListWidgetItem(tr("Manufacturers"));
    manufacturers->setIcon(QIcon(":/img/manufacturer.png"));
    ui->options_listWidget->addItem(manufacturers);

    QListWidgetItem *product = new QListWidgetItem(tr("Product"));
    product->setIcon(QIcon(":/img/chip.png"));
    ui->options_listWidget->addItem(product);

    QListWidgetItem *yamaha = new QListWidgetItem(tr("Yamaha SMT"));
    yamaha->setIcon(QIcon(":/img/chip_64x64.png"));
    ui->options_listWidget->addItem(yamaha);

    m_containerTable = new pMiniTableWidget(this);
    m_containerTable->setMaximumWidth(ColumnMaxWidth);
    ui->containerPage_verticalLayout->addWidget(m_containerTable);
    foreach(QString containerName, m_co->containerNames())
    {
        int row = m_containerTable->rowCount();
        m_containerTable->insertRow(row);
        m_containerTable->addItem(row, 0, containerName);
    }

    m_packageTable = new pMiniTableWidget(this);
    m_packageTable->setMaximumWidth(ColumnMaxWidth);
    ui->packagePage_verticalLayout->addWidget(m_packageTable);
    foreach(QString packageName, m_co->packageNames())
    {
        int row = m_packageTable->rowCount();
        m_packageTable->insertRow(row);
        m_packageTable->addItem(row, 0, packageName);
    }

    m_primaryLabelTable = new pMiniTableWidget(this);
    m_primaryLabelTable->setMaximumWidth(ColumnMaxWidth);
    m_primaryLabelTable->setMaximumHeight(170);
    ui->primaryLabel_widget->layout()->addWidget(m_primaryLabelTable);
    foreach(Label *l, m_co->topLabels())
    {
        int row = m_primaryLabelTable->rowCount();
        m_primaryLabelTable->insertRow(row);
        m_primaryLabelTable->addItem(row, 0, l->name());
    }

    m_secondaryLabelTable = new pMiniTableWidget(this);
    m_secondaryLabelTable->setMaximumWidth(ColumnMaxWidth);
    m_secondaryLabelTable->setMaximumHeight(170);
    ui->secondaryLabel_widget->layout()->addWidget(m_secondaryLabelTable);

    m_manufacturerTable = new pMiniTableWidget(this);
    m_manufacturerTable->setMaximumWidth(ColumnMaxWidth);
    ui->manufacturerPage_verticalLayout->addWidget(m_manufacturerTable);
    foreach(QString manufacturerName, m_co->manufacturerNames())
    {
        int row = m_manufacturerTable->rowCount();
        m_manufacturerTable->insertRow(row);
        m_manufacturerTable->addItem(row, 0, manufacturerName);
    }

    connect(m_primaryLabelTable, SIGNAL(currentItemChanged(QTableWidgetItem*,QTableWidgetItem*)), this, SLOT(primaryLabelChangedHandler()));
    connect(ui->options_listWidget, SIGNAL(currentRowChanged(int)), ui->stackedWidget, SLOT(setCurrentIndex(int)));

    m_primaryLabelTable->setCurrentRow(0);
    ui->options_listWidget->setCurrentRow(0);

    connect(ui->addContainer_toolButton, SIGNAL(clicked()), this, SLOT(addContainerHandler()));
    connect(ui->removeContainer_toolButton, SIGNAL(clicked()), this, SLOT(removeContainerHandler()));
    connect(ui->container_lineEdit, SIGNAL(textChanged(QString)), this, SLOT(updateInterface()));
    connect(m_containerTable, SIGNAL(itemSelectionChanged()), this, SLOT(updateInterface()));

    connect(ui->addPackage_toolButton, SIGNAL(clicked()), this, SLOT(addPackageHandler()));
    connect(ui->removePackage_toolButton, SIGNAL(clicked()), this, SLOT(removePackageHandler()));
    connect(ui->package_lineEdit, SIGNAL(textChanged(QString)), this, SLOT(updateInterface()));
    connect(m_packageTable, SIGNAL(itemSelectionChanged()), this, SLOT(updateInterface()));

    connect(ui->addPrimaryLabel_toolButton, SIGNAL(clicked()), this, SLOT(addPrimaryLabelHandler()));
    connect(ui->removePrimaryLabel_toolButton, SIGNAL(clicked()), this, SLOT(removePrimaryLabelHandler()));
    connect(m_primaryLabelTable, SIGNAL(itemSelectionChanged()), this, SLOT(updateInterface()));
    connect(ui->primaryLabel_lineEdit, SIGNAL(textChanged(QString)), this, SLOT(updateInterface()));

    connect(ui->addSecondaryLabel_toolButton, SIGNAL(clicked()), this, SLOT(addSecondaryLabelHandler()));
    connect(ui->removeSecondaryLabel_toolButton, SIGNAL(clicked()), this, SLOT(removeSecondaryLabelHandler()));
    connect(m_secondaryLabelTable, SIGNAL(itemSelectionChanged()), this, SLOT(updateInterface()));
    connect(ui->secondaryLabel_lineEdit, SIGNAL(textChanged(QString)), this, SLOT(updateInterface()));

    connect(ui->addManufacturer_toolButton, SIGNAL(clicked()), this, SLOT(addManufacturerHandler()));
    connect(ui->removeManufacturer_toolButton, SIGNAL(clicked()), this, SLOT(removeManufacturerHandler()));
    connect(ui->manufacturer_lineEdit, SIGNAL(textChanged(QString)), this, SLOT(updateInterface()));
    connect(m_manufacturerTable, SIGNAL(itemSelectionChanged()), this, SLOT(updateInterface()));

    connect(ui->PoductOpenFile_pushButton, SIGNAL(clicked()), this, SLOT(browseFile()));

    connect(ui->PoductCheck_pushButton, SIGNAL(clicked()), this, SLOT(CheckBOM()));
    connect(ui->PoductReduce_pushButton, SIGNAL(clicked()), this, SLOT(ReduceBOM()));
    connect(ui->PoductAdd_pushButton, SIGNAL(clicked()), this, SLOT(AddBOM()));
    connect(ui->PoductMax_pushButton, SIGNAL(clicked()), this, SLOT(MaximumBOMCalc()));

    connect(ui->ProductBOMCount_spinBox, SIGNAL(valueChanged(int)), this, SLOT(CheckRequest()));

    connect(ui->SmtOpenBOMFile_pushButton, SIGNAL(clicked()), this, SLOT(SmtBrowseBOMFile()));
    connect(ui->SmtOpenPlaceFile_pushButton, SIGNAL(clicked()), this, SLOT(SmtBrowsePlaceFile()));
    connect(ui->SmtGenerateFile_pushButton, SIGNAL(clicked()), this, SLOT(SmtGenerateFile()));


    updateInterface();
}

void OptionsDialog::accept()
{
    MainWindow *w = (MainWindow *) parentWidget();

    MainWindow::Settings settings;
    settings.saveDimensions = ui->saveLayout_checkBox->isChecked();
    settings.markLowStock = ui->markLowStock_checkBox->isChecked();
    settings.showContainers = ui->showContainers_checkBox->isChecked();
    w->setSettings(settings);

    done(QDialog::Accepted);    
}

void OptionsDialog::primaryLabelChangedHandler()
{
    int curRow = m_primaryLabelTable->currentRow();
    QString labelName = m_primaryLabelTable->itemText(curRow, 0);

    if(!labelName.isEmpty())
    {
        m_secondaryLabelTable->removeAll();
        Label *top = m_co->findTopLabel(labelName);

        if(!top->leafs().isEmpty())
        {
            foreach(Label *l, top->leafs())
            {
                int row = m_secondaryLabelTable->rowCount();
                m_secondaryLabelTable->insertRow(row);
                m_secondaryLabelTable->addItem(row, 0, l->name());
            }
        }
    }
}

void OptionsDialog::addContainerHandler()
{
    QString name = ui->container_lineEdit->text();

    if(m_containerTable->hasText(name, 0) >= 0)
    {
        QMessageBox::information(this,
                                 tr("Info"),
                                 tr("Container \"") + name + tr("\" already exists."));
        return;
    }

    int row = m_containerTable->rowCount();
    m_containerTable->insertRow(row);
    m_containerTable->addItem(row, 0, name);

    Container *container = new Container(name, m_co);
    m_co->addContainer(container);
}

void OptionsDialog::removeContainerHandler()
{
    QString name = m_containerTable->currentItem()->text();

    QList<Component*> usingIt;
    foreach(Component *c, m_co->components())
        if(c->container() != 0 && c->container()->name() == name)
            usingIt.append(c);

    if(message(tr("Are you sure you want to remove container \"")
               + name + tr("\"?") + "\n" +
               tr("There are ") + QString::number(usingIt.count()) + tr(" component(s) using it.")))
    {
        qDebug() << "remove container" <<  name;
        int row = m_containerTable->currentRow();
        m_containerTable->removeRow(row);

        foreach(Component *c, usingIt)
            c->setContainer(0);

        m_co->removeContainer(name);
    }
}

void OptionsDialog::addPackageHandler()
{
    QString name = ui->package_lineEdit->text();

    if(m_packageTable->hasText(name, 0) >= 0)
    {
        QMessageBox::information(this,
                                 tr("Info"),
                                 tr("Package \"") + name + tr("\" already exists."));
        return;
    }

    int row = m_packageTable->rowCount();
    m_packageTable->insertRow(row);
    m_packageTable->addItem(row, 0, name);

    Package *package = new Package(name, m_co);
    m_co->addPackage(package);
}

void OptionsDialog::removePackageHandler()
{
    QString name = m_packageTable->currentItem()->text();

    QList<Component*> usingIt;
    foreach(Component *c, m_co->components())
        if(c->stock(name) != 0)
            usingIt.append(c);

    if(message(tr("Are you sure you want to remove package \"")
               + name + tr("\"?") + "\n" +
               tr("There are ") + QString::number(usingIt.count()) + tr(" component(s) using it.")))
    {
        qDebug() << "remove package" <<  name;
        int row = m_packageTable->currentRow();
        m_packageTable->removeRow(row);

        foreach(Component *c, usingIt)
            c->removeStock(name);

        m_co->removePackage(name);
    }
}

void OptionsDialog::addPrimaryLabelHandler()
{
    QString name = ui->primaryLabel_lineEdit->text();

    if(m_co->findLabel(name) != 0)
    {
        QMessageBox::information(this,
                                 tr("Info"),
                                 tr("Label \"") + name + tr("\" already exists.") + "\n" +
                                 tr("The label's name must be unique."));
        return;
    }

    int row = m_primaryLabelTable->rowCount();
    m_primaryLabelTable->insertRow(row);
    m_primaryLabelTable->addItem(row, 0, name);

    Label *topLabel = new Label(name);
    m_co->addTopLabel(topLabel);
}

void OptionsDialog::removePrimaryLabelHandler()
{
    QString name = m_primaryLabelTable->currentItem()->text();

    QList<Component*> usingIt;
    foreach(Component *c, m_co->components())
        if(c->primaryLabel() != 0 && c->primaryLabel()->name() == name)
            usingIt.append(c);

    if(message(tr("Are you sure you want to remove primary label \"")
               + name + tr("\"?") + "\n" +
               tr("There are ") + QString::number(usingIt.count()) + tr(" component(s) using it.")))
    {
        int row = m_primaryLabelTable->currentRow();
        m_primaryLabelTable->removeRow(row);

        foreach(Component *c, usingIt)
        {
            c->setLabel(Component::PrimaryLabel,   0);
            c->setLabel(Component::SecondaryLabel, 0);
        }

        Label *top = m_co->findTopLabel(name);
        foreach(Label *leaf, top->leafs())
            m_co->removeLabel(leaf->name());
        m_co->removeLabel(top->name());
    }
}

void OptionsDialog::addSecondaryLabelHandler()
{
    QString name = ui->secondaryLabel_lineEdit->text();

    int row = m_primaryLabelTable->currentRow();
    QString topLabelName = m_primaryLabelTable->item(row,0)->text();
    Label *topLabel = m_co->findTopLabel(topLabelName);
    qDebug() << topLabel << (topLabel == 0);

    if(m_co->findSecondaryLabel(topLabel,name) != 0)
    {
        QMessageBox::information(this,
                                 tr("Info"),
                                 tr("Label \"") + name + tr("\" already exists.") + "\n" +
                                 tr("The label's name must be unique."));
        return;
    }

    row = m_secondaryLabelTable->rowCount();
    m_secondaryLabelTable->insertRow(row);
    m_secondaryLabelTable->addItem(row, 0, name);

    Label *leaf = new Label(name, topLabel);
    topLabel->addLeaf(leaf);
}

void OptionsDialog::removeSecondaryLabelHandler()
{
    QString name = m_secondaryLabelTable->currentItem()->text();

    QList<Component*> usingIt;
    foreach(Component *c, m_co->components())
        if(c->secondaryLabel() != 0 && c->secondaryLabel()->name() == name)
            usingIt.append(c);

    if(message(tr("Are you sure you want to remove secondary label \"")
               + name + tr("\"?") + "\n" +
               tr("There are ") + QString::number(usingIt.count()) + tr(" component(s) using it.")))
    {
        int row = m_secondaryLabelTable->currentRow();
        m_secondaryLabelTable->removeRow(row);

        foreach(Component *c, usingIt)
        {
            c->setLabel(Component::SecondaryLabel, 0);
        }

        m_co->removeLabel(name);
    }
}

void OptionsDialog::addManufacturerHandler()
{
    QString name = ui->manufacturer_lineEdit->text();

    if(m_manufacturerTable->hasText(name, 0, Qt::CaseInsensitive) >= 0)
    {
        QMessageBox::information(this,
                                 tr("Info"),
                                 tr("Manufacturer \"") + name + tr("\" already exists."));
        return;
    }

    Manufacturer *manufacturer = new Manufacturer(name, m_co);
    m_co->addManufacturer(manufacturer);

    m_manufacturerTable->removeAll();
    foreach(QString name, m_co->manufacturerNames())
    {
        int row = m_manufacturerTable->rowCount();
        m_manufacturerTable->insertRow(row);
        m_manufacturerTable->addItem(row, 0, name);
    }
}

void OptionsDialog::removeManufacturerHandler()
{
    QString name = m_manufacturerTable->currentItem()->text();

    if(message(tr("Are you sure you want to remove manufacturer \"")
               + name + tr("\"?")))
    {
        qDebug() << "remove manufacturer" <<  name;
        int row = m_manufacturerTable->currentRow();
        m_manufacturerTable->removeRow(row);

        m_co->removeManufacturer(name);
    }
}

bool OptionsDialog::message(const QString &msg)
{
    QMessageBox msgBox;
    msgBox.setText(msg);    
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int b = msgBox.exec();    

    //int b = QMessageBox::question(this, tr("Confirm"), msg, QMessageBox::Yes, QMessageBox::No);

    if(b == QMessageBox::Yes)
        return true;
    else
        return false;
}

void OptionsDialog::updateInterface()
{
    // Container
    if(ui->container_lineEdit->text().isEmpty())
        ui->addContainer_toolButton->setEnabled(false);
    else
        ui->addContainer_toolButton->setEnabled(true);

    if(m_containerTable->selectedItems().isEmpty())
        ui->removeContainer_toolButton->setEnabled(false);
    else
        ui->removeContainer_toolButton->setEnabled(true);

    // Package
    if(ui->package_lineEdit->text().isEmpty())
        ui->addPackage_toolButton->setEnabled(false);
    else
        ui->addPackage_toolButton->setEnabled(true);

    if(m_packageTable->selectedItems().isEmpty())
        ui->removePackage_toolButton->setEnabled(false);
    else
        ui->removePackage_toolButton->setEnabled(true);

    // Primary Label
    if(ui->primaryLabel_lineEdit->text().isEmpty())
        ui->addPrimaryLabel_toolButton->setEnabled(false);
    else
        ui->addPrimaryLabel_toolButton->setEnabled(true);

    if(m_primaryLabelTable->selectedItems().isEmpty())
        ui->removePrimaryLabel_toolButton->setEnabled(false);
    else
        ui->removePrimaryLabel_toolButton->setEnabled(true);

    // Secondary Label
    if(ui->secondaryLabel_lineEdit->text().isEmpty())
        ui->addSecondaryLabel_toolButton->setEnabled(false);
    else
        ui->addSecondaryLabel_toolButton->setEnabled(true);

    if(m_secondaryLabelTable->selectedItems().isEmpty())
        ui->removeSecondaryLabel_toolButton->setEnabled(false);
    else
        ui->removeSecondaryLabel_toolButton->setEnabled(true);

    // Manufacturer
    if(ui->manufacturer_lineEdit->text().isEmpty())
        ui->addManufacturer_toolButton->setEnabled(false);
    else
        ui->addManufacturer_toolButton->setEnabled(true);

    if(m_manufacturerTable->selectedItems().isEmpty())
        ui->removeManufacturer_toolButton->setEnabled(false);
    else
        ui->removeManufacturer_toolButton->setEnabled(true);
}

QString filePath;

void OptionsDialog::browseFile()
{
    filePath = QFileDialog::getOpenFileName(this,tr("Select Excel BOM File"),"",tr("BOM (*.xlsx *.xls)"));

    if(!filePath.isNull())
    {
        ui->PoductFileAdr_lineEdit->setText(filePath);
        CheckBOM();       
    }
    else
    {
        ui->ProductInfo_textEdit->setText("No file selected..");
        ui->PoductFileAdr_lineEdit->setText("");
        ui->PoductAdd_pushButton->setEnabled(false);
        ui->PoductCheck_pushButton->setEnabled(false);
        ui->PoductReduce_pushButton->setEnabled(false);
        ui->PoductMax_pushButton->setEnabled(false);
    }
}

void OptionsDialog::CheckBOM()
{
    QString str=ui->ProductBOMCount_spinBox->text();
    int BOMCount = str.toInt();

    if(BOMCount==0)
    {
        ui->ProductInfo_textEdit->setText("BOM Count does not exist..");
        return;
    }    
    ui->ProductInfo_textEdit->setText("Cheking please wait..");
    ui->PoductAdd_pushButton->setEnabled(false);
    ui->PoductCheck_pushButton->setEnabled(false);
    ui->PoductReduce_pushButton->setEnabled(false);   
    ui->PoductMax_pushButton->setEnabled(false); 
    qApp->processEvents();  

    ui->ProductInfo_textEdit->setText("File reading..\r\n");

    QAxObject* excel = new QAxObject( "Excel.Application", 0 );
    QAxObject* workbooks = excel->querySubObject( "Workbooks" );
    QAxObject* workbook = workbooks->querySubObject( "Open(const QString&)", filePath );
    QAxObject* sheets = workbook->querySubObject( "Worksheets" );
    QAxObject* sheet = sheets->querySubObject( "Item( int )", 1 );

    bool FindComponentError = false;
    bool ReduceStockError = false;
    bool AddStockError = false;
    int instock = 0;
    int ComponentCount = 0;

    for (int row=2; row <= 999; row++)
    {        
        //------------- StockNo Find --------------
        QAxObject* cell = sheet->querySubObject("Cells(int,int)",row,1);
        QString StockNo = cell->dynamicCall("Value()").toString();
        cell = sheet->querySubObject("Cells(int,int)",row,2);
        int CountNumber = cell->dynamicCall("Value()").toInt() * BOMCount;         
        cell = sheet->querySubObject("Cells(int,int)",row,3);
        QString Designator= cell->dynamicCall("Value()").toString();                
        
        FindComponentError=true;  

        if(StockNo == "" && CountNumber == 0 && Designator == "")
        {            
            break;
        }           

        foreach(QString name, m_co->componentNames())
        {            
            if(name == StockNo)
            {
                FindComponentError=false;  
                ComponentCount++;              
                foreach(Component *c, m_co->components())
                {
                    if(c->name() == name)
                    {
                        foreach(Package* p, m_co->getPackages())
                        {
                            Stock *s = c->stock(p->name());
                            if(s)
                            {
                                instock = s->stock();                                            
                                break;
                            }
                        }                                 
                        break;
                    }                                          
                }
                break;
            }
        }      
        //----------------- Count Check ---------------
        if(FindComponentError == true)
        {
            ReduceStockError = true;
            AddStockError = true;
            ui->ProductInfo_textEdit->append("Missing: " + StockNo  + " => " + Designator);
        }
        else
        {      
            if(instock == 0)
            {
                ReduceStockError = true;
                ui->ProductInfo_textEdit->append("No Stock: " + StockNo  + " => " + Designator + "(-" + QString::number(CountNumber) + ")");
            }
            else if(instock < CountNumber)
            {                        
                ReduceStockError = true;      
                ui->ProductInfo_textEdit->append("Low Stock: " + StockNo  + " => " + Designator + "(-" + QString::number(CountNumber - instock) + ")");
            }
        }
        //---------------------------------------------          
    }
    workbook->dynamicCall("Close()");
    excel->dynamicCall("Quit()");
    ui->PoductCheck_pushButton->setEnabled(true);  
    ui->PoductMax_pushButton->setEnabled(true);    
    if(ReduceStockError == false)
    {
        ui->PoductReduce_pushButton->setEnabled(true);      
    }
    if(AddStockError == false)
    {
        ui->PoductAdd_pushButton->setEnabled(true);        
    }
    if(ComponentCount && AddStockError == false && ReduceStockError == false)
    {
        ui->ProductInfo_textEdit->append("Has a enough stock.");        
    }    
    ui->ProductInfo_textEdit->append("\r\nCheking done...");
}

void OptionsDialog::ReduceBOM()
{
    QString str=ui->ProductBOMCount_spinBox->text();
    int BOMCount = str.toInt();

    if(BOMCount==0)
    {
        ui->ProductInfo_textEdit->setText("BOM Count does not exist..");
        return;
    }    

    if(!message(tr("Do you want to REDUCE the BOM list?")))
    {
        ui->ProductInfo_textEdit->setText("Cancelled..");
        return;
    }                   

    ui->ProductInfo_textEdit->setText("Reducing please wait..");    
    ui->PoductAdd_pushButton->setEnabled(false);
    ui->PoductCheck_pushButton->setEnabled(false);
    ui->PoductReduce_pushButton->setEnabled(false);
    ui->PoductMax_pushButton->setEnabled(false);
    qApp->processEvents();

    ui->ProductInfo_textEdit->setText("File Open..\r\n");

    QAxObject* excel = new QAxObject( "Excel.Application", 0 );
    QAxObject* workbooks = excel->querySubObject( "Workbooks" );
    QAxObject* workbook = workbooks->querySubObject( "Open(const QString&)", filePath );
    QAxObject* sheets = workbook->querySubObject( "Worksheets" );
    QAxObject* sheet = sheets->querySubObject( "Item( int )", 1 );

    bool FindStock = false;

    for (int row=2; row <= 999; row++)
    {        
        //------------- StockNo Find --------------
        QAxObject* cell = sheet->querySubObject("Cells(int,int)",row,1);
        QString StockNo = cell->dynamicCall("Value()").toString();
        cell = sheet->querySubObject("Cells(int,int)",row,2);
        int CountNumber = cell->dynamicCall("Value()").toInt() * BOMCount;         
        cell = sheet->querySubObject("Cells(int,int)",row,3);
        QString Designator= cell->dynamicCall("Value()").toString();        

        if(StockNo == "" && CountNumber == 0 && Designator == "")
        {
            if(FindStock == true)
            {
                ui->ProductInfo_textEdit->append("Reduce done...");
            }
            break;
        }        
        FindStock = false;
        foreach(QString name, m_co->componentNames())
        {            
            if(name == StockNo)
            {
                FindStock=true;
                foreach(Component *c, m_co->components())
                {
                    if(c->name() == name)
                    {
                        foreach(Package* p, m_co->getPackages())
                        {
                            Stock *s = c->stock(p->name());
                            if(s)
                            {                                
                                s->setStock(s->stock() - CountNumber);                                             
                                c->setTotalStock(c->totalStock() - CountNumber);
                                break;
                            }
                        }                         
                    }
                }                                                     
                break;
            }
        }
    }
    workbook->dynamicCall("Close()");
    excel->dynamicCall("Quit()");

    ui->PoductAdd_pushButton->setEnabled(true);
    ui->PoductCheck_pushButton->setEnabled(true);
    ui->PoductMax_pushButton->setEnabled(true);
}

void OptionsDialog::AddBOM()
{  
    QString str=ui->ProductBOMCount_spinBox->text();
    int BOMCount = str.toInt();

    if(BOMCount==0)
    {
        ui->ProductInfo_textEdit->setText("BOM Count does not exist..");
        return;
    }    

    if(!message(tr("Do you want to Add the BOM list.")))
    {
        ui->ProductInfo_textEdit->setText("Cancelled..");
        return;
    }         

    ui->ProductInfo_textEdit->setText("Adding please wait.."); 
    ui->PoductAdd_pushButton->setEnabled(false);
    ui->PoductCheck_pushButton->setEnabled(false);
    ui->PoductReduce_pushButton->setEnabled(false);
    ui->PoductMax_pushButton->setEnabled(false);
    qApp->processEvents();

    ui->ProductInfo_textEdit->setText("File Open..\r\n");

    QAxObject* excel = new QAxObject( "Excel.Application", 0 );
    QAxObject* workbooks = excel->querySubObject( "Workbooks" );
    QAxObject* workbook = workbooks->querySubObject( "Open(const QString&)", filePath );
    QAxObject* sheets = workbook->querySubObject( "Worksheets" );
    QAxObject* sheet = sheets->querySubObject( "Item( int )", 1 );

    bool FindStock = false;

    for (int row=2; row <= 999; row++)
    {        
        //------------- StockNo Find --------------
        QAxObject* cell = sheet->querySubObject("Cells(int,int)",row,1);
        QString StockNo = cell->dynamicCall("Value()").toString();
        cell = sheet->querySubObject("Cells(int,int)",row,2);
        int CountNumber = cell->dynamicCall("Value()").toInt() * BOMCount;         
        cell = sheet->querySubObject("Cells(int,int)",row,3);
        QString Designator= cell->dynamicCall("Value()").toString();        

        if(StockNo == "" && CountNumber == 0 && Designator == "")
        {
            if(FindStock == true)
            {
                ui->ProductInfo_textEdit->append("Add done...");
            }
            break;
        }        
        FindStock = false;
        foreach(QString name, m_co->componentNames())
        {            
            if(name == StockNo)
            {
                FindStock=true;
                foreach(Component *c, m_co->components())
                {
                    if(c->name() == name)
                    {
                        foreach(Package* p, m_co->getPackages())
                        {
                            Stock *s = c->stock(p->name());
                            if(s)
                            {
                                s->setStock(s->stock() + CountNumber);     
                                c->setTotalStock(c->totalStock() + CountNumber);                                        
                                break;
                            }
                        }                         
                    }
                }                                                     
                break;
            }
        }
    }
    workbook->dynamicCall("Close()");
    excel->dynamicCall("Quit()");

    ui->PoductAdd_pushButton->setEnabled(true);
    ui->PoductCheck_pushButton->setEnabled(true);
    ui->PoductReduce_pushButton->setEnabled(true);  
    ui->PoductMax_pushButton->setEnabled(true);  
}

void OptionsDialog::MaximumBOMCalc()
{    
    ui->PoductAdd_pushButton->setEnabled(false);
    ui->PoductCheck_pushButton->setEnabled(false);
    ui->PoductReduce_pushButton->setEnabled(false);   
    ui->PoductMax_pushButton->setEnabled(false);     

    ui->ProductInfo_textEdit->setText("Calculating..\r\n");

    qApp->processEvents();  

    QAxObject* excel = new QAxObject( "Excel.Application", 0 );
    QAxObject* workbooks = excel->querySubObject( "Workbooks" );
    QAxObject* workbook = workbooks->querySubObject( "Open(const QString&)", filePath );
    QAxObject* sheets = workbook->querySubObject( "Worksheets" );
    QAxObject* sheet = sheets->querySubObject( "Item( int )", 1 );

    bool FindStock;
    bool ReduceStockError;
    bool AddStockError;
    int instock;
    int BOMCount = 1;

    while(BOMCount < 99999)
    {
        FindStock = false;
        ReduceStockError = false;
        AddStockError = false;
        instock = 0;

        for (int row=2; row <= 999; row++)
        {        
            //------------- StockNo Find --------------
            QAxObject* cell = sheet->querySubObject("Cells(int,int)",row,1);
            QString StockNo = cell->dynamicCall("Value()").toString();
            cell = sheet->querySubObject("Cells(int,int)",row,2);
            int CountNumber = cell->dynamicCall("Value()").toInt() * BOMCount;         
            cell = sheet->querySubObject("Cells(int,int)",row,3);
            QString Designator= cell->dynamicCall("Value()").toString();        

            if(StockNo == "" && CountNumber == 0 && Designator == "")
            {
                break;
            }        
            FindStock = false;
            foreach(QString name, m_co->componentNames())
            {            
                if(name == StockNo)
                {
                    FindStock=true;                
                    foreach(Component *c, m_co->components())
                    {
                        if(c->name() == name)
                        {
                            foreach(Package* p, m_co->getPackages())
                            {
                                Stock *s = c->stock(p->name());
                                if(s)
                                {
                                    instock = s->stock();                                            
                                    break;
                                }
                            }                                 
                            break;
                        }                                          
                    }
                    break;
                }
            }
            //----------------- Count Check ---------------
            if(FindStock == false)
            {
                ReduceStockError = true;
                AddStockError = true;
                break;
            }
            else
            {      
                if(instock < CountNumber)
                {                        
                    ReduceStockError = true;      
                    break;
                }
            }
            //---------------------------------------------
        }
        if(ReduceStockError || AddStockError)
        {
            break;
        }
        else
        {
            ui->ProductBOMCount_spinBox->setValue(BOMCount);  
            BOMCount++;
        }
    }
    workbook->dynamicCall("Close()");
    excel->dynamicCall("Quit()");      
    CheckBOM();   
}

void OptionsDialog::CheckRequest()
{
    ui->PoductReduce_pushButton->setEnabled(false);   
}


//---------------------------------------------------

QString BOMfilePath;
QString PlacefilePath;

void OptionsDialog::SmtBrowseBOMFile()
{
    BOMfilePath = QFileDialog::getOpenFileName(this,tr("Select Excel BOM File"),"",tr("BOM (*.xlsx *.xls)"));

    if(!BOMfilePath.isNull())
    {
        ui->SmtBOMAdr_lineEdit->setText(BOMfilePath);
        ui->SmtInfo_textEdit->setText("BOM file selected.");
    }
    else
    {
        ui->SmtInfo_textEdit->setText("No file selected..");
    }
    if(ui->SmtBOMAdr_lineEdit->text() != "" && ui->SmtPlaceAdr_lineEdit->text() != "")
    {
        ui->SmtGenerateFile_pushButton->setEnabled(true);
        ui->SmtInfo_textEdit->setText("Ready to generate..");
    }
    else
    {
        ui->SmtInfo_textEdit->append("Please select the Place file...");
    }
}

void OptionsDialog::SmtBrowsePlaceFile()
{
    PlacefilePath = QFileDialog::getOpenFileName(this,tr("Select Excel PICK PLACE File"),"",tr("Place (*.xlsx *.xls)"));

    if(!PlacefilePath.isNull())
    {
        ui->SmtPlaceAdr_lineEdit->setText(PlacefilePath);
        ui->SmtInfo_textEdit->setText("Place file selected.");
    }
    else
    {
        ui->SmtInfo_textEdit->setText("No file selected..");
    }
    if(ui->SmtBOMAdr_lineEdit->text() != "" && ui->SmtPlaceAdr_lineEdit->text() != "")
    {
        ui->SmtGenerateFile_pushButton->setEnabled(true);
        ui->SmtInfo_textEdit->setText("Ready to generate..");
    }
    else
    {
        ui->SmtInfo_textEdit->append("Please select the BOM file...");
    }
}

void OptionsDialog::SmtGenerateFile()
{
    ui->SmtInfo_textEdit->setText("Generate done..");
}

