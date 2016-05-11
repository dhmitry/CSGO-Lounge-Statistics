#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QFile>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void tableChanged();

    void newFile();
    void open();
    void save();
    void saveAs();
    void close();
    void about();
    void aboutQt();

    void add();
    void remove();

private:
    Ui::MainWindow* ui;
    QStandardItemModel* m_table;
    QFile* m_currentFile;

    bool m_saved;

    void loadTable();
    void updateValues();
    void updateBestWorstTeams();

    void setupPlot();
    void updatePlotData();

    void offerToSave();
    void enableUi();
    void disableUi();

    QString getLastFilePath() const;
};

#endif // MAINWINDOW_H
