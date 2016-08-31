/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <NetworkAction.h>
const Network::ActionId ACT_USER_LOGIN = 10;
const Network::ActionId ACT_USER_LOGOUT = 11;
const Network::ActionId ACT_NEW_MESSAGE = 12;
const Network::ActionId QRY_NAME_CHECK = 13;
const Network::ActionId QRY_ASK_NAME_IS_USED = 14;
const Network::ActionId QRY_ASK_NAME_IS_FREE = 15;
const Network::ActionId ERR_NAME_IS_USED = 16;
