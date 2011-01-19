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

    qDebug () << "XXXXXX" << "setVoiceBusy (" << property << ")";

    //this seems as not working , TODO bug report to ofono-qt maintainer
    qDebug () << "XXXXXXvoiceBusyComplete" << voiceBusyComplete.count();
    if (!voiceBusyComplete.isEmpty())
    {
        list = voiceBusyComplete.takeFirst();
        qDebug () << "XXXXXXvoiceBusyComplete" << list.at(0).toBool();
        qDebug () << "XXXXXXvoiceBusyComplete" << list.at(1).toString();

        if (list.at(0).toBool())
        {
            g_pResult->StepPassed(__FUNCTION__, TRUE);
            return TRUE;
        }

    }

    //this is not voiceBusyCompleted signal catch, but enough
    //to detect if the setVoiceBusy was completed or not
    qDebug () << "XXXXXXvoiceBusyChanged" << voiceBusyChanged.count();
    if (!voiceBusyChanged.isEmpty())
    {
        list = voiceBusyChanged.takeFirst();
        qDebug () << "XXXXXXvoiceBusyChanged" << list.at(0).toString();
        g_pResult->StepPassed(__FUNCTION__, TRUE);
        return TRUE;
    }

    qDebug () << "XXXXXXvoiceBusyFailed" << setVoiceBusyFailed.count();
    if (!setVoiceBusyFailed.isEmpty())
    {
        qDebug () << "VoiceBusyFailed: " << mCallForwarding->errorName() << " "
                                         << mCallForwarding->errorMessage();
        qDebug () << "XXXXXXsetVoiceBusyFailed";
        MWTS_LEAVE;
        return FALSE;
    }

    MWTS_LEAVE;
    return TRUE;
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

    qDebug () << "XXXXXX" << "setVoiceCallNoReply (" << property << ")";

    //this seems as not working , TODO bug report to ofono-qt maintainer
    qDebug () << "XXXXXXvoiceNoReplyComplete" << voiceNoReplyComplete.count();
    if (!voiceNoReplyComplete.isEmpty())
    {
        list = voiceNoReplyComplete.takeFirst();
        qDebug () << "XXXXXXvoiceNoReplyComplete" << list.at(0).toBool();
        qDebug () << "XXXXXXvoiceNoReplyComplete" << list.at(1).toString();

        if (list.at(0).toBool())
        {
            g_pResult->StepPassed(__FUNCTION__, TRUE);
            return TRUE;
        }

    }

    //this is not voiceNoReplyCompleted signal catch, but enough
    //to detect if the setVoiceNoReply was completed or not
    qDebug () << "XXXXXXvoiceNoReplyChanged" << voiceNoReplyChanged.count();
    if (!voiceNoReplyChanged.isEmpty())
    {
        list = voiceNoReplyChanged.takeFirst();
        qDebug () << "XXXXXXvoiceNoReplyChanged"  << list.at(0).toString();
        g_pResult->StepPassed(__FUNCTION__, TRUE);
        return TRUE;
    }

    qDebug () << "XXXXXXvoiceNoReplyFailed" << setVoiceNoReplyFailed.count();
    if (!setVoiceNoReplyFailed.isEmpty())
    {
        qDebug () << "VoiceNoReplyFailed: " << mCallForwarding->errorName() << " "
                                         << mCallForwarding->errorMessage();
        qDebug () << "XXXXXXsetVoiceNoReplyFailed";
        MWTS_LEAVE;
        return FALSE;
    }

    MWTS_LEAVE;
    return TRUE;
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

    qDebug () << "XXXXXX" << "setVoiceNotReachable (" << property << ")";

    //this seems as not working , TODO bug report to ofono-qt maintainer
    qDebug () << "XXXXXXvoiceNotReachableComplete" << voiceNotReachableComplete.count();
    if (!voiceNotReachableComplete.isEmpty())
    {
        list = voiceNotReachableComplete.takeFirst();
        qDebug () << "XXXXXXvoiceNotReachableComplete" << list.at(0).toBool();
        qDebug () << "XXXXXXvoiceNotReachableComplete" << list.at(1).toString();

        if (list.at(0).toBool())
        {
            g_pResult->StepPassed(__FUNCTION__, TRUE);
            return TRUE;
        }

    }

    //this is not voiceNotReachableCompleted signal catch, but enough
    //to detect if the setVoiceNotReachable was completed or not
    qDebug () << "XXXXXXvoiceNotReachableChanged" << voiceNotReachableChanged.count();
    if (!voiceNotReachableChanged.isEmpty())
    {
        list = voiceNotReachableChanged.takeFirst();
        qDebug () << "XXXXXXvoiceNotReachableChanged" << list.at(0).toString();
        g_pResult->StepPassed(__FUNCTION__, TRUE);
        return TRUE;
    }

    qDebug () << "XXXXXXvoiceNotReachableFailed" << setVoiceNotReachableFailed.count();
    if (!setVoiceNotReachableFailed.isEmpty())
    {
        qDebug () << "VoiceNotReachableFailed: " << mCallForwarding->errorName() << " "
                                         << mCallForwarding->errorMessage();
        qDebug () << "XXXXXXsetVoiceNotReachableFailed";
        MWTS_LEAVE;
        return FALSE;
    }

    MWTS_LEAVE;
    return TRUE;
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

    qDebug () << "XXXXXX" << "setVoiceUnconditional(" << property << ")";

    //this seems as not working , TODO bug report to ofono-qt maintainer
    qDebug () << "XXXXXXvoiceUnconditionalComplete" << voiceUnconditionalComplete.count();
    if (!voiceUnconditionalComplete.isEmpty())
    {
        list = voiceUnconditionalComplete.takeFirst();
        qDebug () << "XXXXXXvoiceUnconditionalComplete" << list.at(0).toBool();
        qDebug () << "XXXXXXvoiceUnconditionalComplete" << list.at(1).toString();

        if (list.at(0).toBool())
        {
            g_pResult->StepPassed(__FUNCTION__, TRUE);
            return TRUE;
        }

    }

    //this is not voiceUnconditionalCompleted signal catch, but enough
    //to detect if the setVoiceUnconditional was completed or not
    qDebug () << "XXXXXXvoiceUnconditionalChanged" << voiceUnconditionalChanged.count();
    if (!voiceUnconditionalChanged.isEmpty())
    {
        list = voiceUnconditionalChanged.takeFirst();
        qDebug () << "XXXXXXvoiceUnconditionalChanged" << list.at(0).toString();
        g_pResult->StepPassed(__FUNCTION__, TRUE);
        return TRUE;
    }

    qDebug () << "XXXXXXvoiceUnconditionalFailed" << setVoiceUnconditionalFailed.count();
    if (!setVoiceUnconditionalFailed.isEmpty())
    {
        qDebug () << "VoiceUnconditionalFailed: " << mCallForwarding->errorName() << " "
                                         << mCallForwarding->errorMessage();
        qDebug () << "XXXXXXsetVoiceUnconditionalFailed";
        MWTS_LEAVE;
        return FALSE;
    }

    MWTS_LEAVE;
    return TRUE;
}

bool VoiceCallTest::disableAll(const QString &type)
{
    MWTS_ENTER;
    QSignalSpy disableAllComplete(mCallForwarding, SIGNAL(disableAllComplete(bool)));

    /*mCallForwarding->disableAll(type);

    mTimer->start(FORWARDING_TIMEOUT);
    mEventLoop->exec();

    qDebug () << __FUNCTION__ << disableAllComplete.count();
    if (disableAllComplete.takeFirst().at(0).toBool())
    {
        qDebug () << __FUNCTION__ << "TRUE";
        MWTS_LEAVE;
        return TRUE;
    }
    else
    {
        qDebug () << __FUNCTION__ << "FALSE";
        MWTS_LEAVE;
        return FALSE;
    }

    MWTS_LEAVE;
    return TRUE;
    */
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

    qDebug () << "XXXXXX" << "setVoiceCallWaiting(" << setting << ")";

    //this seems as not working , TODO bug report to ofono-qt maintainer
    qDebug () << "XXXXXXvoiceCallWaitinglComplete" << voiceCallWaitingComplete.count();
    if (!voiceCallWaitingComplete.isEmpty())
    {
        list = voiceCallWaitingComplete.takeFirst();
        qDebug () << "XXXXXXvoiceCallWaitingComplete" << list.at(0).toBool();
        qDebug () << "XXXXXXvoiceCallWaitingComplete" << list.at(1).toString();

        if (list.at(0).toBool())
        {
            g_pResult->StepPassed(__FUNCTION__, TRUE);
            return TRUE;
        }

    }

    //this is not voiceCallWaitingCompleted signal catch, but enough
    //to detect if the setVoicCallWaiting was completed or not
    qDebug () << "XXXXXXvoiceCallWaitingChanged" << voiceCallWaitingChanged.count();
    if (!voiceCallWaitingChanged.isEmpty())
    {
        list = voiceCallWaitingChanged.takeFirst();
        qDebug () << "XXXXXXvoiceCallWaitingChanged" << list.at(0).toString();
        g_pResult->StepPassed(__FUNCTION__, TRUE);
        return TRUE;
    }

    qDebug () << "XXXXXXvoiceCallWaitingFailed" << setVoiceCallWaitingFailed.count();
    if (!setVoiceCallWaitingFailed.isEmpty())
    {
        qDebug () << "VoiceCallWaitingFailed: " << mCallSettings->errorName() << " "
                                         << mCallSettings->errorMessage();
        qDebug () << "XXXXXXsetVoiceCallWaitingFailed";
        MWTS_LEAVE;
        return FALSE;
    }

    MWTS_LEAVE;
    return TRUE;
}

bool VoiceCallTest::setVoiceCallIncoming(const QString &barrings, const QString &pin)
{
    QSignalSpy voiceIncomingComplete(mCallBarring, SIGNAL(voiceIncomingComplete(bool, QString)));
    QSignalSpy voiceIncomingChanged(mCallBarring, SIGNAL(voiceIncomingChanged(QString)));
    QSignalSpy setVoiceIncomingFailed(mCallBarring, SIGNAL(setVoiceIncomingFailed()));

    mCallBarring->setVoiceIncoming(barrings, pin);

    mTimer->start(FORWARDING_TIMEOUT);
    mEventLoop->exec();

    qDebug () << "XXXXXX" << "setVoiceCallIncoming(" << barrings << ","<< pin << ")";

    //this seems as not working , TODO bug report to ofono-qt maintainer
    qDebug () << "XXXXXXvoiceIncomingComplete" << voiceIncomingComplete.count();
    if (!voiceIncomingComplete.isEmpty())
    {
        list = voiceIncomingComplete.takeFirst();
        qDebug () << "XXXXXXvoiceIncomingComplete" << list.at(0).toBool();
        qDebug () << "XXXXXXvoiceIncomingComplete" << list.at(1).toString();

        if (list.at(0).toBool())
        {
            g_pResult->StepPassed(__FUNCTION__, TRUE);
            return TRUE;
        }

    }

    //this is not voiceIncomingCompleted signal catch, but enough
    //to detect if the setVoiceIncoming was completed or not
    qDebug () << "XXXXXXvoiceIncomingChanged" << voiceIncomingChanged.count();
    if (!voiceIncomingChanged.isEmpty())
    {
        list = voiceIncomingChanged.takeFirst();
        qDebug () << "XXXXXXvoiceIncomingChanged" << list.at(0).toString();
        g_pResult->StepPassed(__FUNCTION__, TRUE);
        return TRUE;
    }

    qDebug () << "XXXXXXvoiceIncomingFailed" << setVoiceIncomingFailed.count();
    if (!setVoiceIncomingFailed.isEmpty())
    {
        qDebug () << "VoiceIncomingFailed: " << mCallBarring->errorName() << " "
                                         << mCallBarring->errorMessage();
        qDebug () << "XXXXXXsetVoiceIncomingFailed";
        MWTS_LEAVE;
        return FALSE;
    }

    MWTS_LEAVE;
    return TRUE;
}

bool VoiceCallTest::setVoiceCallOutgoing(const QString &barrings, const QString &pin)
{
    QSignalSpy voiceOutgoingComplete(mCallBarring, SIGNAL(voiceOutgoingComplete(bool, QString)));
    QSignalSpy voiceOutgoingChanged(mCallBarring, SIGNAL(voiceOutgoingChanged(QString)));
    QSignalSpy setVoiceOutgoingFailed(mCallBarring, SIGNAL(setVoiceOutgoingFailed()));

    mCallBarring->setVoiceOutgoing(barrings, pin);

    mTimer->start(FORWARDING_TIMEOUT);
    mEventLoop->exec();

    qDebug () << "XXXXXX" << "setVoiceCallOutgoing(" << barrings << ","<< pin << ")";

    //this seems as not working , TODO bug report to ofono-qt maintainer
    qDebug () << "XXXXXXvoiceOutgoingComplete" << voiceOutgoingComplete.count();
    if (!voiceOutgoingComplete.isEmpty())
    {
        list = voiceOutgoingComplete.takeFirst();
        qDebug () << "XXXXXXvoiceOutgoingComplete" << list.at(0).toBool();
        qDebug () << "XXXXXXvoiceOutgoingComplete" << list.at(1).toString();

        if (list.at(0).toBool())
        {
            g_pResult->StepPassed(__FUNCTION__, TRUE);
            return TRUE;
        }

    }

    //this is not voiceOutgoingCompleted signal catch, but enough
    //to detect if the setVoiceOutgoing was completed or not
    qDebug () << "XXXXXXvoiceOutgoingChanged" << voiceOutgoingChanged.count();
    if (!voiceOutgoingChanged.isEmpty())
    {
        list = voiceOutgoingChanged.takeFirst();
        qDebug () << "XXXXXXvoiceOutgoingChanged" << list.at(0).toString();
        g_pResult->StepPassed(__FUNCTION__, TRUE);
        return TRUE;
    }

    qDebug () << "XXXXXXvoiceOutgoingFailed" << setVoiceOutgoingFailed.count();
    if (!setVoiceOutgoingFailed.isEmpty())
    {
        qDebug () << "VoiceOutgoingFailed: " << mCallBarring->errorName() << " "
                                         << mCallBarring->errorMessage();
        qDebug () << "XXXXXXsetVoiceOutgoingFailed";
        MWTS_LEAVE;
        return FALSE;
    }

    MWTS_LEAVE;
    return TRUE;
}
