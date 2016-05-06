#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDate>
#include <QTextStream>
#include <QString>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_table(nullptr),
    m_currentFile(nullptr),
    m_saved(true)
{
    //Reset focus
    setFocus();

    //UI Setup
    ui->setupUi(this);

    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    ui->dateEdit->setDate(QDate::currentDate());

    //Update the table
    m_currentFile = new QFile("statistics.csv");

    updateTable();

    //Signals & slots
    connect(m_table, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(tableChanged()));

    connect(ui->actionNew, SIGNAL(triggered(bool)), this, SLOT(newFile()));
    connect(ui->actionSave, SIGNAL(triggered(bool)), this, SLOT(save()));
    connect(ui->actionSave_as, SIGNAL(triggered(bool)), this, SLOT(saveAs()));
    connect(ui->actionOpen, SIGNAL(triggered(bool)), this, SLOT(open()));
    connect(ui->actionExit, SIGNAL(triggered(bool)), this, SLOT(close()));
    connect(ui->actionAbout_Qt, SIGNAL(triggered(bool)), this, SLOT(aboutQt()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    event->ignore();
    close();
}

void MainWindow::updateTable()
{
    //Setting up the table & loading from file
    m_table = new QStandardItemModel;

    m_table->setHorizontalHeaderItem(0, new QStandardItem(QString("Date")));
    m_table->setHorizontalHeaderItem(1, new QStandardItem(QString("Winners")));
    m_table->setHorizontalHeaderItem(2, new QStandardItem(QString("Losers")));
    m_table->setHorizontalHeaderItem(3, new QStandardItem(QString("Amount")));


    if(!m_currentFile->open(QIODevice::ReadOnly))
        qDebug() << m_currentFile->errorString();


    QTextStream in(m_currentFile);
    int row = 0;
    while(!in.atEnd()) {
        QString line = in.readLine();

        //parse the line into separate pieces(tokens) with ";" as the delimiter
        QStringList lineToken = line.split(";", QString::SkipEmptyParts);

        // load parsed data to model accordingly
        for (int column = 0; column < lineToken.size(); column++) {
            QString value = lineToken.at(column);
            QStandardItem *item = new QStandardItem(value);

            //add item to the model (row, column, item)
            m_table->setItem(row, column, item);
        }

        row++;
    }
    m_currentFile->close();

    ui->tableView->setModel(m_table);

    //Updating values
    updateValues();
}

void MainWindow::updateValues()
{
    double total = 0;
    for(int i = 0; i < m_table->rowCount(); i++) {
        QString string = m_table->item(i, 3)->text();

        total += string.toDouble();
    }

    ui->totalLineEdit->setText(QString::number(total));
}

//// Signals & slots /////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::tableChanged()
{
    m_saved = false;
}

void MainWindow::newFile()
{
    QString path = QFileDialog::getSaveFileName(this, "New File", "./statistics.csv", "*.csv");

    m_currentFile = new QFile(path);

    if(!m_currentFile->open(QIODevice::WriteOnly | QIODevice::Text))
        qDebug() << m_currentFile->errorString();

    QTextStream out(m_currentFile);
        out << ";";

    m_currentFile->close();

    updateTable();
}

void MainWindow::open()
{
    QString path = QFileDialog::getOpenFileName(this, "Open File", "./", "*.csv");

    m_currentFile = new QFile(path);

    updateTable();
}

void MainWindow::save()
{
    if(!m_currentFile->open(QIODevice::WriteOnly | QIODevice::Text))
        qDebug() << m_currentFile->errorString();

    QTextStream out(m_currentFile);
    for(int row = 0; row < m_table->rowCount(); row++) {
        for(int col = 0; col < 4; col++) {
            out << m_table->item(row, col)->text() << ";";
        }
        out << "\n";
    }

    m_currentFile->close();
}

void MainWindow::saveAs()
{
    QString path = QFileDialog::getSaveFileName(this, "Save File", "./statistics.csv", "*.csv");
    m_currentFile = new QFile(path);

    save();
}

void MainWindow::close()
{
    //If changes saved
    if(m_saved) {
        QApplication::exit();
        return;
    }

    //If changes unsaved
    int result = QMessageBox::question(this, "Changes unsaved", "Changes unsaved. Would you like to save them?",
                                       QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                       QMessageBox::Yes);

    if(result == QMessageBox::Cancel)
        return;
    else if(result == QMessageBox::No)
        QApplication::exit();
    else if(result == QMessageBox::Yes) {
        save();
        QApplication::exit();
    }

}

void MainWindow::aboutQt()
{
    qApp->aboutQt();
}


void MainWindow::on_refreshButton_clicked()
{
    updateTable();
}

void MainWindow::on_addButton_clicked()
{    
    //Check if all information is entered
    if(ui->dateEdit->date().toString("yyyy.MM.dd") == "" || ui->winnersLineEdit->text() == "" ||
       ui->losersLineEdit->text() == "" || ui->amountLineEdit->text() == "") {
        QMessageBox::information(this, "CSGO Lounge Statistics", "You need to enter all information!");

        return;
    }

    //Add new row
    QStandardItem* date = new QStandardItem(ui->dateEdit->date().toString("yyyy.MM.dd"));
    QStandardItem* winners = new QStandardItem(ui->winnersLineEdit->text());
    QStandardItem* losers = new QStandardItem(ui->losersLineEdit->text());
    QStandardItem* amount = new QStandardItem(ui->amountLineEdit->text());

    int rows = m_table->rowCount();

    m_table->setItem(rows, 0, date);
    m_table->setItem(rows, 1, winners);
    m_table->setItem(rows, 2, losers);
    m_table->setItem(rows, 3, amount);

    //Reset line edits
    ui->winnersLineEdit->clear();
    ui->losersLineEdit->clear();
    ui->amountLineEdit->clear();

    m_saved = false;
    updateValues();
}

void MainWindow::on_removeButton_clicked()
{
    QModelIndexList selection = ui->tableView->selectionModel()->selectedRows();
    if(selection.count() == 0)
        return;

    m_table->removeRows(selection.first().row(), selection.count());

    m_saved = false;
    updateValues();
}
