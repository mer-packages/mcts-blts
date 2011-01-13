#include "ofonotestinterface.h"

OfonoTestInterface::OfonoTestInterface(QObject *parent) :
    QObject(parent)
{
    MWTS_ENTER;

    //needs to have to have proper waiting time until getting the dbus answer
    mEventLoop = new QEventLoop;
    mTimer = new QTimer;
    connect( mTimer, SIGNAL(timeout()),
             this, SLOT(testTimeout()));

    MWTS_LEAVE;
}

void OfonoTestInterface::OnUninitialize()
{
    MWTS_ENTER;

    if (mTimer->isActive())
    {
        qDebug() << "Timer active, stopping it...";
        mTimer->stop();
    }

    if (mEventLoop->isRunning())
    {
        qDebug() << "Event loop running, stopping it...";
        mEventLoop->exit();
    }

    qDebug() << "Killing timer...";

    if (mTimer)
    {
        if (mTimer->isActive())
        {
            qDebug() << "Timer active, stopping it...";
            mTimer->stop();
        }
        qDebug() << "Deleting timer...";
        delete mTimer;
        mTimer = NULL;
    }

    qDebug() << "Killing event loop...";
    if (mEventLoop)
    {
        if (mEventLoop->isRunning())
        {
            qDebug() << "Event loop running, stoping it...";
            mEventLoop->exit();
        }
        qDebug() << "Deleting event loop...";
        delete mEventLoop;
        mEventLoop = NULL;
    }

    MWTS_ENTER;
}

void OfonoTestInterface::testTimeout()
{
    MWTS_ENTER;

    if ( mEventLoop->isRunning() )
    {
        qDebug() << "Event loop running! Stopping...";
        mTimer->stop();
        mEventLoop->exit();
    }

    MWTS_LEAVE;
}
