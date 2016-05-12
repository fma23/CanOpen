/**
 * CANopen Heartbeat consumer protocol.
 *
 * @file        CO_HBconsumer.h
 * @ingroup     CO_HBconsumer
 * @version     SVN: \$Id: CO_HBconsumer.h 46 2013-08-24 09:18:16Z jani22 $
 * @author      Janez Paternoster
 * @copyright   2004 - 2013 Janez Paternoster
 *
 * This file is part of CANopenNode, an opensource CANopen Stack.
 * Project home page is <http://canopennode.sourceforge.net>.
 * For more information on CANopen see <http://www.can-cia.org/>.
 *
 * CANopenNode is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
/**
 * @defgroup CO_HBconsumer Heartbeat consumer
 * @ingroup CO_CANopen
 * @{
 *
 * CANopen Heartbeat consumer protocol.
 *
 * Heartbeat consumer monitors Heartbeat messages from remote nodes. If any
 * monitored node don't send his Heartbeat in specified time, Heartbeat consumer
 * sends emergency message. If all monitored nodes are operational, then
 * variable _allMonitoredOperational_ inside CO_HBconsumer_t is set to true.
 * Monitoring starts after the reception of the first HeartBeat (not bootup).
 *
 * @see  @ref CO_NMT_Heartbeat
 */

#ifndef CO_HB_CONS_H
#define CO_HB_CONS_H

/* Includes ------------------------------------------------------------------*/
#include "common.h"


/* Exported define -----------------------------------------------------------*/
/**
 * One monitored node inside CO_HBconsumer_t.
 */
typedef struct{
    uint8_t             NMTstate;       /**< Of the remote node */
    uint8_t             monStarted;     /**< True after reception of the first Heartbeat mesage */
    uint16_t            timeoutTimer;   /**< Time since last heartbeat received */
    uint16_t            time;           /**< Consumer heartbeat time from OD */
    bool                CANrxNew;       /**< True if new Heartbeat message received from the CAN bus */
}CO_HBconsNode_t;

/**
 * Heartbeat consumer object.
 *
 * Object is initilaized by CO_HBconsumer_init(). It contains an array of
 * CO_HBconsNode_t objects.
 */
typedef struct{
    CO_EM_t            *em;             /**< From CO_HBconsumer_init() */
    const uint32_t     *HBconsTime;     /**< From CO_HBconsumer_init() */
    CO_HBconsNode_t    *monitoredNodes; /**< From CO_HBconsumer_init() */
    uint8_t             numberOfMonitoredNodes; /**< From CO_HBconsumer_init() */
    /** True, if all monitored nodes are NMT operational or no node is
        monitored. Can be read by the application */
    uint8_t             allMonitoredOperational;
    CO_CANmodule_t     *CANdevRx;       /**< From CO_HBconsumer_init() */
    uint16_t            CANdevRxIdxStart; /**< From CO_HBconsumer_init() */
}CO_HBconsumer_t;

/* Exported variables -----------------------------------------------------------*/

/* Exported functions -----------------------------------------------------------*/

CO_ReturnError_t CO_HBconsumer_init(
        CO_HBconsumer_t        *HBcons,
        CO_EM_t                *em,
        CO_SDO_t               *SDO,
        const uint32_t          HBconsTime[],
        CO_HBconsNode_t         monitoredNodes[],
        uint8_t                 numberOfMonitoredNodes,
        CO_CANmodule_t         *CANdevRx,
        uint16_t                CANdevRxIdxStart);

void CO_HBconsumer_process(
        CO_HBconsumer_t        *HBcons,
        bool                    NMTisPreOrOperational,
        uint16_t                timeDifference_ms);


#endif    /* CO_HB_CONS_H */
