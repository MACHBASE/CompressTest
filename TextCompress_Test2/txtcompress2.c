/******************************************************************************
 * Copyright of this product 2013-2023,
 * MACHBASE Corporation(or Inc.) or its subsidiaries.
 * All Rights reserved.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <machbase_sqlcli.h>
#include <getopt.h>
#include <unistd.h>
#include <stdarg.h>


#define MACHBASE_PORT_NO    5656
//#define ERROR_CHECK_COUNT   100
#define ERROR_CHECK_COUNT   1000000

#define RC_SUCCESS          0
#define RC_FAILURE          -1

int checkAppendError(SQLHENV aEnv, SQLHDBC aCon, SQLHSTMT aStmt);

#define UNUSED(aVar) do { (void)(aVar); } while(0)

#define CHECK_APPEND_RESULT(aRC, aEnv, aCon, aSTMT)             \
    if( !SQL_SUCCEEDED(aRC) )                                   \
    {                                                           \
        if( checkAppendError(aEnv, aCon, aSTMT) == RC_FAILURE ) \
        {                                                       \
            return RC_FAILURE;                                  \
        }                                                       \
    }                                                           \



time_t         sTotalTimeGap;
time_t         sRealTotalTimeGap;


int            gProcessCount    = 10;
//int            gSleepSeconds    = 60;
int            gSleepSeconds    = 1;
int            gCreateTableFlag = 0;
char           gStartDate[8+1] = {0,};
int            gPortNo   = 5656;
char           gServerIp[40+1] = {"127.0.0.1"};
unsigned long  gEPS   = 80000;
double         gApp   = 1;
time_t         gTimeSec = 1000000; //1sec

int            gTotalInputCount = 0;
int            gDebug_flag = 0;
int            isDisableDelay = 0;
int            gRawSize = 8204;

SQLHENV      gEnv;
SQLHDBC      gCon;

void printError(SQLHENV aEnv, SQLHDBC aCon, SQLHSTMT aStmt, char *aMsg);
time_t getTimeStamp(char *dt);
int connectDB();
void disconnectDB();
int executeDirectSQL(const char *aSQL, int aErrIgnore);
int appendOpen(SQLHSTMT aStmt, const char *sTableName);
int appendData(SQLHSTMT aStmt);
SQLBIGINT appendClose(SQLHSTMT aStmt);
void printColumn(char *aCol, int aLen, char *aFormat, ...);


double randomDouble (double min_num, double max_num)
{
    double d;
    d = (double) rand() / ((double) RAND_MAX + 1);
    return (min_num + d * (max_num - min_num));
}

int randomInt (int min_num, int max_num)
{
    int result = 0, low_num = 0, hi_num = 0;

    if (min_num < max_num) {
        low_num = min_num;
        hi_num = max_num + 1; // include max_num in output
    } else {
        low_num = max_num + 1; // include max_num in output
        hi_num = min_num;
    }

    srand(time(NULL));
    //srand(getTimeStamp());
    result = (rand() % (hi_num - low_num)) + low_num;
    return result;
}

int randomInt2 (int min_num, int max_num, int adding)
{
    int result = 0, low_num = 0, hi_num = 0;

    if (min_num < max_num) {
        low_num = min_num;
        hi_num = max_num + 1; // include max_num in output
    } else {
        low_num = max_num + 1; // include max_num in output
        hi_num = min_num;
    }

    srand(time(NULL) + adding);
    //srand(getTimeStamp());
    result = (rand() % (hi_num - low_num)) + low_num;
    return result;
}



int
convert_datetime_to_timestamp (char  *p_format, time_t  *p_timestamp)
{
    int       result = 0;
    struct tm   newtime; 
    char        buf[8];


    memset (&newtime, 0x00, sizeof (newtime));

    memset (buf, 0x00, sizeof (buf));
    strncpy (buf, p_format, 4);
    newtime.tm_year = atoi (buf) - 1900;

    memset (buf, 0x00, sizeof (buf));
    strncpy (buf, p_format + 5, 2);
    newtime.tm_mon = atoi (buf) - 1;

    memset (buf, 0x00, sizeof (buf));
    strncpy (buf, p_format + 8, 2);
    newtime.tm_mday = atoi (buf);

    memset (buf, 0x00, sizeof (buf));
    strncpy (buf, p_format + 11, 2);
    newtime.tm_hour = atoi (buf);

    memset (buf, 0x00, sizeof (buf));
    strncpy (buf, p_format + 14, 2);
    newtime.tm_min = atoi (buf);

    memset (buf, 0x00, sizeof (buf));
    strncpy (buf, p_format + 17, 2);
    newtime.tm_sec = atoi (buf);

    (*p_timestamp)= mktime (&newtime);


    return (result);

}


int
make_timestamp_to_datetime (char    *p_format,
                            time_t   timestamp)
{
    struct tm   tm;
    time_t      tick;


    memset (&tm, 0x00, sizeof (tm));

    tick = (time_t)timestamp;
    localtime_r (&tick, &tm);


    sprintf (p_format,
            "%04d-%02d-%02d %02d:%02d:%02d",
            tm.tm_year+1900,
            tm.tm_mon +1,
            tm.tm_mday,
            tm.tm_hour,
            tm.tm_min,
            tm.tm_sec );

    return (0);
}



int setDBData (SQLHSTMT aStmt, SQL_APPEND_PARAM *sParam , unsigned long long sTime)
{
    SQLRETURN         sRC;
    SQL_APPEND_PARAM  tParam;

    int            idx = 0;
    int            i = 0, j = 0;
    char           tdata[1+1] = { 0, };
    char           *bdata[10];



#ifdef _APPEND_TIMETEST_
    time_t       totalpresetTimeGap = 0;
    time_t       totalappendTimeGap = 0;
    time_t       totalsetTimeGap = 0;
    time_t       appendStartTime = 0, appendEndTime = 0;
    time_t       setStartTime = 0, setEndTime = 0;
    time_t       presetStartTime = 0, presetEndTime = 0;
#endif

    for (i=0; i<10; i++) {
        bdata[i] = (char *)malloc(sizeof(char) * gRawSize);
    }

#ifdef _APPEND_TIMETEST_
        presetStartTime = getTimeStamp(NULL);
#endif
    // Pre set data
    for ( i = 0; i < 10; i++ ) {
        sprintf (bdata[i], "%d", randomInt(0,1));
        for (j = 1; j < gRawSize; j++) {
            memset (tdata, 0x00, sizeof(tdata));
            sprintf (tdata, "%d", randomInt2(0,1,j));
            strcat(bdata[i], tdata);
        }
    }
    /*
    for ( i = 0; i < 2; i++ ) {
        sprintf (bdata[i], "%d", randomInt(0,1));
        for (j = 1; j < 400000; j++) {
            if ((j % 2) == 0)
                strcat(bdata[i], "1");
            else
                strcat(bdata[i], "0");
        }
    }
    */
#ifdef _APPEND_TIMETEST_
        presetEndTime = getTimeStamp(NULL);
        totalpresetTimeGap += presetEndTime - presetStartTime;
#endif


    tParam.mDateTime.mTime = sTime;
    //tParam.mDateTime.mTime = SQL_APPEND_DATETIME_NOW;
    //fprintf (stderr, "sTime:%llu\n", sTime);
    //fprintf (stderr, "mTime:%llu\n", tParam.mDateTime.mTime);
    for (idx = 1; (unsigned long)idx <= gEPS; idx++)
    {
#ifdef _APPEND_TIMETEST_
        setStartTime = getTimeStamp(NULL);
#endif

        //data
        char *textdata =  bdata[ randomInt2(0, 9, idx) ];
        sParam[0].mText.mLength      = strlen(textdata);
        sParam[0].mText.mData        = textdata;
        //fprintf (stderr, "mData : [%s]", sParam[0].mText.mData);


#ifdef _APPEND_TIMETEST_
        setEndTime = getTimeStamp(NULL);
        appendStartTime = getTimeStamp(NULL);
#endif
        //sRC = SQLAppendDataV2(aStmt, sParam);
        //sRC = SQLAppendDataByTimeV2(aStmt, sParam[0].mDateTime.mTime, sParam);
        sRC = SQLAppendDataByTimeV2(aStmt, tParam.mDateTime.mTime, sParam);

#ifdef _APPEND_TIMETEST_
        appendEndTime = getTimeStamp(NULL);
#endif
        CHECK_APPEND_RESULT(sRC, gEnv, gCon, aStmt);

#ifdef _APPEND_TIMETEST_
        totalappendTimeGap += appendEndTime - appendStartTime;
        totalsetTimeGap += setEndTime - setStartTime;
#endif

    }

#ifdef _APPEND_TIMETEST_
    fprintf (stderr, "preset timegap = %7ld\n", totalpresetTimeGap);
    fprintf (stderr, "set    timegap = %7ld\n", totalsetTimeGap);
    fprintf (stderr, "append timegap = %7ld\n", totalappendTimeGap);
#endif

    for (i=0; i<10; i++) {
        free(bdata[i]);
        bdata[i] = NULL;
    }
    return RC_SUCCESS;
}





void printError(SQLHENV aEnv, SQLHDBC aCon, SQLHSTMT aStmt, char *aMsg)
{
    SQLINTEGER      sNativeError;
    SQLCHAR         sErrorMsg[SQL_MAX_MESSAGE_LENGTH + 1];
    SQLCHAR         sSqlState[SQL_SQLSTATE_SIZE + 1];
    SQLSMALLINT     sMsgLength;

    if( aMsg != NULL )
    {
        fprintf(stderr, "%s\n", aMsg);
    }

    if( SQLError(aEnv, aCon, aStmt, sSqlState, &sNativeError,
        sErrorMsg, SQL_MAX_MESSAGE_LENGTH, &sMsgLength) == SQL_SUCCESS )
    {
        fprintf(stderr, "SQLSTATE-[%s], Machbase-[%d][%s]\n", sSqlState, sNativeError, sErrorMsg);
    }
}

int checkAppendError(SQLHENV aEnv, SQLHDBC aCon, SQLHSTMT aStmt)
{
    SQLINTEGER      sNativeError;
    SQLCHAR         sErrorMsg[SQL_MAX_MESSAGE_LENGTH + 1];
    SQLCHAR         sSqlState[SQL_SQLSTATE_SIZE + 1];
    SQLSMALLINT     sMsgLength;

    if( SQLError(aEnv, aCon, aStmt, sSqlState, &sNativeError,
        sErrorMsg, SQL_MAX_MESSAGE_LENGTH, &sMsgLength) != SQL_SUCCESS )
    {
        return RC_FAILURE;
    }

    fprintf(stderr, "SQLSTATE-[%s], Machbase-[%d][%s]\n", sSqlState, sNativeError, sErrorMsg);

    if( sNativeError != 9604 &&
        sNativeError != 9605 &&
        sNativeError != 9606 )
    {
        return RC_FAILURE;
    }

    return RC_SUCCESS;
}

void appendDumpError(SQLHSTMT    aStmt,
                     SQLINTEGER  aErrorCode,
                     SQLPOINTER  aErrorMessage,
                     SQLLEN      aErrorBufLen,
                     SQLPOINTER  aRowBuf,
                     SQLLEN      aRowBufLen)
{
    char       sErrMsg[1024] = {0, };
    char       sRowMsg[32 * 1024] = {0, };

    UNUSED(aStmt);

    if (aErrorMessage != NULL)
    {
        strncpy(sErrMsg, (char *)aErrorMessage, aErrorBufLen);
    }

    if (aRowBuf != NULL)
    {
        strncpy(sRowMsg, (char *)aRowBuf, aRowBufLen);
    }

    fprintf(stderr, "Append Error : [%d][%s]\n[%s]\n\n", aErrorCode, sErrMsg, sRowMsg);
}


void printColumn(char *aCol, int aLen, char *aFormat, ...)
{
    fprintf(stdout, "%s : ", aCol);

    if( aLen == SQL_NULL_DATA )
    {
        fprintf(stdout, "NULL");
    }
    else
    {
        va_list ap;
        va_start(ap, aFormat);
        vfprintf(stdout, aFormat, ap);
        va_end(ap);
    }
}



time_t getTimeStamp(char *dt)
{
    struct timeval sTimeVal;
    struct tm      stm;
    int            sRet;

    sRet = gettimeofday(&sTimeVal, NULL);

    if (dt != NULL) {
        localtime_r(&sTimeVal.tv_sec, &stm);
        sprintf (dt, "%04d-%02d-%02d %02d:%02d:%02d %06ld",
                stm.tm_year + 1900, stm.tm_mon + 1, stm.tm_mday,
                stm.tm_hour, stm.tm_min, stm.tm_sec,
                sTimeVal.tv_usec);
    }

    if (sRet == 0)
    {
        return (time_t)(sTimeVal.tv_sec * 1000000 + sTimeVal.tv_usec);
    }
    else
    {
        return 0;
    }

}

int connectDB()
{
    char sConnStr[1024];

    if( SQLAllocEnv(&gEnv) != SQL_SUCCESS )
    {
        fprintf(stderr, "SQLAllocEnv error\n");
        return RC_FAILURE;
    }

    if( SQLAllocConnect(gEnv, &gCon) != SQL_SUCCESS )
    {
        fprintf(stderr, "SQLAllocConnect error\n");

        SQLFreeEnv(gEnv);
        gEnv = SQL_NULL_HENV;

        return RC_FAILURE;
    }

    fprintf (stderr, "gServerIp : %s, gPortNo : %d\n", gServerIp, gPortNo);
    sprintf(sConnStr,"SERVER=%s;UID=SYS;PWD=MANAGER;CONNTYPE=1;CONNECTION_TIMEOUT=0;PORT_NO=%d", gServerIp, gPortNo);

    if( SQLDriverConnect( gCon, NULL,
                          (SQLCHAR *)sConnStr,
                          SQL_NTS,
                          NULL, 0, NULL,
                          SQL_DRIVER_NOPROMPT ) != SQL_SUCCESS
      )
    {

        printError(gEnv, gCon, NULL, "SQLDriverConnect error");

        SQLFreeConnect(gCon);
        gCon = SQL_NULL_HDBC;

        SQLFreeEnv(gEnv);
        gEnv = SQL_NULL_HENV;

        return RC_FAILURE;
    }

    return RC_SUCCESS;
}


void disconnectDB()
{
    if( SQLDisconnect(gCon) != SQL_SUCCESS )
    {
        printError(gEnv, gCon, NULL, "SQLDisconnect error");
    }

    SQLFreeConnect(gCon);
    gCon = SQL_NULL_HDBC;

    SQLFreeEnv(gEnv);
    gEnv = SQL_NULL_HENV;
}

int executeDirectSQL(const char *aSQL, int aErrIgnore)
{
    SQLHSTMT sStmt = SQL_NULL_HSTMT;

    if( SQLAllocStmt(gCon, &sStmt) != SQL_SUCCESS )
    {
        if( aErrIgnore == 0 )
        {
            printError(gEnv, gCon, sStmt, "SQLAllocStmt Error");
            return RC_FAILURE;
        }
    }

    if( SQLExecDirect(sStmt, (SQLCHAR *)aSQL, SQL_NTS) != SQL_SUCCESS )
    {

        if( aErrIgnore == 0 )
        {
            printError(gEnv, gCon, sStmt, "SQLExecDirect Error");

            SQLFreeStmt(sStmt,SQL_DROP);
            sStmt = SQL_NULL_HSTMT;
            return RC_FAILURE;
        }
    }

    if( SQLFreeStmt(sStmt, SQL_DROP) != SQL_SUCCESS )
    {
        if (aErrIgnore == 0)
        {
            printError(gEnv, gCon, sStmt, "SQLFreeStmt Error");
            sStmt = SQL_NULL_HSTMT;
            return RC_FAILURE;
        }
    }
    sStmt = SQL_NULL_HSTMT;

    return RC_SUCCESS;
}

int createTable()
{
    int sRC;

    sRC = executeDirectSQL("DROP TABLE COMPRESS_TEST", 1);
    if( sRC != RC_SUCCESS )
    {
        return RC_FAILURE;
    }

    sRC = executeDirectSQL( "CREATE TABLE COMPRESS_TEST "
                            "("
                            "DATA  text       "
                            ")", 0);
    if( sRC != RC_SUCCESS )
    {
        return RC_FAILURE;
    }


    return RC_SUCCESS;
}


int appendOpen(SQLHSTMT aStmt, const char *sTableName)
{
    if( SQLAppendOpen(aStmt, (SQLCHAR *)sTableName, ERROR_CHECK_COUNT) != SQL_SUCCESS )
    {
        printError(gEnv, gCon, aStmt, "SQLAppendOpen Error");
        return RC_FAILURE;
    }

    return RC_SUCCESS;
}

int appendData(SQLHSTMT aStmt)
{
    SQL_APPEND_PARAM sParam[1];

    int i = 0;
    time_t        sStartTime, sEndTime;
    time_t        sRealStartTime, sRealEndTime;
    char          cStartDt[30] = {0,}, cEndDt[30] = {0,};
    char          cRealStartDt[30] = {0,}, cRealEndDt[30] = {0,};
    struct tm     sTm;
    unsigned long long sTime;
    char          year[4+1] = {0,}, month[2+1] = {0,}, day[2+1] = {0,};
    unsigned long long sInterval = 1000000000;
    time_t        sleepGap;
    struct timespec ts;
    time_t        tick, utick, stick, mtick;


    strncpy (year,  gStartDate,   4);
    strncpy (month, gStartDate+4, 2);
    strncpy (day,   gStartDate+6, 2);
    sTm.tm_year = atoi(year) - 1900;
    sTm.tm_mon  = atoi(month)-1;
    sTm.tm_mday = atoi(day);
    sTm.tm_hour = 0;
    sTm.tm_min  = 0;
    sTm.tm_sec  = 0;
    sTime = mktime(&sTm);
    sTime = sTime * MACHBASE_UINT64_LITERAL(1000000000);
    sInterval = sInterval * gSleepSeconds;

    //fprintf(stderr, "press any key to continue...");
    //int c = getchar();


    for (i=1; i <= gProcessCount; i++) {

        memset(sParam, 0, sizeof(sParam));

        sStartTime = getTimeStamp(cStartDt);
        {
            setDBData (aStmt, sParam, sTime);

            if( SQLAppendFlush( aStmt ) != SQL_SUCCESS ) {
                printError(gEnv, gCon, aStmt, "SQLAppendFlush Error");
            }
        }
        sEndTime = getTimeStamp(cEndDt);



        if (isDisableDelay == 0) {

            sRealStartTime = sStartTime;
            sRealEndTime   = sEndTime;
            sRealTotalTimeGap += sRealEndTime - sRealStartTime;
            memcpy (cRealStartDt , cStartDt, sizeof(cStartDt));
            memcpy (cRealEndDt   , cEndDt,   sizeof(cEndDt));

            if ( ((sEndTime - sStartTime) < gTimeSec) ) {
                sleepGap = gTimeSec - (sEndTime - sStartTime);

                sTotalTimeGap += ((sEndTime - sStartTime) + sleepGap);
                ts.tv_sec = 0;
                ts.tv_nsec = sleepGap * 1000;
                convert_datetime_to_timestamp (cStartDt, &tick);
                utick = atoi(cStartDt+20);
                utick += ((sEndTime - sStartTime) + sleepGap);
                stick = utick / gTimeSec;
                mtick = utick % gTimeSec;
                tick += stick;
                make_timestamp_to_datetime (cEndDt, tick);
                sprintf (cEndDt, "%s %06ld", cEndDt, mtick);

                nanosleep (&ts, NULL);

                fprintf(stderr, "[%5d time][%s, %s] |TEST|>> timegap = %10ld usec for %7lu records, %10.2f record/seconds(EPS), use-sleep = %10ld usec "
                                          "[%s, %s] |REAL|>> timegap = %10ld usec  %10.2f record/seconds(EPS)\n", 
                       i, cStartDt, cEndDt, (sEndTime - sStartTime) + sleepGap, gEPS, ((double)gEPS/(double)((sEndTime - sStartTime) + sleepGap))*1000000, sleepGap,
                       cRealStartDt, cRealEndDt, sRealEndTime - sRealStartTime, ((double)gEPS/(double)(sRealEndTime - sRealStartTime))*1000000);
            } else {
                sTotalTimeGap += sEndTime - sStartTime;

                fprintf(stderr, "[%5d time][%s, %s] |TEST|>> timegap = %10ld usec for %7lu records, %10.2f record/seconds(EPS), non-sleep = %10ld usec"
                                          "[%s, %s] |REAL|>> timegap = %10ld usec  %10.2f record/seconds(EPS)\n", 
                       i, cStartDt, cEndDt, sEndTime - sStartTime, gEPS, ((double)gEPS/(double)(sEndTime - sStartTime))*1000000, (time_t) 0,
                       cRealStartDt, cRealEndDt, sRealEndTime - sRealStartTime, ((double)gEPS/(double)(sRealEndTime - sRealStartTime))*1000000);
            }
        } else {
            sTotalTimeGap += sEndTime - sStartTime;

            fprintf(stderr, "[%5d time][%s, %s] |TEST|>> timegap = %10ld usec for %7lu records, %10.2f record/seconds(EPS)\n",
                   i, cStartDt, cEndDt, sEndTime - sStartTime, gEPS, ((double)gEPS/(double)(sEndTime - sStartTime))*1000000);
        }


        sTime = sTime + sInterval;

    }

    return RC_SUCCESS;
}


SQLBIGINT appendClose(SQLHSTMT aStmt)
{
    SQLBIGINT sSuccessCount = 0;
    SQLBIGINT sFailureCount = 0;

    if( SQLAppendClose(aStmt, &sSuccessCount, &sFailureCount) != SQL_SUCCESS )
    {
        printError(gEnv, gCon, aStmt, "SQLAppendClose Error");
        return RC_FAILURE;
    }

    fprintf(stderr, "success : %ld, failure : %ld\n", sSuccessCount, sFailureCount);

    return sSuccessCount;
}


int
getOpt (int  argc, char  *argv[])
{
    struct option options[] = {
        {"disable-delay",    0, 0, 0},
        {"raw-size",         1, 0, 0},
        {0, 0, 0, 0}
    };

    int           opt      = 0;
    int           index    = 0;

    unsigned int  err_flag = 0;
    unsigned int  setA_flag = 0;
    unsigned int  setS_flag = 0;


    while ((opt = getopt_long (argc, argv, "s:n:e:a:S:P:cd", options, &index)) != EOF)
    {
        switch (opt)
        {
            case   0:
                switch(index) {
                    case 0:
                        isDisableDelay = 1;
                        break;
                    case 1:
                        gRawSize = atoi(optarg);
                        break;
                }
                break;

            case 's':
                strncpy(gStartDate, optarg, 8);
                setS_flag = 1;
                break;

            case 'n' :
                gProcessCount = atoi (optarg);
                break;

            case 'e' :
                gEPS = atoi (optarg);
                if (gEPS < 1) {
                    err_flag++;
                }
                break;

            case 'a' :
                gApp = atoi (optarg);
                if (gApp < 1 || gApp > 16) {
                    err_flag++;
                }
                setA_flag = 1;
                break;

            case 'S' :
                strcpy(gServerIp, optarg);
                break;

            case 'P' :
                gPortNo = atoi (optarg);
                break;

            case 'c':
                gCreateTableFlag = 1;
                break;

            case 'd':
                gDebug_flag = 1;
                break;

            default :
                err_flag++;
                break;
        }
    } /* end of while */

    if ( setS_flag == 0 ) {
        fprintf(stderr, "You must set s option \n");
        err_flag++;
        goto error;
    }

    if ( setA_flag == 0 ) {
        fprintf(stderr, "You must set a option\n");
        err_flag++;
        goto error;
    }

error:
    if (err_flag)
    {
        fprintf (stderr,"Usage: cmd [-s <startdate[yyyymmdd]>] [-n <total running count>] [-e <Total EPS>] [-a <client id>] [-P <connect port Number>] [-c enable drop&createtable] \n");
        fprintf (stderr,"       \"-s\" Opt (must) start date for input [format:yyyymmdd]\n");
        fprintf (stderr,"       \"-a\" Opt (must) app(client) index(processid) ( 1 <= index <= 16 )  \n");
        fprintf (stderr,"       \"-e\" Opt set EPS ( default : 80,000 )  \n");
        fprintf (stderr,"       \"-n\" Opt total count \n");
        fprintf (stderr,"       \"-c\" Opt enable drop&createtable\n");
        fflush (stderr);

        return (-1);
    }

    return (0);
}


#if 0
int
getOpt (int  argc, char  *argv[])
{
    int           opt      = 0;
    unsigned int  err_flag = 0;
    unsigned int  setA_flag = 0;
    unsigned int  setS_flag = 0;


    while ((opt = getopt (argc, argv, "s:n:e:a:S:P:cd")) != EOF)
    {
        switch (opt)
        {
            case 's':
                strncpy(gStartDate, optarg, 8);
                setS_flag = 1;
                break;

            case 'n' :
                gProcessCount = atoi (optarg);
                break;

            case 'e' :
                gEPS = atoi (optarg);
                if (gEPS < 1) {
                    err_flag++;
                }
                break;

            case 'a' :
                gApp = atoi (optarg);
                if (gApp < 1 || gApp > 16) {
                    err_flag++;
                }
                setA_flag = 1;
                break;

            case 'S' :
                strcpy(gServerIp, optarg);
                break;

            case 'P' :
                gPortNo = atoi (optarg);
                break;

            case 'c':
                gCreateTableFlag = 1;
                break;

            case 'd':
                gDebug_flag = 1;
                break;

            default :
                err_flag++;
                break;
        }
    } /* end of while */

    if ( setS_flag == 0 ) {
        fprintf(stderr, "You must set s option \n");
        err_flag++;
        goto error;
    }

    if ( setA_flag == 0 ) {
        fprintf(stderr, "You must set a option\n");
        err_flag++;
        goto error;
    }

error:
    if (err_flag)
    {
        fprintf (stderr,"Usage: cmd [-s <startdate[yyyymmdd]>] [-n <total running count>] [-e <Total EPS>] [-a <client id>] [-P <connect port Number>] [-c enable drop&createtable] \n");
        fprintf (stderr,"       \"-s\" Opt (must) start date for input [format:yyyymmdd]\n");
        fprintf (stderr,"       \"-a\" Opt (must) app(client) index(processid) ( 1 <= index <= 16 )  \n");
        fprintf (stderr,"       \"-e\" Opt set EPS ( default : 80,000 )  \n");
        fprintf (stderr,"       \"-n\" Opt total count \n");
        fprintf (stderr,"       \"-c\" Opt enable drop&createtable\n");
        fflush (stderr);

        return (-1);
    }

    return (0);
}
#endif


int main(int argc, char *argv[])
{
    SQLHSTMT    sStmt = SQL_NULL_HSTMT;

    SQLBIGINT   sCount=0;
    //time_t      sStartTime, sEndTime;

    

    if (getOpt (argc, argv))
    {
        fprintf(stderr, "getOpt error\n");

        return RC_FAILURE;
    }

    if (strlen(gStartDate) != 8) {
        
        fprintf(stderr, "You should set Start Date [option: -s 20200403]\n");

        return RC_FAILURE;
    }


    if( connectDB() == RC_SUCCESS )
    {
        fprintf(stderr, "connectDB success\n");
    }
    else
    {
        fprintf(stderr, "connectDB failure\n");
        goto error;
    }

    if (gCreateTableFlag) {
        if( createTable() == RC_SUCCESS )
        {
            fprintf(stderr, "createTable success\n");
        }
        else
        {
            fprintf(stderr, "createTable failure\n");
            goto error;
        }
    }

    /*
    if( SQLSetConnectAppendFlush(gCon, 1) != SQL_SUCCESS )
    {
        printError(gEnv, gCon, NULL, "SQLSetConnectAppendFlush Error");
        goto error;
    }
    */

    if( SQLAllocStmt(gCon, &sStmt) != SQL_SUCCESS )
    {
        printError(gEnv, gCon, sStmt, "SQLAllocStmt Error");
        goto error;
    }

    if( appendOpen(sStmt, "COMPRESS_TEST") == RC_SUCCESS )
    {
        fprintf(stderr, "appendOpen success\n");
    }
    else
    {
        fprintf(stderr, "appendOpen failure\n");
        goto error;
    }

    if( SQLAppendSetErrorCallback(sStmt, appendDumpError) != SQL_SUCCESS )
    {
        printError(gEnv, gCon, sStmt, "SQLAppendSetErrorCallback Error");
        goto error;
    }

    /*
	if( SQLSetStmtAppendInterval(sStmt, 2000) != SQL_SUCCESS )
	{
		printError(gEnv, gCon, sStmt, "SQLSetStmtAppendInterval Error");
		goto error;
	}
    */

    appendData(sStmt);

    sCount = appendClose(sStmt);
    if( sCount >= 0 )
    {
        if (isDisableDelay == 0) {
            fprintf(stderr, "appendClose success\n");
            fprintf(stderr, "|TEST|>> total_timegap = %10ld microseconds for %10ld records, ", sTotalTimeGap, sCount);
            fprintf(stderr, "total %10.2f records/second(EPS)\n",  ((double)sCount/(double)(sTotalTimeGap))*1000000);
            fprintf(stderr, "|REAL|>> total_timegap = %10ld microseconds for %10ld records, ", sRealTotalTimeGap, sCount);
            fprintf(stderr, "total %10.2f records/second(EPS)\n",  ((double)sCount/(double)(sRealTotalTimeGap))*1000000);
        } else {
            fprintf(stderr, "|TEST|>> total_timegap = %10ld microseconds for %10ld records, ", sTotalTimeGap, sCount);
            fprintf(stderr, "total %10.2f records/second(EPS)\n",  ((double)sCount/(double)(sTotalTimeGap))*1000000);
        }
    }
    else
    {
        fprintf(stderr, "appendClose failure\n");
    }

    if( SQLFreeStmt(sStmt, SQL_DROP) != SQL_SUCCESS )
    {
        printError(gEnv, gCon, sStmt, "SQLFreeStmt Error");
        goto error;
    }
    sStmt = SQL_NULL_HSTMT;

    disconnectDB();

    return RC_SUCCESS;

error:
    if( sStmt != SQL_NULL_HSTMT )
    {
        SQLFreeStmt(sStmt, SQL_DROP);
        sStmt = SQL_NULL_HSTMT;
    }

    if( gCon != SQL_NULL_HDBC )
    {
        disconnectDB();
    }

    return RC_FAILURE;
}
