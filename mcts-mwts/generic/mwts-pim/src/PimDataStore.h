#ifndef PIMDATASTORE_H
#define PIMDATASTORE_H

// Includes
#include <QOrganizerManager>
#include <QContactManager>

// namespace
QTM_USE_NAMESPACE

/**
  * Helper class for storing calendar and contacts data stores
  */
class PimDataStore {

public: // constructor

    PimDataStore(QOrganizerManager *mngr=0, QContactManager *cntcMngr=0 ,QString name="default")
    {
        dataStoreName = name;
        calendarManager = mngr;
        contactManager = cntcMngr;
    }

public: // data

    QString dataStoreName;
    QOrganizerManager *calendarManager;
    QContactManager *contactManager;
};

#endif // PIMDATASTORE_H
