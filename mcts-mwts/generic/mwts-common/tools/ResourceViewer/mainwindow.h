#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTcpServer>
#include <QMessageBox>
#include "incrementalplot.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:
    void newData();
    void OnNewConnection();

private:
    Ui::MainWindow *ui;
    QTcpSocket* tcpSocket;
    QTcpServer* tcpServer;
    IncrementalPlot* m_pPlotWidget;
};

#endif // MAINWINDOW_H
