#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDate>
#include <QTextStream>
#include <QString>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QRegExpValidator>
#include <QList>
#include <QVector>
#include <QPen>
#include <QColor>

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

    QRegExpValidator* validator = new QRegExpValidator(QRegExp("[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?"), this);
    ui->amountLineEdit->setValidator(validator);

    //Update the table
    if(getLastFilePath() == "") disableUi();
    m_currentFile = new QFile(getLastFilePath());

    if(m_currentFile->fileName() == "" || !m_currentFile->exists()) disableUi();

    loadTable();
    setupPlot();

    //Signals & slots
    connect(m_table, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(tableChanged()));
    connect(m_table, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(tableChanged()));

    connect(ui->actionNew, SIGNAL(triggered(bool)), this, SLOT(newFile()));
    connect(ui->actionSave, SIGNAL(triggered(bool)), this, SLOT(save()));
    connect(ui->actionSave_as, SIGNAL(triggered(bool)), this, SLOT(saveAs()));
    connect(ui->actionOpen, SIGNAL(triggered(bool)), this, SLOT(open()));
    connect(ui->actionExit, SIGNAL(triggered(bool)), this, SLOT(close()));
    connect(ui->actionAbout_Qt, SIGNAL(triggered(bool)), this, SLOT(aboutQt()));
    connect(ui->actionAbout, SIGNAL(triggered(bool)), this, SLOT(about()));

    connect(ui->addButton, SIGNAL(clicked(bool)), this, SLOT(add()));
    connect(ui->removeButton, SIGNAL(clicked(bool)), this, SLOT(remove()));
    connect(ui->resetGraphButton, SIGNAL(clicked(bool)), this, SLOT(resetGraph()));
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

void MainWindow::loadTable()
{
    //Setting up the table & loading from file
    m_table = new QStandardItemModel;

    m_table->setHorizontalHeaderItem(0, new QStandardItem(QString("Date")));
    m_table->setHorizontalHeaderItem(1, new QStandardItem(QString("Winners")));
    m_table->setHorizontalHeaderItem(2, new QStandardItem(QString("Losers")));
    m_table->setHorizontalHeaderItem(3, new QStandardItem(QString("Amount")));


    if(!m_currentFile->open(QIODevice::ReadOnly)) {
        qDebug() << m_currentFile->errorString();
        return;
    }


    QTextStream in(m_currentFile);
    unsigned int row = 0;
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
    updateValues();
}

void MainWindow::updateValues()
{
    if(m_table->rowCount() == 0)
        return;

    //Total bets
    //Bets won
    //Bets lost
    //Total money
    //Money won
    //Money lost
    //Max won
    //Max lost
    //Best team
    //Worst team

    int betsWon = 0, betsLost = 0;
    double totalMoney = 0, moneyWon = 0, moneyLost = 0;
    double maxWon = 0, maxLost = 0;

    //Money
    for(int i = 0; i < m_table->rowCount(); i++) {
        QString string = m_table->item(i, 3)->text();
        double value = string.toDouble();

        if(value >= 0) {
            betsWon++;
            moneyWon += value;

            if(i == 0) maxWon = value;
            else if(value > maxWon) maxWon = value;
        }
        else if(value < 0) {
            betsLost++;
            moneyLost += value;

            if(i == 0) maxLost = value;
            else if(value < maxLost) maxLost = value;
        }

        totalMoney += value;
    }

    updateBestWorstTeams();

    ui->totalBetsLineEdit->setText(QString::number(m_table->rowCount()));
    ui->betsLostLineEdit->setText(QString::number(betsLost));
    ui->betsWonLineEdit->setText(QString::number(betsWon));
    ui->totalMoneyLineEdit->setText(QString::number(totalMoney));
    ui->moneyLostLineEdit->setText(QString::number(moneyLost));
    ui->moneyWonLineEdit->setText(QString::number(moneyWon));
    ui->maxWonLineEdit->setText(QString::number(maxWon));
    ui->maxLostLineEdit->setText(QString::number(maxLost));
}

void MainWindow::updateBestWorstTeams()
{
    int maxWins = 0, bestTeamWinsRow = 0;
    int maxLosses = 0, worstTeamRow = 0;
    double maxMostMoney = 0;
    int bestTeamMoneyRow = 0;
    for(int i = 0; i < m_table->rowCount(); i++) {
        int wins = 0, losses = 0;
        double mostMoney = 0;

        for(int j = 0; j < m_table->rowCount(); j++) {
            //Best
            if(m_table->item(i, 1)->text().toUpper() == m_table->item(j, 1)->text().toUpper()) {
                wins++;
                mostMoney += m_table->item(j, 3)->text().toDouble();
            }
            //Worst
            if(m_table->item(i, 2)->text().toUpper() == m_table->item(j, 2)->text().toUpper())
                losses++;
        }

        if(wins >= maxWins) {
            maxWins = wins;
            bestTeamWinsRow = i;
        }
        if(losses >= maxLosses) {
            maxLosses = losses;
            worstTeamRow = i;
        }
        if(mostMoney >= maxMostMoney) {
            maxMostMoney = mostMoney;
            bestTeamMoneyRow = i;
        }
    }

    ui->bestTeamWinsLineEdit->setText(m_table->item(bestTeamWinsRow, 1)->text() + " (" + QString::number(maxWins) + ")");
    ui->bestTeamMoneyLineEdit->setText(m_table->item(bestTeamMoneyRow, 1)->text() + " (" + QString::number(maxMostMoney) + ")");
    ui->worstTeamLossesLineEdit->setText(m_table->item(worstTeamRow, 2)->text() + " (" + QString::number(maxLosses) + ")");
}

void MainWindow::setupPlot()
{
    if(ui->plot->graphCount() > 0)
        ui->plot->removeGraph(0);

    ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    ui->plot->addGraph();
    ui->plot->graph(0)->setPen(QPen(QColor(30, 144, 255)));

    ui->plot->xAxis->setVisible(false);
    ui->plot->xAxis->setOffset(10);
    ui->plot->xAxis->grid()->setPen(QPen(Qt::white));
    ui->plot->xAxis->grid()->setZeroLinePen(QPen(Qt::white));

    ui->plot->yAxis->setOffset(10);
    ui->plot->yAxis->grid()->setPen(QPen(Qt::white));

    ui->plot->xAxis2->setVisible(false);
    ui->plot->xAxis2->setOffset(10);
    ui->plot->xAxis2->setTicks(false);

    ui->plot->yAxis2->setVisible(true);
    ui->plot->yAxis2->setOffset(10);
    ui->plot->yAxis2->setTicks(false);

    updatePlotData();
}

void MainWindow::updatePlotData()
{
    QVector<double> x, y;
    x.push_back(0);
    y.push_back(0);

    double total = 0;

    for(int i = m_table->rowCount(); i > 0; i--) {
        x.push_back(m_table->rowCount() + 1 - i);

        total += m_table->item(i - 1, 3)->text().toDouble();
        y.push_back(total);
    }

    ui->plot->graph(0)->setData(x, y);

    ui->plot->xAxis->setRange(0, m_table->rowCount());
    ui->plot->yAxis->setRange(ui->moneyLostLineEdit->text().toDouble(), ui->moneyWonLineEdit->text().toDouble());


    ui->plot->replot();
}

void MainWindow::offerToSave()
{
    int result = QMessageBox::question(this, "Betting Statistics", "Changes unsaved. Would you like to save them?",
                                       QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                       QMessageBox::Yes);

    if(result == QMessageBox::Yes)
        save();
    else
        return;
}

QString MainWindow::getLastFilePath() const
{
    QFile file("lastfile.txt");
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << file.errorString();
        return "";
    }

    QTextStream in(&file);
    QString path = in.readLine();

    file.close();

    return path;
}

void MainWindow::enableUi()
{
    ui->dateEdit->setEnabled(true);
    ui->amountLineEdit->setEnabled(true);
    ui->winnersLineEdit->setEnabled(true);
    ui->losersLineEdit->setEnabled(true);
    ui->addButton->setEnabled(true);
    ui->removeButton->setEnabled(true);

    ui->plot->setEnabled(true);
}

void MainWindow::disableUi()
{
    ui->dateEdit->setEnabled(false);
    ui->amountLineEdit->setEnabled(false);
    ui->winnersLineEdit->setEnabled(false);
    ui->losersLineEdit->setEnabled(false);
    ui->addButton->setEnabled(false);
    ui->removeButton->setEnabled(false);

    ui->plot->removeGraph(0);
    ui->plot->replot();
    ui->plot->setEnabled(false);
}

//// Signals & slots /////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::tableChanged()
{
    ui->tableView->sortByColumn(0, Qt::DescendingOrder);

    m_saved = false;

    updateValues();
    updatePlotData();
}

void MainWindow::newFile()
{
    if(!m_saved) offerToSave();

    QString path = QFileDialog::getSaveFileName(this, "New File", "./statistics.csv", "*.csv");
    m_currentFile = new QFile(path);


    if(!m_currentFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << m_currentFile->errorString();
        return;
    }

    QTextStream out(m_currentFile);
        out << ";";

    m_currentFile->close();

    enableUi();
    loadTable();
}

void MainWindow::open()
{
    if(!m_saved) offerToSave();

    QString selectedFilter = "CSV (*.csv)";
    QString path = QFileDialog::getOpenFileName(this, "Open File", "./", "CSV (*.csv)", &selectedFilter);

    m_currentFile = new QFile(path);

    if(m_currentFile->fileName() == "") {
        ui->tableView->setModel(nullptr);
        disableUi();
    }
    else {
        enableUi();
        loadTable();
        setupPlot();
    }

}

void MainWindow::save()
{
    if(m_currentFile->fileName() == "") {
        newFile();
        return;
    }

    if(!m_currentFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << m_currentFile->errorString();
        return;
    }

    QTextStream out(m_currentFile);
    for(int row = 0; row < m_table->rowCount(); row++) {
        for(int col = 0; col < 4; col++) {
            out << m_table->item(row, col)->text() << ";";
        }
        out << "\n";
    }

    m_currentFile->close();

    m_saved = true;
}

void MainWindow::saveAs()
{
    QString path = QFileDialog::getSaveFileName(this, "Save File", "./statistics.csv", "*.csv");

    if(path != "") {
        m_currentFile = new QFile(path);
        save();
    }
}

void MainWindow::close()
{
    //Save last file path
    QFile file("lastfile.txt");

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out << m_currentFile->fileName();

    file.close();

    //If changes saved, exit the application
    if(m_saved) {
        QApplication::exit();
        return;
    }

    //If changes unsaved, ask if user would like to save them
    int result = QMessageBox::question(this, "Betting Statistics", "Changes unsaved. Would you like to save them?",
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

void MainWindow::about()
{
    QMessageBox::information(this, "About", "This program was made by dhmitry\nVersion 1.0.3", QMessageBox::Ok);
}

void MainWindow::aboutQt()
{
    qApp->aboutQt();
}

void MainWindow::add()
{    
    //Check if all information is entered
    if(ui->dateEdit->date().toString("yyyy.MM.dd") == "" || ui->winnersLineEdit->text() == "" ||
       ui->losersLineEdit->text() == "" || ui->amountLineEdit->text() == "") {
        QMessageBox::information(this, "Betting Statistics", "You need to enter all information!");

        return;
    }

    //Add new row
    QStandardItem* date = new QStandardItem(ui->dateEdit->date().toString("yyyy.MM.dd"));
    QStandardItem* winners = new QStandardItem(ui->winnersLineEdit->text());
    QStandardItem* losers = new QStandardItem(ui->losersLineEdit->text());
    QStandardItem* amount = new QStandardItem(ui->amountLineEdit->text());

    QList<QStandardItem*> newRow;
    newRow.append(date);
    newRow.append(winners);
    newRow.append(losers);
    newRow.append(amount);

    m_table->insertRow(m_table->rowCount(), newRow);

    //Reset line edits
    ui->winnersLineEdit->clear();
    ui->losersLineEdit->clear();
    ui->amountLineEdit->clear();


    ui->tableView->sortByColumn(0, Qt::DescendingOrder);

    m_saved = false;
    updateValues();
    updatePlotData();
}

void MainWindow::remove()
{
    QModelIndexList selection = ui->tableView->selectionModel()->selectedRows();

    if(selection.count() == 0)
        return;

    while(!selection.isEmpty()) {
        m_table->removeRow(selection.last().row());
        selection.removeLast();
    }


    m_saved = false;
    updateValues();
    updatePlotData();
}

void MainWindow::resetGraph()
{
    updatePlotData();
}
