/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <vector>
#include <stdint.h>

namespace Network
{

typedef int32_t ActionId;

typedef std::vector<ActionId> ActionsStorage;

const ActionId UNKNOWN_ACTION = 0;
const ActionId ACTION_NOT_FOUND_ERROR = 1;
const ActionId ACTION_ERROR = 2;
const ActionId ACTION_SUCCESS = 3;
const ActionId QUERY_ERROR = 4;
const ActionId AFFECTOR_ERROR = 5;
}
