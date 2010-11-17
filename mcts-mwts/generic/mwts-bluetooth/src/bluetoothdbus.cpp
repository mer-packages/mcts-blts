/* bluetoothdbus.cpp
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

#include "bluetoothdbus.h"
#include "dbus-headers/agent.h"

#define DBUS_NAME "org.bluez"
#define DBUS_PATH "/"
#define DBUS_INTERFACE_MANAGER "org.bluez.Manager"
#define DBUS_INTERFACE_ADAPTER "org.bluez.Adapter"
#define DBUS_INTERFACE_DEVICE "org.bluez.Device"
#define DBUS_INTERFACE_AGENT "org.bluez.Agent"
#define DBUS_INTERFACE_AUDIO "org.bluez.Audio"
#define DBUS_APP_NAME "MwtsBluetooth"
#define DBUS_AGENT_CAPABILITY "DisplayYesNo"

#define DISCOVERY_TIMEOUT 120000    //milliseconds


BluetoothDbus::BluetoothDbus(QObject* parent):
        BluetoothApiInterface(parent),
        m_pManager(0),
        m_pAdapter(0),
        m_pDevice(0),
        m_pAgent(0),
        m_pAudio(0),
        m_pEventLoop(0)
{
    MWTS_ENTER;
    MWTS_LEAVE;
}

BluetoothDbus::~BluetoothDbus()
{
    MWTS_ENTER;
    delete m_pEventLoop;
    m_pEventLoop = 0;
    delete m_pAgent;
    m_pAgent = 0;
    delete m_pAudio;
    m_pAudio = 0;
    delete m_pDevice;
    m_pDevice = 0;
    delete m_pAdapter;
    m_pAdapter = 0;
    qDebug() << "delete adapter";
    delete m_pManager;
    m_pManager = 0;
    qDebug() << "delete manager";
    MWTS_LEAVE;
}

/**
*   @brief      Connects us to dbus.
*   @return     true if succeeded, otherwise false
*/
bool BluetoothDbus::Init()
{
    MWTS_ENTER;
    bool bret = true;
    QString adapter_path;

    /* Create Eventloop */
    m_pEventLoop = new QEventLoop(this);

    /* Create Manager */
    if(bret) m_pManager = create_interface(DBUS_INTERFACE_MANAGER,DBUS_PATH);

    /* Get Default Adapter Path */
    if(bret)
    {
        QDBusReply<QDBusObjectPath> reply = m_pManager->call(QLatin1String("DefaultAdapter"));
        bret = reply.isValid();
        qDebug() << "Has a Default Adapter Path == " << bret;
        if(bret) adapter_path = reply.value().path();
        if(!bret)
        {
            qCritical() << "No adapter default path availlable";
            QDBusError err = reply.error();
            if(err.isValid())
                qDebug() << "QDBusError: " << err.message();
        }
    }

    /* Create Adapter */
    if(bret) m_pAdapter = create_interface(DBUS_INTERFACE_ADAPTER,adapter_path);
    if(bret) connect(m_pAdapter,
                     SIGNAL(PropertyChanged(QString,QDBusVariant)),
                     this,
                     SLOT(PropertyChanged(QString,QDBusVariant)));
    if(bret) connect(m_pAdapter,
                     SIGNAL(DeviceFound(QString,QVariantMap)),
                     this,
                     SLOT(DeviceFound(QString,QVariantMap)));

    MWTS_LEAVE;
    return bret;
}

/**
*   @brief      Disconnects us from dbus.
*   @return     true if succeeded, otherwise false
*/
bool BluetoothDbus::Uninit()
{
    MWTS_ENTER;
    bool bret = true;

    delete m_pEventLoop;
    m_pEventLoop = 0;
    qDebug() << "EventLoop deleted";

    delete m_pAgent;
    m_pAgent = 0;
    qDebug() << "Agent deleted";

    QDBusConnection::disconnectFromBus(DBUS_APP_NAME);
    qDebug() << "Disconnected from Dbus";

    delete m_pAudio;
    m_pAudio = 0;
    qDebug() << "Audio deleted";

    delete m_pDevice;
    m_pDevice = 0;
    qDebug() << "Device deleted";

    delete m_pAdapter;
    m_pAdapter = 0;
    qDebug() << "Adapter deleted";

    delete m_pManager;
    m_pManager = 0;
    qDebug() << "Manager deleted";

    MWTS_LEAVE;
    return bret;
}

/**
*   @brief      Connect (pairs) two devices.
*   @return     true if succeeded, otherwise false
*/
bool BluetoothDbus::Connect()
{
    MWTS_ENTER;
    bool bret = true;
    QString device_path;

    /* Create Agent */
    m_pAgent = new Agent(this,m_sPin.toInt());

    /* Double check agents existence */
    qDebug() << "Connection connected == "<< QDBusConnection::systemBus().isConnected();
    QObject* obj = QDBusConnection::systemBus().objectRegisteredAt(m_pAgent->path());
    bret = ((obj)?(true):(false));
    qDebug() << "Agent object found === "<< bret;

    /* If server, register to adapter */
//    if(bret)	//activate this if you are creating a server/client implementation
//    {
//        qDebug() << "Register agent";
//        m_pAdapter->call(QLatin1String("RegisterAgent"),
//                         qVariantFromValue(QDBusObjectPath(m_pAgent->path())),
//                         QVariant(DBUS_AGENT_CAPABILITY));
//        qDebug() << m_pAdapter->lastError().message();
//    }

    /* Pair Devices */
    if(bret) device_path = pair_devices();
    bret = !device_path.isEmpty();

    /* Get proxy for device */
    if(bret)
    {
        qDebug() << "Create device interface of path: " << device_path;
        m_pDevice = create_interface(DBUS_INTERFACE_DEVICE,device_path);

        /* Check if paired */
        QDBusReply<QVariantMap> listreply = m_pDevice->call(QLatin1String("GetProperties"));
        QVariant var = listreply.value().value(QString("Paired"));
        bret = var.isValid();
        if(bret && var.canConvert<bool>())
        {
            bret = var.toBool();           // is the device paired?
            qDebug() << "Device is paired == " << bret;
        }
        else
            qWarning() << "Got invalid *Paired* property value";
    }

    /* Route audio to headset */
    if(bret && m_bIsHeadset)
    {
        qDebug() << "Create audio interface of path: " << device_path;
        m_pAudio = create_interface(DBUS_INTERFACE_AUDIO,device_path);

        /* Connect audio */
        qDebug() << "Connecting audio";
        m_pAudio->call(QLatin1String("Connect"));
    }

    MWTS_LEAVE;
    return bret;
}

/**
*   @brief      Disconnect a connection between two devices.
*   @return     true if succeeded, otherwise false
*/
bool BluetoothDbus::Disconnect()
{
    MWTS_ENTER;
    bool bret = true;

    /* check if has device */
    QString path = has_device();
    if(path.isEmpty())
        return true;

    /* create device if not exists */
    if(!m_pDevice)
        m_pDevice = create_interface(DBUS_INTERFACE_DEVICE,path);

    /* Kill audio routing */
    if(m_bIsHeadset || m_pAudio)
    {
        /* Create audio interface if necessary */
        if(!m_pAudio)
        {
            qDebug() << "Create audio interface of path: " << path;
            m_pAudio = create_interface(DBUS_INTERFACE_AUDIO,path);
        }

        /* Disconnect audio */
        qDebug() << "Disconnecting audio";
        m_pAudio->call(QLatin1String("Disconnect"));
    }

    /* disconnect */
    m_pDevice->call(QLatin1String("Disconnect"));

    /* check if still connected */
    bret = !is_connected();

    if(!bret) qCritical() << "Error while disconnecting the device";
    else qDebug() << "Device disconnected succesfully";

    MWTS_LEAVE;
    return bret;
}

/**
*   @brief      Removes a device from the device list.
*   @return     true if succeeded, otherwise false
*/
bool BluetoothDbus::Remove()
{
    MWTS_ENTER;
    bool bret = false;

    //quick out
    if(!m_pAdapter)
        return false;

    /* check if exist */
    QString path = has_device();

    /* remove device */
    if(!path.isEmpty())
    {
        m_pAdapter->call(QLatin1String("RemoveDevice"),
                         qVariantFromValue(QDBusObjectPath(path)));
    }

    /* check if still exists */
    bret = has_device().isEmpty();
    if(!bret) qCritical() << "Error while removing the device";
    else qDebug() << "Device removed succesfully";

    /* free resources */
    if(m_pDevice)
    {
        delete m_pDevice;
        m_pDevice = 0;
    }

    MWTS_LEAVE;
    return bret;
}

/**
*   @brief  Get the device mac address
*   @return const QString&,     the mac address
*/
QString& BluetoothDbus::GetAddress()
{
    MWTS_ENTER;

    /* Get address */
    QDBusReply<QVariantMap> listreply = m_pAdapter->call(QLatin1String("GetProperties"));
    QVariant var = listreply.value().value(QString("Address"));
    if(var.canConvert<QString>())
    {
        m_sAddress = var.toString();
        qDebug() << "Address found";
    }
    else
        qWarning() << "Got invalid *Address* property value";

    MWTS_LEAVE;
    return m_sAddress;
}

/**
 * Set BT device's scan mode
 * @param mode  Scan mode
 * @return      true if succeeded, otherwise false
 */
bool BluetoothDbus::SetScanMode(ScanMode scanmode)
{
    MWTS_ENTER;
    bool bret = true;
    bool mode = true;

    if(scanmode == ScanModeInvisible)
        mode = false;

    qDebug() << "Set visibility on =="<<mode;

    /* Set Discoverable */
    m_pAdapter->call(QLatin1String("SetProperty"),
                     qVariantFromValue(QString("Discoverable")),
                     qVariantFromValue(QDBusVariant(mode)));

    qDebug() << m_pAdapter->lastError().message();

    /* Check Discoverable */
    QDBusReply<QVariantMap> listreply = m_pAdapter->call(QLatin1String("GetProperties"));
    QVariant var = listreply.value().value(QString("Discoverable"));
    if(var.canConvert<bool>())
    {
        bret = var.toBool();
        qDebug() << "Discoverable found";
    }
    else
        qWarning() << "Got invalid *Discoverable* property value";

    if(bret == mode) bret = true;       //Discoverable changed
    else bret = false;                  //Discoverable did not change

    qDebug() << "Discoverable changed ==" << bret;


    MWTS_LEAVE;
    return bret;
}

/**
 * Set BT device's power mode
 * @param mode  Power mode, on = true, off = false
 * @return      true if succeeded, otherwise false
 */
bool BluetoothDbus::SetPowerMode(bool isOn)
{
    MWTS_ENTER;
    bool bret = true;

    qDebug() << "Set power =="<<isOn;

    /* Set Power */
    m_pAdapter->call(QLatin1String("SetProperty"),
                     qVariantFromValue(QString("Powered")),
                     qVariantFromValue(QDBusVariant(isOn)));

    qDebug() << m_pAdapter->lastError().message();

    /* Check Power */
    QDBusReply<QVariantMap> listreply = m_pAdapter->call(QLatin1String("GetProperties"));
    QVariant var = listreply.value().value(QString("Powered"));
    if(var.canConvert<bool>())
    {
        bret = var.toBool();
        qDebug() << "Powered found";
    }
    else
        qWarning() << "Got invalid *Powered* property value";

    if(bret == isOn) bret = true;       //power changed
    else bret = false;                  //power did not change

    qDebug() << "Power changed ==" << bret;

    MWTS_LEAVE;
    return bret;
}

/**
 * Make BT device scan and resolve names
 * @return  true if succeeded, otherwise false
 */
bool BluetoothDbus::Scan()
{
    MWTS_ENTER;
    bool bret = true;

    /* Check that has everything */
    if(!m_pAdapter)
    {
        bret = false;
        qCritical() << "Adapter not initialized";
    }

    /* Init Timeout */
    QTimer timer(this);
    connect(&timer,SIGNAL(timeout()),this,SLOT(Timeout()));
    timer.start(DISCOVERY_TIMEOUT);

    /* Start Discovery */
    m_pAdapter->call(QLatin1String("StartDiscovery"));
    qDebug() << "Scanning...";

    /* Main Loop */
    int ret = m_pEventLoop->exec();

    /* Stop Discovery */
    m_pAdapter->call(QLatin1String("StopDiscovery"));
    qDebug() << "Scanning stopped";

    disconnect(&timer,SIGNAL(timeout()),this,SLOT(Timeout()));

    /* Check values */
    if(ret < 0)
    {
        bret = false;
        qCritical() << "Timeout occurred while discovering";
    }
    else
        qDebug() << "Discovery was succesful";

    MWTS_LEAVE;
    return bret;
}

/**
*   @brief      Pair Devices
*   @return     QString     device path
*/
QString BluetoothDbus::pair_devices()
{
    MWTS_ENTER;
    QString path;

    /* pair */
    qDebug() << "create arguments";
    QList<QVariant> argumentList;
    argumentList << qVariantFromValue(QString(m_sHost))
                 << qVariantFromValue(QDBusObjectPath(m_pAgent->path()))
                 << qVariantFromValue(QString(DBUS_AGENT_CAPABILITY));
    qDebug() << "Host:" << m_sHost;
    qDebug() << "Agent Path:" << m_pAgent->path();
    qDebug() << "Capability:" << DBUS_AGENT_CAPABILITY;

    qDebug() << "Start pairing";
    QDBusPendingReply<QDBusObjectPath> reply = m_pAdapter->callWithArgumentList(QDBus::BlockWithGui,
                                                                                QLatin1String("CreatePairedDevice"),
                                                                                argumentList);

    /* wait */
    if(!reply.isFinished())
        reply.waitForFinished();

    if(!reply.isValid())
    {
        qWarning() << "Invalid reply: " << reply.error().message();

        /* Try if it exists */
        path = has_device();
    }
    else
        path = reply.value().path();

    if(path.isEmpty())
        qWarning() << "Error while creating a device";

    MWTS_LEAVE;
    return path;
}

/**
*   @brief      Checks whether we have a device with specified mac
*   @return     QString     object path of device
*/
QString BluetoothDbus::has_device()
{
    MWTS_ENTER;
    QString sret;

    if(m_sHost.isEmpty())
        return sret;

    /* check if exist */
    QDBusReply<QDBusObjectPath> reply = m_pAdapter->call(QLatin1String("FindDevice"),QVariant(m_sHost));
    qDebug() << "Device found == " << reply.isValid();

    if(reply.isValid())
        sret = reply.value().path();

    MWTS_LEAVE;
    return sret;
}

/**
*   @brief      Creates a DBus Interface and returns instance of it.
*   @param      QString         type of the interface.
*   @param      QString         path of the object.
*   @return     DBusInterface   dbus interface instance.
*/
QDBusInterface* BluetoothDbus::create_interface(const QString& type, const QString& path)
{
    MWTS_ENTER;
    QDBusInterface* pret = 0;

    qDebug() << "Create dbus interface: " << type ;
    pret = new QDBusInterface(DBUS_NAME,path,type,QDBusConnection::systemBus());
    qDebug() << "Has a dbus Interface == " << pret->isValid() << " of type " << type;
    if(!pret->isValid()) qCritical() << "No dbus Interface "<< type << " availlable";

    MWTS_LEAVE;
    return pret;
}

/**
*   @brief      Tells whether you are connected to the current device.
*               Returns false if m_pDevice doesn't exist.
*   @return     bool    true if is connected, false if not.
*/
bool BluetoothDbus::is_connected()
{
    MWTS_ENTER;
    bool bret = true;

    if(!m_pDevice)
    {
        qWarning() << "No device pointer. Cannot check if connected.";
        MWTS_LEAVE;
        return false;
    }

    QDBusReply<QVariantMap> reply = m_pDevice->call(QLatin1String("GetProperties"));
    QVariant var = reply.value().value(QString("Connected"));
    bret = var.isValid();
    if(bret && var.canConvert<bool>())
        bret = var.toBool();           // is the device connected?
    else
        qWarning() << "Got invalid *Connected* property value";

    MWTS_LEAVE;
    return bret;
}

/**
*   @brief  Logs all property changed events
*   @param  QString name,   name of the property
*   @param  QDBusVariant value, value of the property
*/
void BluetoothDbus::PropertyChanged(QString name,QDBusVariant value)
{
    MWTS_ENTER;
    qDebug() << name<<"property changed";
    if(value.variant().canConvert<QString>())
        qDebug() << "with value: "<<value.variant().toString();
    if(name == "Discovering" && value.variant().value<bool>() == false)
    {
        qDebug() << "Stop eventloop";
        m_pEventLoop->exit();
    }
    MWTS_LEAVE;
}

/**
*   @brief  This slot is called when inquiry founds a new device
*   @param  QString address, mac address of the found device
*   @param  QVariantMap values, device values.
*/
void BluetoothDbus::DeviceFound(QString address, QVariantMap values)
{
    MWTS_ENTER;
    QString name;
    if(values.value(QString("Name")).canConvert<QString>())
        name = values.value(QString("Name")).toString();
    qDebug() << "Found device:"<< address <<"\t\t"<<name;
    g_pResult->Write("Found device: "+ address +"\t\t"+name);
    MWTS_LEAVE;
}

/**
*   @brief  Takes in incoming timeout signals and processes them
*/
void BluetoothDbus::Timeout()
{
    MWTS_ENTER;
    qDebug() << "Timeout occurred";
    if(m_pEventLoop->isRunning())
    {
        qDebug() << "Stopping eventloop";
        m_pEventLoop->exit(-1);                 // -1 signals an error
    }
    MWTS_LEAVE;
}
