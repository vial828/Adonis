/***************************************************************************//**
* \file cy_device_headers.h
*
* \brief
* Common header file to be included by the drivers.
*
********************************************************************************
* \copyright
* (c) (2016-2023), Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.
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

#ifndef _CY_DEVICE_HEADERS_H_
#define _CY_DEVICE_HEADERS_H_

#if defined (COMPONENT_SECURE_DEVICE)

#if defined (PSE845GPS4DFMC4)
    #include "pse845gps4dfmc4_s.h"
#elif defined (PSE845GPS4DFNC4)
    #include "pse845gps4dfnc4_s.h"
#elif defined (PSE846GPS4DBZC4)
    #include "pse846gps4dbzc4_s.h"
#elif defined (PSE846GPS4DAEI3)
    #include "pse846gps4daei3_s.h"
#elif defined (PSE845GPS2DFMC4)
    #include "pse845gps2dfmc4_s.h"
#elif defined (PSE845GPS2DFNC4)
    #include "pse845gps2dfnc4_s.h"
#elif defined (PSE846GPS2DBZC4)
    #include "pse846gps2dbzc4_s.h"
#elif defined (PSE846GPS2DAEI3)
    #include "pse846gps2daei3_s.h"
#elif defined (PSE845GOS4DFMC4)
    #include "pse845gos4dfmc4_s.h"
#elif defined (PSE845GOS4DFNC4)
    #include "pse845gos4dfnc4_s.h"
#elif defined (PSE846GOS4DBZC4)
    #include "pse846gos4dbzc4_s.h"
#elif defined (PSE846GOS4DAEI3)
    #include "pse846gos4daei3_s.h"
#elif defined (PSE845GOS2DFMC4)
    #include "pse845gos2dfmc4_s.h"
#elif defined (PSE845GOS2DFNC4)
    #include "pse845gos2dfnc4_s.h"
#elif defined (PSE846GOS2DBZC4)
    #include "pse846gos2dbzc4_s.h"
#elif defined (PSE846GOS2DAEI3)
    #include "pse846gos2daei3_s.h"
#else
    #error Undefined part number
#endif

#else
#if defined (PSE845GPS4DFMC4)
    #include "pse845gps4dfmc4.h"
#elif defined (PSE845GPS4DFNC4)
    #include "pse845gps4dfnc4.h"
#elif defined (PSE846GPS4DBZC4)
    #include "pse846gps4dbzc4.h"
#elif defined (PSE846GPS4DAEI3)
    #include "pse846gps4daei3.h"
#elif defined (PSE845GPS2DFMC4)
    #include "pse845gps2dfmc4.h"
#elif defined (PSE845GPS2DFNC4)
    #include "pse845gps2dfnc4.h"
#elif defined (PSE846GPS2DBZC4)
    #include "pse846gps2dbzc4.h"
#elif defined (PSE846GPS2DAEI3)
    #include "pse846gps2daei3.h"
#elif defined (PSE845GOS4DFMC4)
    #include "pse845gos4dfmc4.h"
#elif defined (PSE845GOS4DFNC4)
    #include "pse845gos4dfnc4.h"
#elif defined (PSE846GOS4DBZC4)
    #include "pse846gos4dbzc4.h"
#elif defined (PSE846GOS4DAEI3)
    #include "pse846gos4daei3.h"
#elif defined (PSE845GOS2DFMC4)
    #include "pse845gos2dfmc4.h"
#elif defined (PSE845GOS2DFNC4)
    #include "pse845gos2dfnc4.h"
#elif defined (PSE846GOS2DBZC4)
    #include "pse846gos2dbzc4.h"
#elif defined (PSE846GOS2DAEI3)
    #include "pse846gos2daei3.h"
#else
    #error Undefined part number
#endif

#endif

#endif /* _CY_DEVICE_HEADERS_H_ */


/* [] END OF FILE */
