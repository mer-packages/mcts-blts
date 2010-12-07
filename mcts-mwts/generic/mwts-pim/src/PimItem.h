#ifndef PIMITEM_H
#define PIMITEM_H

// Includes
#include <QOrganizerItemId>
#include <QContact>

// namespace
QTM_USE_NAMESPACE

/**
  * Helper class for storing calendar items
  */
class PimItem {

public: // constructor

    PimItem(QOrganizerItemId iId=QOrganizerItemId(), QContactLocalId cId=-1, QString name="default", int iType=-1)
    {
        itemLocalId = iId;
        contactLocalId = cId;
        dataStoreName = name;
        itemType = iType;
    }

public: // data

    int itemType;
    QString dataStoreName;
    QOrganizerItemId itemLocalId;
    QContactLocalId contactLocalId;
};

#endif // PIMITEM_H
