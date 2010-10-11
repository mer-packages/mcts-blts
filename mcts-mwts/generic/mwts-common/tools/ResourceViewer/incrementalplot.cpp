#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include "incrementalplot.h"
#if QT_VERSION >= 0x040000
#include <qpaintengine.h>
#endif

CurveData::CurveData():
    d_count(0)
{
}

void CurveData::append(double *x, double *y, int count)
{
    int newSize = ( (d_count + count) / 1000 + 1 ) * 1000;
    if ( newSize > size() )
    {
        d_x.resize(newSize);
        d_y.resize(newSize);
    }

    for ( register int i = 0; i < count; i++ )
    {
        d_x[d_count + i] = x[i];
        d_y[d_count + i] = y[i];
    }
    d_count += count;


}

int CurveData::count() const
{
    return d_count;
}

int CurveData::size() const
{
    return d_x.size();
}

const double *CurveData::x() const
{
    return d_x.data();
}

const double *CurveData::y() const
{
    return d_y.data();
}

IncrementalPlot::IncrementalPlot(QWidget *parent):
    QwtPlot(parent)
{
    setAutoReplot(true);

    int i;
    for(i=0; i<NUM_PLOTS; i++)
    {
        d_data[i]=new CurveData;;
        d_curve[i]=new QwtPlotCurve;
        d_curve[i]->setStyle(QwtPlotCurve::Lines);
        d_curve[i]->setPaintAttribute(QwtPlotCurve::PaintFiltered);
        d_curve[i]->setPen(QColor(Qt::white));
        const QColor &c = Qt::white;
        d_curve[i]->setSymbol(QwtSymbol(QwtSymbol::XCross,
            QBrush(c), QPen(c), QSize(10, 10)) );
        d_curve[i]->attach(this);
    }
}

IncrementalPlot::~IncrementalPlot()
{
    delete d_data;
}

void IncrementalPlot::appendData(int nCurve, double x, double y)
{
    appendData(nCurve, &x, &y, 1);
}

void IncrementalPlot::appendData(int nCurve, double *x, double *y, int size)
{
    QwtPlotCurve* curve=d_curve[nCurve];
    CurveData* data=d_data[nCurve];
    data->append(x, y, size);
    curve->setRawData(data->x(), data->y(), data->count());

    const bool cacheMode =
        canvas()->testPaintAttribute(QwtPlotCanvas::PaintCached);

    canvas()->setAttribute(Qt::WA_PaintOutsidePaintEvent, true);

    canvas()->setPaintAttribute(QwtPlotCanvas::PaintCached, false);
    curve->draw(curve->dataSize() - size, curve->dataSize() - 1);
    canvas()->setPaintAttribute(QwtPlotCanvas::PaintCached, cacheMode);

    canvas()->setAttribute(Qt::WA_PaintOutsidePaintEvent, false);
}

void IncrementalPlot::removeData()
{
/*    delete d_curve;
    d_curve = NULL;

    delete d_data;
    d_data = NULL;
*/
    replot();
}
