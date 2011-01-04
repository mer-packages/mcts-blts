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
    g_pResult->StepPassed( __FUNCTION__, result);

    free(pinType);
    free(oldPin);
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
    g_pResult->StepPassed( __FUNCTION__, result);

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
    g_pResult->StepPassed( __FUNCTION__, result);

    free(pinType);
    free(pin);

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

    return ENOERR;

}
