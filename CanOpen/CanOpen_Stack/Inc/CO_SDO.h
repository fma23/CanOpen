/**
 * CANopen Service Data Object - server protocol.
 *
 * @file        CO_SDO.h
 * @ingroup     CO_SDO
 * @version     SVN: \$Id: CO_SDO.h 46 2013-08-24 09:18:16Z jani22 $
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
 * @defgroup CO_SDO SDO server
 * @ingroup CO_CANopen
 * @{
 *
 * CANopen Service Data Object - server protocol.
 *
 * Service data objects (SDOs) allow the access to any entry of the CANopen
 * Object dictionary. An SDO establishes a peer-to-peer communication channel
 * between two devices. In addition, the SDO protocol enables to transfer any
 * amount of data in a segmented way. Therefore the SDO protocol is mainly used
 * in order to communicate configuration data.
 *
 * All CANopen devices must have implemented SDO server and first SDO server
 * channel. Servers serves data from Object dictionary. Object dictionary
 * is a collection of variables, arrays or records (structures), which can be
 * used by the stack or by the application. This file (CO_SDO.h) implements
 * SDO server.
 *
 * SDO client can be (optionally) implemented on one (or multiple, if multiple
 * SDO channels are used) device in CANopen network. Usually this is master
 * device and provides also some kind of user interface, so configuration of
 * the network is possible. Code for the SDO client is in file CO_SDOmaster.h.
 *
 * SDO communication cycle is initiated by the client. Client can upload (read) data
 * from device or can download (write) data to device. If data are less or equal
 * of 4 bytes long, communication is finished by one server response (expedited
 * transfer). If data are longer, they are split into multiple segments of
 * request/response pairs (normal or segmented transfer). For longer data there
 * is also a block transfer protocol, which transfers larger block of data in
 * secure way with little protocol overhead. If error occurs during SDO transfer
 * #CO_SDO_abortCode_t is send by client or server and transfer is terminated.
 */
/**
 * @defgroup CO_SDO_messageContents SDO message contents
 *
 * Excerpt from CiA DS301, V4.2.
 *
 * For CAN identifier see #CO_Default_CAN_ID_t
 *
 * Expedited transfer is used for transmission of up to 4 data bytes. It consists
 * of one SDO request and one response. For longer variables is used segmented
 * or block transfer.
 *
 * ####Initiate SDO download (client request)
 *  - byte 0:       SDO command specifier. 8 bits: `0010nnes` (nn: if e=s=1,
 *                  number of data bytes, that do *not* contain data; e=1 for
 *                  expedited transfer; s=1 if data size is indicated).
 *  - byte 1..2:    Object index.
 *  - byte 3:       Object subIndex.
 *  - byte 4..7:    Expedited data or data size if segmented transfer.
 *
 * ####Initiate SDO download (server response)
 *  - byte 0:       SDO command specifier. 8 bits: `01100000`.
 *  - byte 1..2:    Object index.
 *  - byte 3:       Object subIndex.
 *  - byte 4..7:    reserved.
 *
 * ####Download SDO segment (client request)
 *  - byte 0:       SDO command specifier. 8 bits: `000tnnnc` (t: toggle bit set
 *                  to 0 in first segment; nnn: number of data bytes, that do
 *                  *not* contain data; c=1 if this is the last segment).
 *  - byte 1..7:    Data segment.
 *
 * ####Download SDO segment (server response)
 *  - byte 0:       SDO command specifier. 8 bits: `001t0000` (t: toggle bit set
 *                  to 0 in first segment).
 *  - byte 1..7:    Reserved.
 *
 * ####Initiate SDO upload (client request)
 *  - byte 0:       SDO command specifier. 8 bits: `01000000`.
 *  - byte 1..2:    Object index.
 *  - byte 3:       Object subIndex.
 *  - byte 4..7:    Reserved.
 *
 * ####Initiate SDO upload (server response)
 *  - byte 0:       SDO command specifier. 8 bits: `0100nnes` (nn: if e=s=1,
 *                  number of data bytes, that do *not* contain data; e=1 for
 *                  expedited transfer; s=1 if data size is indicated).
 *  - byte 1..2:    Object index.
 *  - byte 3:       Object subIndex.
 *  - byte 4..7:    reserved.
 *
 * ####Upload SDO segment (client request)
 *  - byte 0:       SDO command specifier. 8 bits: `011t0000` (t: toggle bit set
 *                  to 0 in first segment).
 *  - byte 1..7:    Reserved.
 *
 * ####Upload SDO segment (server response)
 *  - byte 0:       SDO command specifier. 8 bits: `000tnnnc` (t: toggle bit set
 *                  to 0 in first segment; nnn: number of data bytes, that do
 *                  *not* contain data; c=1 if this is the last segment).
 *  - byte 1..7:    Data segment.
 *
 * ####Abort SDO transfer (client or server)
 *  - byte 0:       SDO command specifier. 8 bits: `10000000`.
 *  - byte 1..2:    Object index.
 *  - byte 3:       Object subIndex.
 *  - byte 4..7:    #CO_SDO_abortCode_t.
 *
 * ####Block transfer
 *     See DS301 V4.2.
 */

#ifndef CO_SDO_H
#define CO_SDO_H

/* Includes ------------------------------------------------------------------*/
#include "common.h"

/* Exported define -----------------------------------------------------------*/
/**
 * SDO abort codes.
 *
 * Send with Abort SDO transfer message.
 *
 * The abort codes not listed here are reserved.
 */
typedef enum{
    CO_SDO_AB_NONE                  = 0x00000000U, /**< 0x00000000, No abort */
    CO_SDO_AB_TOGGLE_BIT            = 0x05030000U, /**< 0x05030000, Toggle bit not altered */
    CO_SDO_AB_TIMEOUT               = 0x05040000U, /**< 0x05040000, SDO protocol timed out */
    CO_SDO_AB_CMD                   = 0x05040001U, /**< 0x05040001, Command specifier not valid or unknown */
    CO_SDO_AB_BLOCK_SIZE            = 0x05040002U, /**< 0x05040002, Invalid block size in block mode */
    CO_SDO_AB_SEQ_NUM               = 0x05040003U, /**< 0x05040003, Invalid sequence number in block mode */
    CO_SDO_AB_CRC                   = 0x05040004U, /**< 0x05040004, CRC error (block mode only) */
    CO_SDO_AB_OUT_OF_MEM            = 0x05040005U, /**< 0x05040005, Out of memory */
    CO_SDO_AB_UNSUPPORTED_ACCESS    = 0x06010000U, /**< 0x06010000, Unsupported access to an object */
    CO_SDO_AB_WRITEONLY             = 0x06010001U, /**< 0x06010001, Atempt to read a write only object */
    CO_SDO_AB_READONLY              = 0x06010002U, /**< 0x06010002, Atempt to write a read only object */
    CO_SDO_AB_NOT_EXIST             = 0x06020000U, /**< 0x06020000, Object does not exist */
    CO_SDO_AB_NO_MAP                = 0x06040041U, /**< 0x06040041, Object cannot be mapped to the PDO */
    CO_SDO_AB_MAP_LEN               = 0x06040042U, /**< 0x06040042, Number and length of object to be mapped exceeds PDO length */
    CO_SDO_AB_PRAM_INCOMPAT         = 0x06040043U, /**< 0x06040043, General pparameter incompatibility reasons */
    CO_SDO_AB_DEVICE_INCOMPAT       = 0x06040047U, /**< 0x06040047, General internal incompatibility in device */
    CO_SDO_AB_HW                    = 0x06060000U, /**< 0x06060000, Access failed due to hardware error */
    CO_SDO_AB_TYPE_MISMATCH         = 0x06070010U, /**< 0x06070010, Data type does not match, length of service parameter does not match */
    CO_SDO_AB_DATA_LONG             = 0x06070012U, /**< 0x06070012, Data type does not match, length of service parameter too high */
    CO_SDO_AB_DATA_SHORT            = 0x06070013U, /**< 0x06070013, Data type does not match, length of service parameter too short */
    CO_SDO_AB_SUB_UNKNOWN           = 0x06090011U, /**< 0x06090011, Sub index does not exist */
    CO_SDO_AB_INVALID_VALUE         = 0x06090030U, /**< 0x06090030, Invalid value for parameter (download only). */
    CO_SDO_AB_VALUE_HIGH            = 0x06090031U, /**< 0x06090031, Value range of parammeter written too high */
    CO_SDO_AB_VALUE_LOW             = 0x06090032U, /**< 0x06090032, Value range of parammeter written too low */
    CO_SDO_AB_MAX_LESS_MIN          = 0x06090036U, /**< 0x06090036, Maximum value is less than minimum value. */
    CO_SDO_AB_NO_RESOURCE           = 0x060A0023U, /**< 0x060A0023, Resource not available: SDO connection */
    CO_SDO_AB_GENERAL               = 0x08000000U, /**< 0x08000000, General error */
    CO_SDO_AB_DATA_TRANSF           = 0x08000020U, /**< 0x08000020, Data cannot be transfered or stored to application */
    CO_SDO_AB_DATA_LOC_CTRL         = 0x08000021U, /**< 0x08000021, Data cannot be transfered or stored to application because of local control */
    CO_SDO_AB_DATA_DEV_STATE        = 0x08000022U, /**< 0x08000022, Data cannot be transfered or stored to application because of present device state */
    CO_SDO_AB_DATA_OD               = 0x08000023U, /**< 0x08000023, Object dictionary not present or dynamic generation fails */
    CO_SDO_AB_NO_DATA               = 0x08000024U  /**< 0x08000024, No data available */
}CO_SDO_abortCode_t;

/**
 * @defgroup CO_SDO_objectDictionary Object dictionary
 *
 * CANopen Object dictionary implementation in CANopenNode.
 *
 * CANopen Object dictionary is a collection of different data items, which can
 * be used by the stack or by the application.
 *
 * Each Object dictionary entry is located under 16-bit index, as specified
 * by the CANopen:
 *  - 0x0001..0x025F: Data type definitions.
 *  - 0x1000..0x1FFF: Communication profile area.
 *  - 0x2000..0x5FFF: Manufacturer-specific profile area.
 *  - 0x6000..0x9FFF: Standardized device profile area for eight logical devices.
 *  - 0xA000..0xAFFF: Standardized network variable area.
 *  - 0xB000..0xBFFF: Standardized system variable area.
 *  - Other:          Reserved.
 *
 * If Object dictionary entry has complex data type (array or structure),
 * then 8-bit subIndex specifies the sub-member of the entry. In that case
 * subIndex 0x00 is encoded as uint8_t and specifies the highest available
 * subIndex with that entry. Subindex 0xFF has special meaning in the standard
 * and is not supported by CANopenNode.
 *
 * ####Object type of one Object dictionary entry
 *  - NULL:         Not used by CANopenNode.
 *  - DOMAIN:       Block of data of variable length. Data and length are
 *                  under control of the application.
 *  - DEFTYPE:      Definition of CANopen basic data type, for example
 *                  INTEGER16.
 *  - DEFSTRUCT:    Definition of complex data type - structure, which is
 *                  used with RECORD.
 *  - VAR:          Variable of CANopen basic data type. Subindex is 0.
 *  - ARRAY:        Array of multiple variables of the same CANopen basic
 *                  data type. Subindex 1..arrayLength specifies sub-member.
 *  - RECORD:       Record or structure of multiple variables of different
 *                  CANopen basic data type. Subindex specifies sub-member.
 *
 *
 * ####Implementation in CANopenNode
 * Object dictionary in CANopenNode is implemeted in CO_OD.h and CO_OD.c files.
 * These files are application specific and must be generated by Object
 * dictionary editor (application is included by the stack).
 *
 * CO_OD.h and CO_OD.c files include:
 *  - Structure definitions for records.
 *  - Global declaration and initialization of all variables, arrays and records
 *    mapped to Object dictionary. Variables are distrbuted in multiple objects,
 *    depending on memory location. This eases storage to different memories in
 *    microcontroller, like eeprom or flash.
 *  - Constant array of multiple Object dictionary entries of type
 *    CO_OD_entry_t. If object type is record, then entry includes additional
 *    constant array with members of type CO_OD_entryRecord_t. Each OD entry
 *    includes information: index, maxSubIndex, #CO_SDO_OD_attributes_t, data size and
 *    pointer to variable.
 *
 *
 * Function CO_SDO_init() initializes object CO_SDO_t, which includes SDO
 * server and Object dictionary. Interface to Object dictionary is provided by
 * following functions: CO_OD_find() finds OD entry by index, CO_OD_getLength()
 * returns length of variable, CO_OD_getAttribute returns attribute and
 * CO_OD_getDataPointer() returns pointer to data. These functions are used by
 * SDO server and by PDO configuration.
 *
 * Application doesn't need to know anything about the Object dictionary. It can
 * use variables specified in CO_OD.h file directly. If it needs more control
 * over the CANopen communication with the variables, it can configure additional
 * functionality with function CO_OD_configure(). Additional functionality
 * include: @ref CO_SDO_OD_function and #CO_SDO_OD_flags_t.
 */


/**
 * @defgroup CO_SDO_OD_function Object Dictionary function
 *
 * Optional application specific function, which may manipulate data downloaded
 * or uploaded via SDO.
 *
 * Object dictionary function is external function defined by application or
 * by other stack files. It may be registered for specific Object dictionary
 * entry (with specific index). If it is registered, it is called (through
 * function pointer) from SDO server. It may verify and manipulate data during
 * SDO transfer. Object dictionary function can be registered by function
 * CO_OD_configure().
 *
 * ####SDO download (writing to Object dictionary)
 *     After SDO client transfers data to the server, data are stored in internal
 *     buffer. If data contains multibyte variable and processor is big endian,
 *     then data bytes are swapped. Object dictionary function is called if
 *     registered. Data may be verified and manipulated inside that function. After
 *     function exits, data are copied to location as specified in CO_OD_entry_t.
 *
 * ####SDO upload (reading from Object dictionary)
 *     Before start of SDO upload, data are read from Object dictionary into
 *     internal buffer. If necessary, bytes are swapped.
 *     Object dictionary function is called if registered. Data may be
 *     manipulated inside that function. After function exits, data are
 *     transfered via SDO server.
 *
 * ####Domain data type
 *     If data type is domain, then length is not specified by Object dictionary.
 *     In that case Object dictionary function must be used. In case of
 *     download it must store the data in own location. In case of upload it must
 *     write the data (maximum size is specified by length) into data buffer and
 *     specify actual length. With domain data type it is possible to transfer
 *     data, which are longer than #CO_SDO_BUFFER_SIZE. In that case
 *     Object dictionary function is called multiple times between SDO transfer.
 *
 * ####Parameter to function:
 *     ODF_arg     - Pointer to CO_ODF_arg_t object filled before function call.
 *
 * ####Return from function:
 *  - 0: Data transfer is successful
 *  - Different than 0: Failure. See #CO_SDO_abortCode_t.
 */

/**
 * SDO buffer size.
 *
 * Size of the internal SDO buffer.
 *
 * Size must be at least equal to size of largest variable in @ref CO_SDO_objectDictionary.
 * If data type is domain, data length is not limited to SDO buffer size. If
 * block transfer is implemeted, value should be set to 889.
 *
 * Value can be in range from 7 to 889 bytes.
 */
    #ifndef CO_SDO_BUFFER_SIZE
        #define CO_SDO_BUFFER_SIZE    32
    #endif

/**
 * Object Dictionary attributes. Bit masks for attribute in CO_OD_entry_t.
 */
typedef enum{
    CO_ODA_MEM_ROM          = 0x0001U,  /**< Variable is located in ROM memory */
    CO_ODA_MEM_RAM          = 0x0002U,  /**< Variable is located in RAM memory */
    CO_ODA_MEM_EEPROM       = 0x0003U,  /**< Variable is located in EEPROM memory */
    CO_ODA_READABLE         = 0x0004U,  /**< SDO server may read from the variable */
    CO_ODA_WRITEABLE        = 0x0008U,  /**< SDO server may write to the variable */
    CO_ODA_RPDO_MAPABLE     = 0x0010U,  /**< Variable is mappable for RPDO */
    CO_ODA_TPDO_MAPABLE     = 0x0020U,  /**< Variable is mappable for TPDO */
    CO_ODA_TPDO_DETECT_COS  = 0x0040U,  /**< If variable is mapped to any PDO, then
                                             PDO is automatically send, if variable
                                             changes its value */
    CO_ODA_MB_VALUE         = 0x0080U   /**< True when variable is a multibyte value */
}CO_SDO_OD_attributes_t;

/**
 * Common DS301 object dictionary entries.
 */
typedef enum{
    OD_H1000_DEV_TYPE             = 0x1000U,/**< Device type */
    OD_H1001_ERR_REG              = 0x1001U,/**< Error register */
    OD_H1002_MANUF_STATUS_REG     = 0x1002U,/**< Manufacturer status register */
    OD_H1003_PREDEF_ERR_FIELD     = 0x1003U,/**< Predefined error field */
    OD_H1004_RSV                  = 0x1004U,/**< Reserved */
    OD_H1005_COBID_SYNC           = 0x1005U,/**< Sync message cobid */
    OD_H1006_COMM_CYCL_PERIOD     = 0x1006U,/**< Communication cycle period */
    OD_H1007_SYNC_WINDOW_LEN      = 0x1007U,/**< Sync windows length */
    OD_H1008_MANUF_DEV_NAME       = 0x1008U,/**< Manufacturer device name */
    OD_H1009_MANUF_HW_VERSION     = 0x1009U,/**< Manufacturer hardware version */
    OD_H100A_MANUF_SW_VERSION     = 0x100AU,/**< Manufacturer software version */
    OD_H100B_RSV                  = 0x100BU,/**< Reserved */
    OD_H100C_GUARD_TIME           = 0x100CU,/**< Guard time */
    OD_H100D_LIFETIME_FACTOR      = 0x100DU,/**< Life time factor */
    OD_H100E_RSV                  = 0x100EU,/**< Reserved */
    OD_H100F_RSV                  = 0x100FU,/**< Reserved */
    OD_H1010_STORE_PARAM_FUNC     = 0x1010U,/**< Store parametter in persistent memory function */
    OD_H1011_REST_PARAM_FUNC      = 0x1011U,/**< Restore default parameter function */
    OD_H1012_COBID_TIME           = 0x1012U,/**< Timestamp message cobid */
    OD_H1013_HIGH_RES_TIMESTAMP   = 0x1013U,/**< High resolution timestamp */
    OD_H1014_COBID_EMERGENCY      = 0x1014U,/**< Emergency message cobid */
    OD_H1015_INHIBIT_TIME_MSG     = 0x1015U,/**< Inhibit time message */
    OD_H1016_CONSUMER_HB_TIME     = 0x1016U,/**< Consumer heartbeat time */
    OD_H1017_PRODUCER_HB_TIME     = 0x1017U,/**< Producer heartbeat time */
    OD_H1018_IDENTITY_OBJECT      = 0x1018U,/**< Identity object */
    OD_H1019_SYNC_CNT_OVERFLOW    = 0x1019U,/**< Sync counter overflow value */
    OD_H1020_VERIFY_CONFIG        = 0x1020U,/**< Verify configuration */
    OD_H1021_STORE_EDS            = 0x1021U,/**< Store EDS */
    OD_H1022_STORE_FORMAT         = 0x1022U,/**< Store format */
    OD_H1023_OS_CMD               = 0x1023U,/**< OS command */
    OD_H1024_OS_CMD_MODE          = 0x1024U,/**< OS command mode */
    OD_H1025_OS_DBG_INTERFACE     = 0x1025U,/**< OS debug interface */
    OD_H1026_OS_PROMPT            = 0x1026U,/**< OS prompt */
    OD_H1027_MODULE_LIST          = 0x1027U,/**< Module list */
    OD_H1028_EMCY_CONSUMER        = 0x1028U,/**< Emergency consumer object */
    OD_H1029_ERR_BEHAVIOR         = 0x1029U,/**< Error behavior */
    OD_H1200_SDO_SERVER_PARAM     = 0x1200U,/**< SDO server paraeters */
    OD_H1280_SDO_CLIENT_PARAM     = 0x1280U,/**< SDO client parameters */
    OD_H1400_RXPDO_1_PARAM        = 0x1400U,/**< RXPDO communication parameter */
    OD_H1401_RXPDO_2_PARAM        = 0x1401U,/**< RXPDO communication parameter */
    OD_H1402_RXPDO_3_PARAM        = 0x1402U,/**< RXPDO communication parameter */
    OD_H1403_RXPDO_4_PARAM        = 0x1403U,/**< RXPDO communication parameter */
    OD_H1600_RXPDO_1_MAPPING      = 0x1600U,/**< RXPDO mapping paramaters */
    OD_H1601_RXPDO_2_MAPPING      = 0x1601U,/**< RXPDO mapping paramaters */
    OD_H1602_RXPDO_3_MAPPING      = 0x1602U,/**< RXPDO mapping paramaters */
    OD_H1603_RXPDO_4_MAPPING      = 0x1603U,/**< RXPDO mapping paramaters */
    OD_H1800_TXPDO_1_PARAM        = 0x1800U,/**< TXPDO communication parameter */
    OD_H1801_TXPDO_2_PARAM        = 0x1801U,/**< TXPDO communication parameter */
    OD_H1802_TXPDO_3_PARAM        = 0x1802U,/**< TXPDO communication parameter */
    OD_H1803_TXPDO_4_PARAM        = 0x1803U,/**< TXPDO communication parameter */
    OD_H1A00_TXPDO_1_MAPPING      = 0x1A00U,/**< TXPDO mapping parameters */
    OD_H1A01_TXPDO_2_MAPPING      = 0x1A01U,/**< TXPDO mapping parameters */
    OD_H1A02_TXPDO_3_MAPPING      = 0x1A02U,/**< TXPDO mapping parameters */
    OD_H1A03_TXPDO_4_MAPPING      = 0x1A03U /**< TXPDO mapping parameters */
}CO_ObjDicId_t;

/**
 * Bit masks for flags associated with variable from @ref CO_SDO_objectDictionary.
 *
 * This additional functionality of any variable in @ref CO_SDO_objectDictionary can be
 * enabled by function CO_OD_configure(). Location of the flag byte can be
 * get from function CO_OD_getFlagsPointer().
 */
typedef enum{
    /** Variable was written by RPDO. Flag can be cleared by application */
    CO_ODFL_RPDO_WRITTEN        = 0x01U,
    /** Variable is mapped to TPDO */
    CO_ODFL_TPDO_MAPPED         = 0x02U,
    /** Change of state bit, initially copy of attribute from CO_OD_entry_t.
    If set and variable is mapped to TPDO, TPDO will be automatically send,
    if variable changed */
    CO_ODFL_TPDO_COS_ENABLE     = 0x04U,
    /** PDO send bit, can be set by application. If variable is mapped into
    TPDO, TPDO will be send and bit will be cleared. */
    CO_ODFL_TPDO_SEND           = 0x08U,
    /** Variable was accessed by SDO download */
    CO_ODFL_SDO_DOWNLOADED      = 0x10U,
    /** Variable was accessed by SDO upload */
    CO_ODFL_SDO_UPLOADED        = 0x20U,
    /** Reserved */
    CO_ODFL_BIT_6               = 0x40U,
    /** Reserved */
    CO_ODFL_BIT_7               = 0x80U
}CO_SDO_OD_flags_t;

/**
 * Object for one entry with specific index in @ref CO_SDO_objectDictionary.
 */
typedef struct {
    /** The index of Object from 0x1000 to 0xFFFF */
    uint16_t            index;
    /** Number of (sub-objects - 1). If Object Type is variable, then
    maxSubIndex is 0, otherwise maxSubIndex is equal or greater than 1. */
    uint8_t             maxSubIndex;
    /** If Object Type is record, attribute is set to zero. Attribute for
    each member is then set in special array with members of type
    CO_OD_entryRecord_t. If Object Type is Array, attribute is common for
    all array members. See #CO_SDO_OD_attributes_t. */
    uint16_t            attribute;
    /** If Object Type is Variable, length is the length of variable in bytes.
    If Object Type is Array, length is the length of one array member.
    If Object Type is Record, length is zero. Length for each member is
    set in special array with members of type CO_OD_entryRecord_t.
    If Object Type is Domain, length is zero. Length is specified
    by application in @ref CO_SDO_OD_function. */
    uint16_t            length;
    /** If Object Type is Variable, pData is pointer to data.
    If Object Type is Array, pData is pointer to data. Data doesn't
    include Sub-Object 0.
    If object type is Record, pData is pointer to special array
    with members of type CO_OD_entryRecord_t.
    If object type is Domain, pData is null. */
    void               *pData;
}CO_OD_entry_t;

/**
 * Object for record type entry in @ref CO_SDO_objectDictionary.
 *
 * See CO_OD_entry_t.
 */
typedef struct{
    /** See #CO_SDO_OD_attributes_t */
    void               *pData;
    /** Length of variable in bytes. If object type is Domain, length is zero */
    uint16_t            attribute;
    /** Pointer to data. If object type is Domain, pData is null */
    uint16_t            length;
}CO_OD_entryRecord_t;

/**
 * Object contains all information about the object being transfered by SDO server.
 *
 * Object is used as an argument to @ref CO_SDO_OD_function. It is also
 * part of the CO_SDO_t object.
 */
typedef struct{
    /** Informative parameter. It may point to object, which is connected
    with this OD entry. It can be used inside @ref CO_SDO_OD_function, ONLY
    if it was registered by CO_OD_configure() function before. */
    void               *object;
    /** SDO data buffer contains data, which are exchanged in SDO transfer.
    @ref CO_SDO_OD_function may verify or manupulate that data before (after)
    they are written to (read from) Object dictionary. Data have the same
    endianes as processor. Pointer must NOT be changed. (Data up to length
    can be changed.) */
    uint8_t            *data;
    /** Pointer to location in object dictionary, where data are stored.
    (informative refference to old data, read only). Data have the same
    endianes as processor. If data type is Domain, this variable is null. */
    const void         *ODdataStorage;
    /** Length of data in the above buffer. Read only, except for domain. If
    data type is domain see @ref CO_SDO_OD_function for special rules by upload. */
    uint16_t            dataLength;
    /** Attribute of object in Object dictionary (informative, must NOT be changed). */
    uint16_t            attribute;
    /** Pointer to the #CO_SDO_OD_flags_t byte. */
    uint8_t            *pFlags;
    /** Index of object in Object dictionary (informative, must NOT be changed). */
    uint16_t            index;
    /** Subindex of object in Object dictionary (informative, must NOT be changed). */
    uint8_t             subIndex;
    /** True, if SDO upload is in progress, false if SDO download is in progress. */
    bool                reading;
    /** Used by domain data type. Indicates the first segment. Variable is informative. */
    bool                firstSegment;
    /** Used by domain data type. If false by download, then application will
    receive more segments during SDO communication cycle. If uploading,
    application may set variable to false, so SDO server will call
    @ref CO_SDO_OD_function again for filling the next data. */
    bool                lastSegment;
    /** Used by domain data type. By upload @ref CO_SDO_OD_function may write total
    data length, so this information will be send in SDO upload initiate phase. It
    is not necessary to specify this variable. By download this variable contains
    total data size, if size is indicated in SDO download initiate phase */
    uint32_t            dataLengthTotal;
}CO_ODF_arg_t;

/**
 * Object is used as array inside CO_SDO_t, parallel to @ref CO_SDO_objectDictionary.
 *
 * Object is generated by function CO_OD_configure(). It is then used as
 * extension to Object dictionary entry at specific index.
 */
typedef struct{
    /** Pointer to @ref CO_SDO_OD_function */
    CO_SDO_abortCode_t (*pODFunc)(CO_ODF_arg_t *ODF_arg);
    /** Pointer to object, which will be passed to @ref CO_SDO_OD_function */
    void               *object;
    /** Pointer to #CO_SDO_OD_flags_t. If object type is array or record, this
    variable points to array with length equal to number of subindexes. */
    uint8_t            *flags;
}CO_OD_extension_t;

/**
 * SDO server object.
 */
typedef struct{
    /** 8 data bytes of the received message. */
    uint8_t             CANrxData[8];   /* Take care for correct (word) alignment! */
    /** SDO data buffer of size #CO_SDO_BUFFER_SIZE. */
    uint8_t             databuffer[CO_SDO_BUFFER_SIZE]; /* Take care for correct (word) alignment! */
    /** Internal flag indicates, that this object has own OD */
    bool                ownOD;
    /** Pointer to the @ref CO_SDO_objectDictionary (array) */
    const CO_OD_entry_t *OD;
    /** Size of the @ref CO_SDO_objectDictionary */
    uint16_t            ODSize;
    /** Pointer to array of CO_OD_extension_t objects. Size of the array is
    equal to ODSize. */
    CO_OD_extension_t  *ODExtensions;
    /** Offset in buffer of next data segment being read/written */
    uint16_t            bufferOffset;
    /** Sequence number of OD entry as returned from CO_OD_find() */
    uint16_t            entryNo;
    /** CO_ODF_arg_t object with additional variables. Reference to this object
    is passed to @ref CO_SDO_OD_function */
    CO_ODF_arg_t        ODF_arg;
    /** From CO_SDO_init() */
    uint8_t             nodeId;
    /** Current internal state of the SDO server state machine */
    uint8_t             state;
    /** Toggle bit in segmented transfer or block sequence in block transfer */
    uint8_t             sequence;
    /** Timeout timer for SDO communication */
    uint16_t            timeoutTimer;
    /** Number of segments per block with 1 <= blksize <= 127 */
    uint8_t             blksize;
    /** True, if CRC calculation by block transfer is enabled */
    bool                crcEnabled;
    /** Calculated CRC code */
    uint16_t            crc;
    /** Length of data in the last segment in block upload */
    uint8_t             lastLen;
    /** Indication end of block transfer */
    bool                endOfTransfer;
    /** Variable indicates, if new SDO message received from CAN bus */
    bool                CANrxNew;
    /** From CO_SDO_init() */
    CO_CANmodule_t     *CANdevTx;
    /** CAN transmit buffer inside CANdev for CAN tx message */
    CO_CANtx_t         *CANtxBuff;
}CO_SDO_t;

/* Exported variables -----------------------------------------------------------*/

/* Exported functions -----------------------------------------------------------*/
CO_ReturnError_t CO_SDO_init(
        CO_SDO_t               *SDO,
        uint16_t                COB_IDClientToServer,
        uint16_t                COB_IDServerToClient,
        uint16_t                ObjDictIndex_SDOServerParameter,
        CO_SDO_t               *parentSDO,
        const CO_OD_entry_t     OD[],
        uint16_t                ODSize,
        CO_OD_extension_t       ODExtensions[],
        uint8_t                 nodeId,
        CO_CANmodule_t         *CANdevRx,
        uint16_t                CANdevRxIdx,
        CO_CANmodule_t         *CANdevTx,
        uint16_t                CANdevTxIdx);

int8_t CO_SDO_process(
        CO_SDO_t               *SDO,
        bool                    NMTisPreOrOperational,
        uint16_t                timeDifference_ms,
        uint16_t                SDOtimeoutTime);

void CO_OD_configure(
        CO_SDO_t               *SDO,
        uint16_t                index,
        CO_SDO_abortCode_t    (*pODFunc)(CO_ODF_arg_t *ODF_arg),
        void                   *object,
        uint8_t                *flags,
        uint8_t                 flagsSize);

uint16_t CO_OD_find(CO_SDO_t *SDO, uint16_t index);
uint16_t CO_OD_getLength(CO_SDO_t *SDO, uint16_t entryNo, uint8_t subIndex);
uint16_t CO_OD_getAttribute(CO_SDO_t *SDO, uint16_t entryNo, uint8_t subIndex);
void* CO_OD_getDataPointer(CO_SDO_t *SDO, uint16_t entryNo, uint8_t subIndex);
uint8_t* CO_OD_getFlagsPointer(CO_SDO_t *SDO, uint16_t entryNo, uint8_t subIndex);
uint32_t CO_SDO_initTransfer(CO_SDO_t *SDO, uint16_t index, uint8_t subIndex);
uint32_t CO_SDO_readOD(CO_SDO_t *SDO, uint16_t SDOBufferSize);
uint32_t CO_SDO_writeOD(CO_SDO_t *SDO, uint16_t length);

#endif    /* CO_SDO_H */
