#include "voicecalltest.h"

VoiceCallTest::VoiceCallTest(QObject *parent)
    :OfonoTestInterface(parent)
{
    MWTS_ENTER;
    MWTS_LEAVE;
}

VoiceCallTest::~VoiceCallTest()
{
    MWTS_ENTER;
    MWTS_LEAVE;
}

void VoiceCallTest::OnInitialize()
{
    MWTS_ENTER;

    QString modem = g_pConfig->value("DEFAULT/modem").toString();

    mCallForwarding = new OfonoCallForwarding(OfonoModem::AutomaticSelect, modem, this);
    mCallSettings = new OfonoCallSettings(OfonoModem::AutomaticSelect, modem, this);
    mCallBarring = new OfonoCallBarring(OfonoModem::AutomaticSelect, modem, this);

    qDebug () << __FUNCTION__ << "selected modem: " << mCallForwarding->modem()->model();

    if (!mCallForwarding->modem()->powered())
    {
        mCallForwarding->modem()->setPowered(true);

        mTimer->start(MODEM_TIMEOUT);
        mEventLoop->exec();
    }

    if (!mCallForwarding->modem()->online())
    {
        mCallForwarding->modem()->setOnline(true);

        mTimer->start(MODEM_TIMEOUT);
        mEventLoop->exec();
    }

    if (mCallForwarding->isValid())
    {        
        g_pResult->StepPassed("modem is valid: ", TRUE);
    }
    else
    {
        qCritical ("modem is not valid");
        g_pResult->StepPassed(__FUNCTION__, FALSE);
        MWTS_LEAVE;
        return;
    }

    g_pResult->StepPassed("modem is powered: ", mCallForwarding->modem()->powered());
    g_pResult->StepPassed("modem is online: ", mCallForwarding->modem()->online());

    MWTS_LEAVE;
}

void VoiceCallTest::OnUninitialize()
{
    MWTS_ENTER;

    OfonoTestInterface::OnUninitialize();
    if (mCallForwarding)
    {
        delete mCallForwarding;
        mCallForwarding = NULL;
    }

    if (mCallSettings)
    {
        delete mCallSettings;
        mCallSettings = NULL;
    }

    if (mCallBarring)
    {
        delete mCallBarring;
        mCallBarring = NULL;
    }

    MWTS_LEAVE;
}

bool VoiceCallTest::setVoiceCallBusy(const QString &property)
{
    MWTS_ENTER;    
    QSignalSpy voiceBusyComplete(mCallForwarding, SIGNAL(voiceBusyComplete(bool, QString)));
    QSignalSpy voiceBusyChanged(mCallForwarding, SIGNAL(voiceBusyChanged(QString)));
    QSignalSpy setVoiceBusyFailed(mCallForwarding, SIGNAL(setVoiceBusyFailed()));

    mCallForwarding->setVoiceBusy(property);

    mTimer->start(FORWARDING_TIMEOUT);
    mEventLoop->exec();

    qDebug () << __FUNCTION__ << property;
    return this->onSpySignals(voiceBusyComplete, voiceBusyChanged, setVoiceBusyFailed, "Busy");
}

bool VoiceCallTest::setVoiceCallNoReply(const QString &property)
{
    MWTS_ENTER;
    QSignalSpy voiceNoReplyComplete(mCallForwarding, SIGNAL(voiceNoReplyComplete(bool, QString)));
    QSignalSpy voiceNoReplyChanged(mCallForwarding, SIGNAL(voiceNoReplyChanged(QString)));
    QSignalSpy setVoiceNoReplyFailed(mCallForwarding, SIGNAL(setVoiceNoReplyFailed()));

    mCallForwarding->setVoiceNoReply(property);

    mTimer->start(FORWARDING_TIMEOUT);
    mEventLoop->exec();

    qDebug () << __FUNCTION__ << property;
    return this->onSpySignals(voiceNoReplyComplete, voiceNoReplyChanged, setVoiceNoReplyFailed, "NoReply");
}

bool VoiceCallTest::setVoiceCallNotReachable(const QString &property)
{
    MWTS_ENTER;
    QSignalSpy voiceNotReachableComplete(mCallForwarding, SIGNAL(voiceNotReachableComplete(bool, QString)));
    QSignalSpy voiceNotReachableChanged(mCallForwarding, SIGNAL(voiceNotReachableChanged(QString)));
    QSignalSpy setVoiceNotReachableFailed(mCallForwarding, SIGNAL(setVoiceNotReachableFailed()));

    mCallForwarding->setVoiceNotReachable(property);

    mTimer->start(FORWARDING_TIMEOUT);
    mEventLoop->exec();

    qDebug () << __FUNCTION__ <<  property;
    return this->onSpySignals(voiceNotReachableComplete, voiceNotReachableChanged, setVoiceNotReachableFailed, "NotReachable");
}

bool VoiceCallTest::setVoiceCallUnconditional(const QString &property)
{
    MWTS_ENTER;
    QSignalSpy voiceUnconditionalComplete(mCallForwarding, SIGNAL(voiceUnconditionalComplete(bool, QString)));
    QSignalSpy voiceUnconditionalChanged(mCallForwarding, SIGNAL(voiceUnconditionalChanged(QString)));
    QSignalSpy setVoiceUnconditionalFailed(mCallForwarding, SIGNAL(setVoiceUnconditionalFailed()));

    mCallForwarding->setVoiceUnconditional(property);

    mTimer->start(FORWARDING_TIMEOUT);
    mEventLoop->exec();

    qDebug () << __FUNCTION__ << property;
    return this->onSpySignals(voiceUnconditionalComplete, voiceUnconditionalChanged, setVoiceUnconditionalFailed, "Unconditional");
}

bool VoiceCallTest::setVoiceCallWaiting (const QString &setting)
{
    MWTS_ENTER;
    QSignalSpy voiceCallWaitingComplete(mCallSettings, SIGNAL(voiceCallWaitingComplete(bool, QString)));
    QSignalSpy voiceCallWaitingChanged(mCallSettings, SIGNAL(voiceCallWaitingChanged(QString)));
    QSignalSpy setVoiceCallWaitingFailed(mCallSettings, SIGNAL(setVoiceCallWaitingFailed()));

    mCallSettings->setVoiceCallWaiting(setting);

    mTimer->start(FORWARDING_TIMEOUT);
    mEventLoop->exec();

    qDebug () << __FUNCTION__ << setting;
    return this->onSpySignals(voiceCallWaitingComplete, voiceCallWaitingChanged, setVoiceCallWaitingFailed, "CallWaiting");
}

bool VoiceCallTest::setVoiceCallIncoming(const QString &barrings, const QString &pin)
{
    QSignalSpy voiceIncomingComplete(mCallBarring, SIGNAL(voiceIncomingComplete(bool, QString)));
    QSignalSpy voiceIncomingChanged(mCallBarring, SIGNAL(voiceIncomingChanged(QString)));
    QSignalSpy setVoiceIncomingFailed(mCallBarring, SIGNAL(setVoiceIncomingFailed()));

    mCallBarring->setVoiceIncoming(barrings, pin);

    mTimer->start(FORWARDING_TIMEOUT);
    mEventLoop->exec();

    qDebug () << __FUNCTION__ << barrings << ","<< pin;
    return this->onSpySignals(voiceIncomingComplete, voiceIncomingChanged, setVoiceIncomingFailed, "Incoming");
}

bool VoiceCallTest::setVoiceCallOutgoing(const QString &barrings, const QString &pin)
{
    QSignalSpy voiceOutgoingComplete(mCallBarring, SIGNAL(voiceOutgoingComplete(bool, QString)));
    QSignalSpy voiceOutgoingChanged(mCallBarring, SIGNAL(voiceOutgoingChanged(QString)));
    QSignalSpy setVoiceOutgoingFailed(mCallBarring, SIGNAL(setVoiceOutgoingFailed()));

    mCallBarring->setVoiceOutgoing(barrings, pin);

    mTimer->start(FORWARDING_TIMEOUT);
    mEventLoop->exec();

    qDebug () << __FUNCTION__ << barrings << "," << pin;
    return this->onSpySignals(voiceOutgoingComplete, voiceOutgoingChanged, setVoiceOutgoingFailed, "Outgoing");
}

bool VoiceCallTest::onSpySignals (QSignalSpy &complete, QSignalSpy &changed, QSignalSpy &failed, const QString &name)
{   
    qDebug () << "voice" << name << "Complete:" << complete.count();
    if (!complete.isEmpty())
    {
        list = complete.takeFirst();
        qDebug () << "voice" << name << "Complete:" <<  list.at(0).toBool();
        qDebug () << "voice" << name << "Complete:" <<  list.at(1).toString();

        if (list.at(0).toBool())
        {
            MWTS_LEAVE;
            return TRUE;
        }
    }

    qDebug () << "voice" << name << "Changed:" << changed.count();
    if (!changed.isEmpty())
    {
        list = changed.takeFirst();
        qDebug () << "voice" << name << "Changed:" << list.at(0).toString();

        MWTS_LEAVE;
        return TRUE;
    }

    qDebug () << "voice" << name << "Failed" << failed.count();
    if (!failed.isEmpty())
    {
        qDebug () << "voice" << name << "Failed";

        MWTS_LEAVE;
        return FALSE;
    }

    MWTS_LEAVE;
    return TRUE;
}
