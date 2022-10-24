#include "main.h"
#include <stdlib.h>
#include <limits.h>
#include <math.h>

/*
$GPRMC
        Recommended minimum specific GPS/Transit data

eg1. $GPRMC,081836,A,3751.65,S,14507.36,E,000.0,360.0,130998,011.3,E*62
eg2. $GPRMC,225446,A,4916.45,N,12311.12,W,000.5,054.7,191194,020.3,E*68


225446       Time of fix 22:54:46 UTC
        A            Navigation receiver warning A = OK, V = warning
4916.45,N    Latitude 49 deg. 16.45 min North
12311.12,W   Longitude 123 deg. 11.12 min West
000.5        Speed over ground, Knots
054.7        Course Made Good, True
191194       Date of fix  19 November 1994
020.3,E      Magnetic variation 20.3 deg East
*68          mandatory checksum


eg3. $GPRMC,220516,A,5133.82,N,00042.24,W,173.8,231.8,130694,004.2,W*70
1    2    3    4    5     6    7    8      9     10  11 12


1   220516     Time Stamp
2   A          validity - A-ok, V-invalid
3   5133.82    current Latitude
4   N          North/South
5   00042.24   current Longitude
6   W          East/West
7   173.8      Speed in knots
8   231.8      True course
9   130694     Date Stamp
10  004.2      Variation
11  W          East/West
12  *70        checksum


        eg4. $GPRMC,hhmmss.ss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,ddmmyy,x.x,a*hh
1    = UTC of position fix
2    = Data status (V=navigation receiver warning)
3    = Latitude of fix
4    = N or S
5    = Longitude of fix
6    = E or W
7    = Speed over ground in knots
8    = Track made good in degrees True
9    = UT date
10   = Magnetic variation degrees (Easterly var. subtracts from true course)
11   = E or W
12   = Checksum
 */
struct GpsData_t gpsData;
volatile struct GpsData_t gpsDataXchange;

static double parseCoord(char *ptr, char **endPtr, char validPositive, char validNegative) {
    if(*ptr == ',') {
        *endPtr = ptr+1;
        return HUGE_VAL;
    }
    unsigned long coordDeg = strtoul(ptr, &ptr, 10);
    if (coordDeg == ULONG_MAX) return NAN;
    if (*(ptr) != '.') return NAN;
    double coordMin = strtod(ptr - 2, &ptr);
    if (coordMin == HUGE_VAL || coordMin == -HUGE_VAL) return NAN;
    if (*(ptr++) != ',') return NAN;
    double result = (coordDeg / 100)  // NOLINT(bugprone-integer-division)
                    + coordMin / 60.0;
    if (*ptr == validNegative) {
        result = -result;
    } else if (*ptr != validPositive) {
        return NAN;
    }
    if (ptr[1] != ',') return NAN;
    *endPtr = ptr + 2;
    return result;
}

void parseNmea(char *buf) {
    gpsData.parsed = false;
    gpsData.validTime = false;
    gpsData.valid = false;
    if (strncmp(buf, "$GPRMC,", 7) != 0) {
        return;
    }
    char *ptr = buf + 7;
    unsigned long dateLong = strtoul(ptr, &ptr, 10);
    if (dateLong == 0 || dateLong == ULONG_MAX) {
        gpsData.parsed = true;
        return;
    };
    gpsData.hour = dateLong / 10000;//todo timezone + dst
    gpsData.minute = (dateLong / 100) % 100;
    gpsData.validTime = true;
    if ((ptr = strchr(ptr, ',')) == NULL) return;
    switch (ptr[1]) {
        case 'A':
            gpsData.valid = true;
            break;
        case 'V':
            gpsData.valid = false;
            break;
        default:
            return;
    }
    gpsData.lat = parseCoord(ptr + 3, &ptr, 'N', 'S');
    if (isnan(gpsData.lat)) {
        return;
    }
    gpsData.lon = parseCoord(ptr, &ptr, 'E', 'W');
    if (isnan(gpsData.lon)) {
        return;
    }
    gpsData.speedKts = strtod(ptr, &ptr);
    if (gpsData.speedKts == HUGE_VAL || gpsData.speedKts == -HUGE_VAL) {
        return;
    }
    gpsData.parsed = true;
}

#define BUFF_LEN (256)

_Noreturn void speed_task(void *params) {
    (void) params;
    static char buff[BUFF_LEN];
    int idx = 0;
    while (true) {
        int c = uart_getc(UART_ID);
        if (c == 13 || c == 10) {
            buff[idx] = 0;
            if (idx > 0) {
                parseNmea(buff);
                if (gpsData.parsed) {
                    if(xSemaphoreTake(gpsDataMutex, 5000)) {
                        gpsDataXchange = gpsData;
                        xSemaphoreGive(gpsDataMutex);
                        xTaskNotify(ledTaskHandle, 0, eNoAction);
                    }
                }
            }
            idx = 0;
        } else {
            buff[idx] = c;
            idx = (idx + 1) % BUFF_LEN;
        }
    }
}
