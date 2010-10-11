#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <qwt_plot_grid.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_layout.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    m_pPlotWidget = ui->plotWidget;
    m_pPlotWidget->setMargin(4);

    m_pPlotWidget->updateLayout();


    m_pPlotWidget->setFrameStyle(QFrame::NoFrame);
    m_pPlotWidget->setLineWidth(0);
    m_pPlotWidget->setCanvasLineWidth(2);


    m_pPlotWidget->plotLayout()->setAlignCanvasToScales(true);

    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->setMajPen(QPen(Qt::gray, 0, Qt::DotLine));
    grid->attach( m_pPlotWidget);

    m_pPlotWidget->setCanvasBackground(QColor(60, 60, 60));

    m_pPlotWidget->setAxisScale(m_pPlotWidget->xBottom, 0, 10);
    m_pPlotWidget->setAxisScale(m_pPlotWidget->yLeft, 0, 100);

    m_pPlotWidget->replot();



    tcpSocket = NULL;
    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen(QHostAddress::Any, 12345))
    {
        QMessageBox::critical(this, "ResourceViewer",
            "Unable to start the server: " + tcpServer->errorString());
    }
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(OnNewConnection()));

    qDebug()<< "Starting";
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::newData()
{
    static int nCounter=0;
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);

    if (tcpSocket->bytesAvailable() <= 0 )
    {
        return;
    }
    QByteArray data = tcpSocket->readAll();

    QString str(data);
    QStringList values=str.split(' ', QString::SkipEmptyParts);
    if(values.size()<10)
    {
        return;
    }
    qDebug()<< values;
//    06:50:02     all    9.00    0.00    7.00    0.00    1.00    0.00    0.00   83.00    692.00
//    qDebug()<<data;

    double cpu_user=values.at(2).toDouble();
    double cpu_sys=values.at(4).toDouble();
    double cpu_iowait=values.at(5).toDouble();
    double cpu_idle=values.at(9).toDouble();
    double cpu_total=100.0-cpu_idle;

    m_pPlotWidget->appendData(PLOT_CPU_SYS, nCounter, cpu_sys);
    m_pPlotWidget->appendData(PLOT_CPU_USER, nCounter, cpu_user);
    m_pPlotWidget->appendData(PLOT_CPU_IOWAIT, nCounter, cpu_iowait);
    m_pPlotWidget->appendData(PLOT_CPU_TOTAL, nCounter, cpu_total);
    if(nCounter>10)
    {
        m_pPlotWidget->setAxisScale(m_pPlotWidget->xBottom, 0, nCounter);
    }
//    m_pPlotWidget->updateLayout();

    m_pPlotWidget->replot();
    nCounter++;

}


void MainWindow::OnNewConnection()
{
    qDebug()<< "New connection!!";

    tcpSocket = tcpServer->nextPendingConnection();

    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(newData()));

    connect(tcpSocket, SIGNAL(disconnected()),
            tcpSocket, SLOT(deleteLater()));

    tcpSocket->write("blaablaa");

}
