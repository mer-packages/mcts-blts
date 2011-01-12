/*
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

#include "stable.h"
#include "interface.h"
#include "ofonotest.h"

#include <MwtsCommon>

// MIN's module information
const char *module_date = __DATE__;
const char *module_time = __TIME__;

// ofono-qt test class
OfonoTest test;

/**
 * ChangePin
 * Changes the pin given by string type.
 * Usage: ChangePin [pinType, oldPin, newPin]
 * @param pinType, oldPin, newPin
 * @return ENOERR
 */
LOCAL int ChangePin(MinItemParser * item)
{
    MWTS_ENTER;

    char *pinType = NULL;
    char *oldPin = NULL;
    char *newPin = NULL;
    bool result = false;

    if (ENOERR != mip_get_next_string( item, &pinType ))
    {
        qCritical() << "could not parse pin type";
        return EINVAL;
    }

    if (ENOERR != mip_get_next_string( item, &oldPin ))
    {
        qCritical() << "could not parse old pin code";
        return EINVAL;
    }

    if (ENOERR != mip_get_next_string( item, &newPin ))
    {
        qCritical() << "could not parse new pin code";
        return EINVAL;
    }

    result = test.changePin(pinType, oldPin, newPin);
    g_pResult->StepPassed( __PRETTY_FUNCTION__, result);

    free(pinType);
    free(oldPin);
    free(newPin);

    return ENOERR;
}

//TODO resetPin, it does not work. It might be a bug report to ofono
LOCAL int ResetPin(MinItemParser * item)
{
    MWTS_ENTER;

    char *pinType = NULL;
    char *puk = NULL;
    char *newPin = NULL;
    bool result = false;

    if (ENOERR != mip_get_next_string( item, &pinType ))
    {
        qCritical() << "could not parse pin type";
        return EINVAL;
    }

    if (ENOERR != mip_get_next_string( item, &puk ))
    {
        qCritical() << "could not parse old puk code";
        return EINVAL;
    }

    if (ENOERR != mip_get_next_string( item, &newPin ))
    {
        qCritical() << "could not parse new pin code";
        return EINVAL;
    }

    result = test.resetPin(pinType, puk, newPin);
    g_pResult->StepPassed( __PRETTY_FUNCTION__, result);

    free(pinType);
    free(puk);
    free(newPin);

    return ENOERR;
}


/**
 * EnablePin
 * Activates the lock for the particular pin type. The
 * device will ask for a PIN automatically next time the
 * device is turned on or the SIM is removed and
 * re-inserter. The current PIN is required for the
 * operation to succeed.
 * Usage: EnablePin [pinType, pin]
 * @param pinType, pin
 * @return ENOERR
 */
LOCAL int EnablePin(MinItemParser * item)
{
    MWTS_ENTER;

    char *pinType = NULL;
    char *pin = NULL;
    bool result = false;

    if (ENOERR != mip_get_next_string( item, &pinType ))
    {
        qCritical() << "could not parse pin type";
        return EINVAL;
    }

    if (ENOERR != mip_get_next_string( item, &pin ))
    {
        qCritical() << "could not parse current pin code";
        return EINVAL;
    }

    result = test.enablePin(pinType, pin);
    g_pResult->StepPassed( __PRETTY_FUNCTION__, result);

    free(pinType);
    free(pin);

    return ENOERR;
}

/**
 * DisablePin
 * Deactivates the lock for the particular pin type. The
 * current PIN is required for the operation to succeed.
 * Usage: ChangePin [pinType, pin]
 * @param pinType, pin
 * @return ENOERR
 */
LOCAL int DisablePin(MinItemParser * item)
{
    MWTS_ENTER;

    char *pinType = NULL;
    char *pin = NULL;
    bool result = false;

    if (ENOERR != mip_get_next_string( item, &pinType ))
    {
        qCritical() << "could not parse pin type";
        return EINVAL;
    }

    if (ENOERR != mip_get_next_string( item, &pin ))
    {
        qCritical() << "could not parse current pin code";
        return EINVAL;
    }

    result = test.disablePin(pinType, pin);
    g_pResult->StepPassed( __PRETTY_FUNCTION__, result);

    free(pinType);
    free(pin);

    return ENOERR;
}

/**
 * VerifyPin
 * Verifies the invalid pin by enabling or disabling the pin
 * Usage: VerifyPin [pinType, pin]
 * @param pinType, pin
 * @return ENOERR
 */
LOCAL int VerifyPin(MinItemParser * item)
{
    MWTS_ENTER;

    char *pinType = NULL;
    char *pin = NULL;
    bool result = false;

    if (ENOERR != mip_get_next_string( item, &pinType ))
    {
        qCritical() << "could not parse pin type";
        return EINVAL;
    }

    if (ENOERR != mip_get_next_string( item, &pin ))
    {
        qCritical() << "could not parse current pin code";
        return EINVAL;
    }

    result = test.verifyPin("valid", pinType, pin);
    g_pResult->StepPassed( __PRETTY_FUNCTION__, result);

    free(pinType);
    free(pin);

    return ENOERR;
}

/**
 * VerifyInvalidPin
 * Verifies the invalid pin by enabling or disabling the pin
 * Usage: VerifyInvalidPin [pinType, invalid pin]
 * @param pinType, invalid pin
 * @return ENOERR
 */
LOCAL int VerifyInvalidPin(MinItemParser * item)
{
    MWTS_ENTER;

    char *pinType = NULL;
    char *pin = NULL;
    bool result = false;

    if (ENOERR != mip_get_next_string( item, &pinType ))
    {
        qCritical() << "could not parse pin type";
        return EINVAL;
    }

    if (ENOERR != mip_get_next_string( item, &pin ))
    {
        qCritical() << "could not parse current pin code";
        return EINVAL;
    }

    result = test.verifyPin("invalid", pinType, pin);
    g_pResult->StepPassed( __PRETTY_FUNCTION__, result);

    free(pinType);
    free(pin);

    return ENOERR;
}

/**
 * VerifyPuk
 * Verifies the puk by resetting a new pin code (hard-coded)
 * TODO resetPin, it does not work. It might be a bug report to ofono
 * Usage: VerifyPuk [puk]
 * @param puk code
 * @return ENOERR
 */
LOCAL int VerifyPuk(MinItemParser * item)
{
    MWTS_ENTER;

    char *puk = NULL;
    bool result = false;

    if (ENOERR != mip_get_next_string( item, &puk ))
    {
        qCritical() << "could not parse puk code";
        return EINVAL;
    }

    result = test.verifyPuk("valid", puk);
    g_pResult->StepPassed( __PRETTY_FUNCTION__, result);

    free(puk);

    return ENOERR;
}

/**
 * VerifyInvalidPuk
 * Verifies the invalid puk by resetting a new pin code (hard-coded)
 * TODO resetPin, it does not work. It might be a bug report to ofono
 * Usage: VerifyPuk [puk]
 * @param invalid puk code
 * @return ENOERR
 */
LOCAL int VerifyInvalidPuk(MinItemParser * item)
{
    MWTS_ENTER;

    char *puk = NULL;
    bool result = false;

    if (ENOERR != mip_get_next_string( item, &puk ))
    {
        qCritical() << "could not parse puk code";
        return EINVAL;
    }

    result = test.verifyPuk("invalid", puk);
    g_pResult->StepPassed( __PRETTY_FUNCTION__, result);

    free(puk);

    return ENOERR;
}

/**
  * Enters the currently pending pin. The type value must match the
  * pin type being asked in the "pin required" (SimInfo) property
  * @param pinType, pin
  * @return ENOERR
  */
LOCAL int EnterPin(MinItemParser * item)
{
    MWTS_ENTER;

    char *pinType = NULL;
    char *pin = NULL;
    bool result = false;

    if (ENOERR != mip_get_next_string( item, &pinType ))
    {
        qCritical() << "could not parse pin type";
        return EINVAL;
    }

    if (ENOERR != mip_get_next_string( item, &pin ))
    {
        qCritical() << "could not parse current pin code";
        return EINVAL;
    }

    result = test.enterPin(pinType, pin);
    g_pResult->StepPassed( __PRETTY_FUNCTION__, result);

    free(pinType);
    free(pin);

    return ENOERR;
}

/**
 * Prints info about the sim card (locked pins, pin required)
 * @return ENOERR
 */
LOCAL int SimInfo(__attribute__((unused)) MinItemParser * item)
{
    MWTS_ENTER;

    test.simInfo();

    return ENOERR;
}

/**
 * Test function list for MIN
 * @return ENOERR
 */
int ts_get_test_cases( DLList** list )
{
    // declare common functions like Init, Close..
    MwtsMin::DeclareFunctions(list);

    ENTRYTC(*list,"ChangePin", ChangePin);
    ENTRYTC(*list,"EnablePin", EnablePin);
    ENTRYTC(*list,"DisablePin", DisablePin);
    ENTRYTC(*list,"EnterPin", EnterPin);
    ENTRYTC(*list,"ResetPin", ResetPin);

    ENTRYTC(*list,"VerifyPin", VerifyPin);
    ENTRYTC(*list,"VerifyInvalidPin", VerifyInvalidPin);

    ENTRYTC(*list,"VerifyPuk", VerifyPuk);
    ENTRYTC(*list,"VerifyInvalidPuk", VerifyInvalidPuk);

    ENTRYTC(*list,"SimInfo", SimInfo);

    return ENOERR;

}
