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
    void aboutQt();

    void on_refreshButton_clicked();
    void on_addButton_clicked();
    void on_removeButton_clicked();

private:
    Ui::MainWindow* ui;
    QStandardItemModel* m_table;
    QFile* m_currentFile;

    bool m_saved;

    void updateTable();
    void updateValues();
};

#endif // MAINWINDOW_H
