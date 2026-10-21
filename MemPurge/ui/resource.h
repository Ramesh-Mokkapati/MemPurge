// MemPurge - ui/resource.h
#pragma once

// Icons / menus / dialogs
#define IDR_MAINFRAME               128
#define IDR_TRAYMENU                129
#define IDD_MAIN                    130
#define IDD_PAGE_MEMORY             131
#define IDD_PAGE_SETTINGS           132
#define IDD_PAGE_EXCLUSIONS         133

// Main dialog
#define IDC_TAB                     1000

// Memory page
#define IDC_PHYS_TEXT               1010
#define IDC_PHYS_BAR                1011
#define IDC_COMMIT_TEXT             1012
#define IDC_COMMIT_BAR              1013
#define IDC_AREA_WORKINGSET         1020
#define IDC_AREA_SYSTEMFILECACHE    1021
#define IDC_AREA_MODIFIEDPAGELIST   1022
#define IDC_AREA_STANDBYLIST        1023
#define IDC_AREA_STANDBYLOW         1024
#define IDC_AREA_COMBINEDPAGELIST   1025
#define IDC_AREA_REGISTRYCACHE      1026
#define IDC_AREA_MODIFIEDFILECACHE  1027
#define IDC_OPTIMIZE                1030
#define IDC_STATUS                  1031
#define IDC_DETAILS                 1032

// Settings page
#define IDC_INTERVAL_EDIT           1040
#define IDC_INTERVAL_SPIN           1041
#define IDC_LOWMEM_EDIT             1042
#define IDC_LOWMEM_SPIN             1043
#define IDC_HOTKEY_ENABLE           1044
#define IDC_HOTKEY                  1045
#define IDC_HOTKEY_WARNING          1046
#define IDC_RUN_ON_STARTUP          1050
#define IDC_START_MINIMIZED         1051
#define IDC_CLOSE_TO_TRAY           1052
#define IDC_ALWAYS_ON_TOP           1053
#define IDC_SHOW_NOTIFICATIONS      1054
#define IDC_SHOW_VIRTUAL            1055
#define IDC_LOW_PRIORITY            1056
#define IDC_MIDDLE_CLICK            1057
#define IDC_TRAY_USAGE              1058
#define IDC_WARNING_EDIT            1060
#define IDC_WARNING_SPIN            1061
#define IDC_DANGER_EDIT             1062
#define IDC_DANGER_SPIN             1063
#define IDC_RESET                   1064

// Exclusions page
#define IDC_EXCL_LIST               1070
#define IDC_EXCL_COMBO              1071
#define IDC_EXCL_ADD                1072
#define IDC_EXCL_REMOVE             1073

// Tray menu commands
#define ID_TRAY_OPTIMIZE            32771
#define ID_TRAY_SHOW                32772
#define ID_TRAY_EXIT                32773

// Strings
#define IDS_APP_TITLE               2000
#define IDS_TAB_MEMORY              2001
#define IDS_TAB_SETTINGS            2002
#define IDS_TAB_EXCLUSIONS          2003

#define IDS_AREA_FIRST              2010   // + AreaIndex, in execution order
#define IDS_AREA_WORKINGSET         2010
#define IDS_AREA_SYSTEMFILECACHE    2011
#define IDS_AREA_MODIFIEDPAGELIST   2012
#define IDS_AREA_STANDBYLIST        2013
#define IDS_AREA_STANDBYLOW         2014
#define IDS_AREA_COMBINEDPAGELIST   2015
#define IDS_AREA_REGISTRYCACHE      2016
#define IDS_AREA_MODIFIEDFILECACHE  2017

#define IDS_STATUS_SUCCESS          2020
#define IDS_STATUS_NOTSUPPORTED     2021
#define IDS_STATUS_PRIVILEGE        2022
#define IDS_STATUS_FAILED           2023

#define IDS_PHYS_FMT                2030
#define IDS_COMMIT_FMT              2031
#define IDS_OPTIMIZING_FMT          2032
#define IDS_FREED_FMT               2033
#define IDS_DETAILS_TITLE           2034
#define IDS_RESULT_LINE_FMT         2035
#define IDS_RESULT_LINE_ERR_FMT     2036
#define IDS_OS_FMT                  2037
#define IDS_TOOLTIP_FMT             2038
#define IDS_NOTIFY_FMT              2039
#define IDS_READY                   2040

#define IDS_REASON_MANUAL           2050
#define IDS_REASON_HOTKEY           2051
#define IDS_REASON_INTERVAL         2052
#define IDS_REASON_LOWMEMORY        2053
#define IDS_REASON_TRAY             2054

#define IDS_HOTKEY_FAILED           2060
#define IDS_STARTUP_FAILED_FMT      2061
#define IDS_SETTINGS_SAVE_FAILED    2062

#define IDS_CLI_FREED_FMT           2070
#define IDS_USAGE                   2071
#define IDS_UNKNOWN_SWITCH_FMT      2072

#define IDS_UNIT_B                  2080
#define IDS_UNIT_KB                 2081
#define IDS_UNIT_MB                 2082
#define IDS_UNIT_GB                 2083
#define IDS_UNIT_TB                 2084

#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE    134
#define _APS_NEXT_COMMAND_VALUE     32774
#define _APS_NEXT_CONTROL_VALUE     1074
#define _APS_NEXT_SYMED_VALUE       2100
#endif
#endif
