#ifndef _INCREMENTALPLOT_H_
#define _INCREMENTALPLOT_H_ 1

#include <qwt_array.h>
#include <qwt_plot.h>

class QwtPlotCurve;

#define NUM_PLOTS       4
#define PLOT_CPU_USER   0
#define PLOT_CPU_SYS    1
#define PLOT_CPU_IOWAIT 2
#define PLOT_CPU_TOTAL  3

class CurveData
{
    // A container class for growing data
public:

    CurveData();

    void append(double *x, double *y, int count);

    int count() const;
    int size() const;
    const double *x() const;
    const double *y() const;

private:
    int d_count;
    QwtArray<double> d_x;
    QwtArray<double> d_y;
};



class IncrementalPlot : public QwtPlot
{
    Q_OBJECT
public:
    IncrementalPlot(QWidget *parent = NULL);
    virtual ~IncrementalPlot();

    void appendData(int nCurve, double x, double y);
    void appendData(int nCurve, double *x, double *y, int size);

    void removeData();

private:
    CurveData* d_data[NUM_PLOTS];
    QwtPlotCurve* d_curve[NUM_PLOTS];

};

#endif // _INCREMENTALPLOT_H_
