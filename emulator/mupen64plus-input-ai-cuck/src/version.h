/* This header file is for versioning information
 *
 */

#if !defined(INPUT_CUCK_VERSION_H)
#define INPUT_CUCK_VERSION_H

#define PLUGIN_NAME    "Mupen64Plus Deep RL input cucking plugin (man in the middle for normal input plugins)"
#define PLUGIN_VERSION           0x010000
#define INPUT_PLUGIN_API_VERSION 0x020100
#define CONFIG_API_VERSION       0x020100

#define VERSION_PRINTF_SPLIT(x) (((x) >> 16) & 0xffff), (((x) >> 8) & 0xff), ((x) & 0xff)

#endif /* #define INPUT_CUCK_VERSION_H */
