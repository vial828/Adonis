/***************************************************************************//**
* \file cyhal_i3c_impl.h
*
* Description:
* Provides a high level interface for interacting with the Infineon I3C. This is
* a wrapper around the lower level PDL API.
*
********************************************************************************
* \copyright
* Copyright 2023 Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation
*
* SPDX-License-Identifier: Apache-2.0
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#pragma once

#if (CYHAL_DRIVER_AVAILABLE_I3C)

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/** Named define for maximum number of the devices attached to the I3C bus. */
#define CYHAL_I3C_MAX_ATTACHED_DEVICES     (CY_I3C_MAX_DEVS)

/** Named define for I3C Broadcast Address. */
#define CYHAL_I3C_BROADCAST_ADDR           (CY_I3C_BROADCAST_ADDR)

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* CYHAL_DRIVER_AVAILABLE_I3C */
