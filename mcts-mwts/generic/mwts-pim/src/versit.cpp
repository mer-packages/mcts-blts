/* versit.cpp -- source code for Versit class
 *
 * This file is part of MCTS
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Tommi Toropainen; tommi.toropainen@nokia.com;
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

// class header
#include "versit.h"

// includes
#include <QStringList>
#include <QBuffer>
#include <QVersitReader>
#include <QVersitWriter>
#include <QVersitDocument>
#include <QVersitContactImporter>
#include <QVersitContactExporter>
#include <QVersitOrganizerImporter>
#include <QVersitOrganizerExporter>
#include <QFile>

/**
 * Constructor for Versit class
 */
Versit::Versit()
{
}

/**
 * Destructor for Versit class
 */
Versit::~Versit()
{
}


/**
 * ImportContactFromVcard function
 */
bool Versit::ImportContactsFromVcard(QList<QContact>& contacts, QString vCardFile)
{
    bool result;

    // see if the file exists
    QFile vcard(vCardFile);
    if(!vcard.exists())
    {
        qCritical()<<"vCard file: "<<vCardFile<<" not found";
        return false;
    }

    // open the file
    if(!vcard.open(QFile::ReadOnly))
    {
        qCritical()<<"vCard file: "<<vCardFile<<" cannot be opened";
        return false;
    }

    // read the content
    QTextStream stream(&vcard);
    while(!stream.atEnd())
    {
        m_inputVCard.append(stream.readLine());
        // next line
        m_inputVCard.append("\n");
    }
    vcard.close();

    QBuffer input;
    input.open(QBuffer::ReadWrite);
    input.write(m_inputVCard);
    //input.write(m_inputVCard);
    input.seek(0);

    qDebug()<<"Importing vCard:\n"<<m_inputVCard;

    // read vCard
    QVersitReader reader;
    reader.setDevice(&input);
    result = reader.startReading();
    reader.waitForFinished();
    if(!result)
    {
        qCritical()<<"vCard reading failed";
        return false;
    }

    qDebug()<<"Reading succeeded, importing...";

    // reading succeeded
    QList<QVersitDocument> inputDocuments = reader.results();

    // importer
    QVersitContactImporter importer;
    result = importer.importDocuments(inputDocuments);

    if(!result)
    {
        qCritical()<<"vCard importing failed";
        return false;
    }

    qDebug()<<"Import operation succeeded";

    // get contacts
    contacts = importer.contacts();
    if(contacts.count()==0)
    {
        qCritical()<<"0 contacts were imported";
        return false;
    }
    return true;
}

/**
  * ExportContactsToVcard function
  */
bool Versit::ExportContactsToVcard(QList<QContact>& contacts)
{
    bool result;

    qDebug()<<"Trying to export: "<<contacts.count()<<" contacts";

    // create exported and try to export
    QVersitContactExporter exporter;
    result = exporter.exportContacts(contacts, QVersitDocument::VCard30Type);
    if(!result)
    {
        qCritical()<<"Contact exporting failed";
        return false;
    }

    qDebug()<<"Writing exporting results...";

    // get output
    QList<QVersitDocument> outputDocuments = exporter.documents();
    // write to buffer
    QBuffer output;
    output.open(QBuffer::ReadWrite);
    QVersitWriter writer;
    writer.setDevice(&output);
    result = writer.startWriting(outputDocuments); // Remember to check the return value
    writer.waitForFinished();

    // if errors
    if(!result || writer.error()!=QVersitWriter::NoError)
    {
        qCritical()<<"Versit writing failed!";
        return false;
    }
    QString resultVcard(output.buffer());

    qDebug()<<"Success, exported contact: "<<resultVcard;
    return true;
}

/**
 * ImportCalendarItemFromIcalendar function
 */
 bool Versit::ImportCalendarItemFromIcalendar(QList<QOrganizerItem>& items, QString iCalFile)
 {
     bool result;

     // see if the file exists
     QFile iCal(iCalFile);
     if(!iCal.exists())
     {
         qCritical()<<"iCalendar file: "<<iCalFile<<" not found";
         return false;
     }

     // open the file
     if(!iCal.open(QFile::ReadOnly))
     {
         qCritical()<<"iCalendar file: "<<iCalFile<<" cannot be opened";
         return false;
     }

     // read the content
     QTextStream stream(&iCal);
     while(!stream.atEnd())
     {
         m_inputIcal.append(stream.readLine());
         // next line
         m_inputIcal.append("\n");
     }
     iCal.close();

     QBuffer input;
     input.open(QBuffer::ReadWrite);
     input.write(m_inputIcal);
     input.seek(0);

     qDebug()<<"Importing iCalendar:\n"<<m_inputIcal;

     // read iCalendar
     QVersitReader reader;
     reader.setDevice(&input);
     result = reader.startReading();
     reader.waitForFinished();
     if(!result || reader.error()!=QVersitReader::NoError)
     {
         qCritical()<<"iCalendar reading failed";
         return false;
     }

     qDebug()<<"Reading succeeded, importing...";

     // reading succeeded
     QList<QVersitDocument> inputDocuments = reader.results();

     // importer
     QVersitOrganizerImporter importer;
     QVersitDocument doc;
     const int count = inputDocuments.count();
     // go through all
     for(int i=0;i<count;++i)
     {
         doc = inputDocuments[i];
         result = importer.importDocument(doc);
         if(!result)
         {
             qCritical()<<"iCalendar importing failed";
             return false;
         }
     }

     qDebug()<<"Import operation succeeded";

     // get items
     items =  importer.items();
     if(items.count()==0)
     {
         qCritical()<<"0 calendar items were imported";
         return false;
     }
    return true;
 }

/**
  * ExportCalendarItemToIcalendar function
  */
 bool Versit::ExportCalendarItemToIcalendar(QList<QOrganizerItem>& items)
 {
     bool result;

     qDebug()<<"Trying to export: "<<items.count()<<" calendar items";

     // create exported and try to export
     QVersitOrganizerExporter exporter;
     result = exporter.exportItems(items, QVersitDocument::ICalendar20Type);
     if(!result)
     {
         qCritical()<<"Calendar item exporting failed";
         return false;
     }

     qDebug()<<"Writing exporting results...";

     // get output
     QVersitDocument outputDocument = exporter.document();
     // write to buffer
     QBuffer output;
     output.open(QBuffer::ReadWrite);
     QVersitWriter writer;
     writer.setDevice(&output);
     result = writer.startWriting(outputDocument); // Remember to check the return value
     writer.waitForFinished();

     // if errors
     if(!result || writer.error()!=QVersitWriter::NoError)
     {
         qCritical()<<"Versit writing failed!";
         return false;
     }
     QString resultIcal(output.buffer());

     qDebug()<<"Success, exported calendar items: "<<resultIcal;
     return true;
 }

//eof

