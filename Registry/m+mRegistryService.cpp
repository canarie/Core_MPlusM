
//
//  File:       m+mRegistryService.cpp
//
//  Project:    m+m
//
//  Contains:   The class definition for the m+m Registry Service.
//
//  Written by: Norman Jaffe
//
//  Copyright:  (c) 2014 by H Plus Technologies Ltd. and Simon Fraser University.
//
//              All rights reserved. Redistribution and use in source and binary forms, with or
//              without modification, are permitted provided that the following conditions are met:
//                * Redistributions of source code must retain the above copyright notice, this list
//                  of conditions and the following disclaimer.
//                * Redistributions in binary form must reproduce the above copyright notice, this
//                  list of conditions and the following disclaimer in the documentation and / or
//                  other materials provided with the distribution.
//                * Neither the name of the copyright holders nor the names of its contributors may
//                  be used to endorse or promote products derived from this software without
//                  specific prior written permission.
//
//              THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
//              EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//              OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
//              SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//              INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
//              TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
//              BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//              CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//              ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
//              DAMAGE.
//
//  Created:    2014-02-06
//
//--------------------------------------------------------------------------------------------------

#include "m+mRegistryService.hpp"
#include "m+mColumnNameValidator.hpp"
#include "m+mMatchRequestHandler.hpp"
#include "m+mPingRequestHandler.hpp"
#include "m+mRegisterRequestHandler.hpp"
#include "m+mRegistryCheckThread.hpp"
#include "m+mUnregisterRequestHandler.hpp"

#include <m+m/m+mClientChannel.hpp>
#include <m+m/m+mRequests.hpp>
#include <m+m/m+mServiceRequest.hpp>
#include <m+m/m+mUtilities.hpp>

//#include <odlEnable.h>
#include <odlInclude.h>

#include <ctime>
#if defined(__APPLE__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wc++11-long-long"
#endif // defined(__APPLE__)
#include "sqlite3.h"
#if defined(__APPLE__)
# pragma clang diagnostic pop
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wunknown-pragmas"
# pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#endif // defined(__APPLE__)
/*! @file
 @brief The class definition for the m+m %Registry Service. */
#if defined(__APPLE__)
# pragma clang diagnostic pop
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Namespace references
#endif // defined(__APPLE__)

using namespace MplusM;
using namespace MplusM::Common;
using namespace MplusM::Parser;
using namespace MplusM::Registry;

#if defined(__APPLE__)
# pragma mark Private structures, constants and variables
#endif // defined(__APPLE__)

/*! @brief The name of the index for the 'channelname' column of the 'Requests' table. */
#define REQUESTS_CHANNELNAME_I_         "Requests_channelname_idx"

/*! @brief The name of the index for the 'requests' column of the 'Requests' table. */
#define REQUESTS_REQUEST_I_             "Requests_request_idx"

/*! @brief The name of the index for the 'keywords_id' column of the 'RequestsKeywords' table. */
#define REQUESTSKEYWORDS_KEYWORDS_ID_I_ "RequestsKeywords_Keywords_id_idx"

/*! @brief The name of the index for the 'requests_id' column of the 'RequestsKeywords' table. */
#define REQUESTSKEYWORDS_REQUESTS_ID_I_ "RequestsKeywords_Requests_id_idx"

/*! @brief The name of the index for the 'name' column of the 'Services' table. */
#define SERVICES_NAME_I_                "Services_name_idx"

/*! @brief A shortcut for the standard format for a 'Text' column. */
#define TEXTNOTNULL_                    "Text NOT NULL DEFAULT _"

/*! @brief A shortcut for the case-sensitive form of a 'Text' column. */
#define BINARY_                         "COLLATE BINARY"

/*! @brief A shortcut for the case-insensitive form of a 'Text' column. */
#define NOCASE_                         "COLLATE NOCASE"

namespace MplusM
{
    namespace Registry
    {
        /*! @brief A function that provides bindings for parameters in an SQL statement.
         @param[in] statement The prepared statement that is to be updated.
         @param[in] stuff The source of data that is to be bound.
         @returns The SQLite error from the bind operation. */
        typedef int (*BindFunction)
            (sqlite3_stmt * statement,
            const void *    stuff);

        /*! @brief The data needed to add a request-keyword entry into the database. */
        struct RequestKeywordData
        {
            /*! @brief The name of the request. */
            YarpString _request;

            /*! @brief The service channel for the request. */
            YarpString _channel;

            /*! @brief A keyword for the request. */
            YarpString _key;

        }; // RequestKeywordData

        /*! @brief The data needed to add a service entry into the database. */
        struct ServiceData
        {
            /*! @brief The service channel for the service. */
            YarpString _channel;

            /*! @brief The description of the service. */
            YarpString _description;

            /*! @brief The path to the executable for the service. */
            YarpString _executable;

            /*! @brief The extra information for the service. */
            YarpString _extraInfo;

            /*! @brief The name of the service. */
            YarpString _name;

            /*! @brief The description of the requests for the service. */
            YarpString _requestsDescription;

            /*! @brief The tag modifier for the service. */
            YarpString _tag;

        }; // ServiceData

    } // Registry

} // MplusM

#if defined(__APPLE__)
# pragma mark Global constants and variables
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Local functions
#endif // defined(__APPLE__)

#if defined(ODL_ENABLE_LOGGING_)
/*! @brief Provide a symbolic name for an SQL status value.
 @param[in] sqlRes The status value to be checked.
 @returns A string representing the symbolic name for the status value. */
static const char *
mapStatusToStringForSQL(const int sqlRes)
{
    const char * result;

    switch (sqlRes)
    {
        case SQLITE_OK :
            result = "SQLITE_OK";
            break;

        case SQLITE_ERROR :
            result = "SQLITE_ERROR";
            break;

        case SQLITE_INTERNAL :
            result = "SQLITE_INTERNAL";
            break;

        case SQLITE_PERM :
            result = "SQLITE_PERM";
            break;

        case SQLITE_ABORT :
            result = "SQLITE_ABORT";
            break;

        case SQLITE_BUSY :
            result = "SQLITE_BUSY";
            break;

        case SQLITE_LOCKED :
            result = "SQLITE_LOCKED";
            break;

        case SQLITE_NOMEM :
            result = "SQLITE_NOMEM";
            break;

        case SQLITE_READONLY :
            result = "SQLITE_READONLY";
            break;

        case SQLITE_INTERRUPT :
            result = "SQLITE_INTERRUPT";
            break;

        case SQLITE_IOERR :
            result = "SQLITE_IOERR";
            break;

        case SQLITE_CORRUPT :
            result = "SQLITE_CORRUPT";
            break;

        case SQLITE_NOTFOUND :
            result = "SQLITE_NOTFOUND";
            break;

        case SQLITE_FULL :
            result = "SQLITE_FULL";
            break;

        case SQLITE_CANTOPEN :
            result = "SQLITE_CANTOPEN";
            break;

        case SQLITE_PROTOCOL :
            result = "SQLITE_PROTOCOL";
            break;

        case SQLITE_EMPTY :
            result = "SQLITE_EMPTY";
            break;

        case SQLITE_SCHEMA :
            result = "SQLITE_SCHEMA";
            break;

        case SQLITE_TOOBIG :
            result = "SQLITE_TOOBIG";
            break;

        case SQLITE_CONSTRAINT :
            result = "SQLITE_CONSTRAINT";
            break;

        case SQLITE_MISMATCH :
            result = "SQLITE_MISMATCH";
            break;

        case SQLITE_MISUSE :
            result = "SQLITE_MISUSE";
            break;

        case SQLITE_NOLFS :
            result = "SQLITE_NOLFS";
            break;

        case SQLITE_AUTH :
            result = "SQLITE_AUTH";
            break;

        case SQLITE_FORMAT :
            result = "SQLITE_FORMAT";
            break;

        case SQLITE_RANGE :
            result = "SQLITE_RANGE";
            break;

        case SQLITE_NOTADB :
            result = "SQLITE_NOTADB";
            break;

        case SQLITE_NOTICE :
            result = "SQLITE_NOTICE";
            break;

        case SQLITE_WARNING :
            result = "SQLITE_WARNING";
            break;

        case SQLITE_ROW :
            result = "SQLITE_ROW";
            break;

        case SQLITE_DONE :
            result = "SQLITE_DONE";
            break;

        default :
            result = "<Unknown>";
            break;

    }
    return result;
} // mapStatusToStringForSQL
#endif // defined(ODL_ENABLE_LOGGING_)

/*! @brief Perform a simple operation on the database.
 @param[in] database The database to be modified.
 @param[in] sqlStatement The operation to be performed.
 @param[in] doBinds A function that will fill in any parameters in the statement.
 @param[in] data The custom information used with the binding function.
 @returns @c true if the operation was successfully performed and @c false otherwise. */
static bool
performSQLstatementWithNoResults(sqlite3 *    database,
                                 const char * sqlStatement,
                                 BindFunction doBinds,
                                 const void * data)
{
    ODL_ENTER(); //####
    ODL_P2("database = ", database, "data = ", data); //####
    ODL_S1("sqlStatement = ", sqlStatement); //####
    bool okSoFar = true;

    try
    {
        if (database)
        {
            sqlite3_stmt * prepared = NULL;
            int            sqlRes = sqlite3_prepare_v2(database, sqlStatement,
                                                       static_cast<int>(strlen(sqlStatement)),
                                                       &prepared, NULL);

            ODL_LL1("sqlRes <- ", sqlRes); //####
            ODL_S1("sqlRes <- ", mapStatusToStringForSQL(sqlRes)); //####
            if ((SQLITE_OK == sqlRes) && prepared)
            {
                if (doBinds)
                {
                    sqlRes = doBinds(prepared, data);
                    ODL_LL1("sqlRes <- ", sqlRes); //####
                    ODL_S1("sqlRes <- ", mapStatusToStringForSQL(sqlRes)); //####
                    okSoFar = (SQLITE_OK == sqlRes);
                }
                if (okSoFar)
                {
                    do
                    {
                        sqlRes = sqlite3_step(prepared);
                        ODL_LL1("sqlRes <- ", sqlRes); //####
                        ODL_S1("sqlRes <- ", mapStatusToStringForSQL(sqlRes)); //####
                        if (SQLITE_BUSY == sqlRes)
                        {
                            ConsumeSomeTime(10.0);
                        }
                    }
                    while (SQLITE_BUSY == sqlRes);
                    if (SQLITE_DONE != sqlRes)
                    {
                        ODL_LOG("(SQLITE_DONE != sqlRes)"); //####
                        okSoFar = false;
                    }
                }
                sqlite3_finalize(prepared);
            }
            else
            {
                ODL_LOG("! ((SQLITE_OK == sqlRes) && prepared)"); //####
                okSoFar = false;
            }
        }
        else
        {
            ODL_LOG("! (database)"); //####
            okSoFar = false;
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_EXIT_B(okSoFar); //####
    return okSoFar;
} // performSQLstatementWithNoResults

/*! @brief Perform a simple operation on the database.
 @param[in] database The database to be modified.
 @param[in] sqlStatement The operation to be performed.
 @returns @c true if the operation was successfully performed and @c false otherwise. */
static bool
performSQLstatementWithNoResultsNoArgs(sqlite3 *    database,
                                       const char * sqlStatement)
{
    ODL_ENTER(); //####
    ODL_P1("database = ", database); //####
    ODL_S1("sqlStatement = ", sqlStatement); //####
    bool okSoFar = true;

    try
    {
        if (database)
        {
            sqlite3_stmt * prepared = NULL;
            int            sqlRes = sqlite3_prepare_v2(database, sqlStatement,
                                                       static_cast<int>(strlen(sqlStatement)),
                                                       &prepared, NULL);

            ODL_LL1("sqlRes <- ", sqlRes); //####
            ODL_S1("sqlRes <- ", mapStatusToStringForSQL(sqlRes)); //####
            if ((SQLITE_OK == sqlRes) && prepared)
            {
                if (okSoFar)
                {
                    do
                    {
                        sqlRes = sqlite3_step(prepared);
                        ODL_LL1("sqlRes <- ", sqlRes); //####
                        ODL_S1("sqlRes <- ", mapStatusToStringForSQL(sqlRes)); //####
                        if (SQLITE_BUSY == sqlRes)
                        {
                            ConsumeSomeTime(10.0);
                        }
                    }
                    while (SQLITE_BUSY == sqlRes);
                    if (SQLITE_DONE != sqlRes)
                    {
                        ODL_LOG("(SQLITE_DONE != sqlRes)"); //####
                        okSoFar = false;
                    }
                }
                sqlite3_finalize(prepared);
            }
            else
            {
                ODL_LOG("! ((SQLITE_OK == sqlRes) && prepared)"); //####
                okSoFar = false;
            }
        }
        else
        {
            ODL_LOG("! (database)"); //####
            okSoFar = false;
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_EXIT_B(okSoFar); //####
    return okSoFar;
} // performSQLstatementWithNoResultsNoArgs

/*! @brief Perform a simple operation on the database, ignoring constraint errors.
 @param[in] database The database to be modified.
 @param[in] sqlStatement The operation to be performed.
 @param[in] doBinds A function that will fill in any parameters in the statement.
 @param[in] data The custom information used with the binding function.
 @returns @c true if the operation was successfully performed and @c false otherwise. */
static bool
performSQLstatementWithNoResultsAllowConstraint(sqlite3 *    database,
                                                const char * sqlStatement,
                                                BindFunction doBinds,
                                                const void * data)
{
    ODL_ENTER(); //####
    ODL_P2("database = ", database, "data = ", data); //####
    ODL_S1("sqlStatement = ", sqlStatement); //####
    bool okSoFar = true;

    try
    {
        if (database)
        {
            sqlite3_stmt * prepared = NULL;
            int            sqlRes = sqlite3_prepare_v2(database, sqlStatement,
                                                       static_cast<int>(strlen(sqlStatement)),
                                                       &prepared, NULL);

            ODL_LL1("sqlRes <- ", sqlRes); //####
            ODL_S1("sqlRes <- ", mapStatusToStringForSQL(sqlRes)); //####
            if ((SQLITE_OK == sqlRes) && prepared)
            {
                if (doBinds)
                {
                    sqlRes = doBinds(prepared, data);
                    ODL_LL1("sqlRes <- ", sqlRes); //####
                    ODL_S1("sqlRes <- ", mapStatusToStringForSQL(sqlRes)); //####
                    okSoFar = (SQLITE_OK == sqlRes);
                }
                if (okSoFar)
                {
                    do
                    {
                        sqlRes = sqlite3_step(prepared);
                        ODL_LL1("sqlRes <- ", sqlRes); //####
                        ODL_S1("sqlRes <- ", mapStatusToStringForSQL(sqlRes)); //####
                        if (SQLITE_BUSY == sqlRes)
                        {
                            ConsumeSomeTime(10.0);
                        }
                    }
                    while (SQLITE_BUSY == sqlRes);
                    if ((SQLITE_CONSTRAINT != sqlRes) && (SQLITE_DONE != sqlRes))
                    {
                        ODL_LOG("((SQLITE_CONSTRAINT != sqlRes) && " //####
                                "(SQLITE_DONE != sqlRes))"); //####
                        okSoFar = false;
                    }
                }
                sqlite3_finalize(prepared);
            }
            else
            {
                ODL_LOG("! ((SQLITE_OK == sqlRes) && prepared)"); //####
                okSoFar = false;
            }
        }
        else
        {
            ODL_LOG("! (database)"); //####
            okSoFar = false;
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_EXIT_B(okSoFar); //####
    return okSoFar;
} // performSQLstatementWithNoResultsAllowConstraint

/*! @brief Perform an operation that can return multiple rows of results.
 @param[in] database The database to be modified.
 @param[in,out] resultList The list to be filled in with the values from the column of interest.
 @param[in] sqlStatement The operation to be performed.
 @param[in] columnOfInterest1 The column containing the first value of interest.
 @param[in] columnOfInterest2 The column containing the second value of interest.
 @param[in] doBinds A function that will fill in any parameters in the statement.
 @param[in] data The custom information used with the binding function.
 @returns @c true if the operation was successfully performed and @c false otherwise. */
static bool
performSQLstatementWithDoubleColumnResults(sqlite3 *          database,
                                           yarp::os::Bottle & resultList,
                                           const char *       sqlStatement,
                                           const int          columnOfInterest1,
                                           const int          columnOfInterest2,
                                           BindFunction       doBinds,
                                           const void *       data)
{
    ODL_ENTER(); //####
    ODL_P3("database = ", database, "resultList = ", &resultList, "data = ", data); //####
    ODL_LL2("columnOfInterest1 = ", columnOfInterest1, "columnOfInterest2 = ", //####
            columnOfInterest2); //####
    ODL_S1("sqlStatement = ", sqlStatement); //####
    bool okSoFar = true;

    try
    {
        if (database && (0 <= columnOfInterest1) && (0 <= columnOfInterest2))
        {
            sqlite3_stmt * prepared = NULL;
            int            sqlRes = sqlite3_prepare_v2(database, sqlStatement,
                                                       static_cast<int>(strlen(sqlStatement)),
                                                       &prepared, NULL);

            ODL_LL1("sqlRes <- ", sqlRes); //####
            ODL_S1("sqlRes <- ", mapStatusToStringForSQL(sqlRes)); //####
            if ((SQLITE_OK == sqlRes) && prepared)
            {
                if (doBinds)
                {
                    sqlRes = doBinds(prepared, data);
                    ODL_LL1("sqlRes <- ", sqlRes); //####
                    ODL_S1("sqlRes <- ", mapStatusToStringForSQL(sqlRes)); //####
                    okSoFar = (SQLITE_OK == sqlRes);
                }
                if (okSoFar)
                {
                    for (sqlRes = SQLITE_ROW; SQLITE_ROW == sqlRes; )
                    {
                        do
                        {
                            sqlRes = sqlite3_step(prepared);
                            ODL_LL1("sqlRes <- ", sqlRes); //####
                            ODL_S1("sqlRes <- ", mapStatusToStringForSQL(sqlRes)); //####
                            if (SQLITE_BUSY == sqlRes)
                            {
                                ConsumeSomeTime(10.0);
                            }
                        }
                        while (SQLITE_BUSY == sqlRes);
                        if (SQLITE_ROW == sqlRes)
                        {
                            // Gather the column data...
                            int colCount = sqlite3_column_count(prepared);

                            ODL_LL1("colCount <- ", colCount); //####
                            if ((0 < colCount) && (columnOfInterest1 < colCount) &&
                                (columnOfInterest2 < colCount))
                            {
                                yarp::os::Bottle & subList = resultList.addList();
                                const char *       value =
                                    reinterpret_cast<const char *>(sqlite3_column_text(prepared,
                                                                                columnOfInterest1));

                                ODL_S1("value <- ", value); //####
                                if (value)
                                {
                                    subList.addString(value);
                                }
                                value =
                                    reinterpret_cast<const char *>(sqlite3_column_text(prepared,
                                                                                columnOfInterest2));
                                ODL_S1("value <- ", value); //####
                                if (value)
                                {
                                    subList.addString(value);
                                }
                            }
                        }
                    }
                    if (SQLITE_DONE != sqlRes)
                    {
                        ODL_LOG("(SQLITE_DONE != sqlRes)"); //####
                        okSoFar = false;
                    }
                }
                sqlite3_finalize(prepared);
            }
            else
            {
                ODL_LOG("! ((SQLITE_OK == sqlRes) && prepared)"); //####
                okSoFar = false;
            }
        }
        else
        {
            ODL_LOG("! (database && (0 <= columnOfInterest1) && (0 <= columnOfInterest2))"); //####
            okSoFar = false;
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }

    ODL_EXIT_B(okSoFar); //####
    return okSoFar;
} // performSQLstatementWithDoubleColumnResults

/*! @brief Perform an operation that can return multiple rows of results.
 @param[in] database The database to be modified.
 @param[in,out] resultList The list to be filled in with the values from the column of interest.
 @param[in] sqlStatement The operation to be performed.
 @param[in] columnOfInterest The column containing the value of interest.
 @param[in] doBinds A function that will fill in any parameters in the statement.
 @param[in] data The custom information used with the binding function.
 @returns @c true if the operation was successfully performed and @c false otherwise. */
static bool
performSQLstatementWithSingleColumnResults(sqlite3 *          database,
                                           yarp::os::Bottle & resultList,
                                           const char *       sqlStatement,
                                           const int          columnOfInterest = 0,
                                           BindFunction       doBinds = NULL,
                                           const void *       data = NULL)
{
    ODL_ENTER(); //####
    ODL_P3("database = ", database, "resultList = ", &resultList, "data = ", data); //####
    ODL_LL1("columnOfInterest = ", columnOfInterest); //####
    ODL_S1("sqlStatement = ", sqlStatement); //####
    bool okSoFar = true;

    try
    {
        if (database && (0 <= columnOfInterest))
        {
            sqlite3_stmt * prepared = NULL;
            int            sqlRes = sqlite3_prepare_v2(database, sqlStatement,
                                                       static_cast<int>(strlen(sqlStatement)),
                                                       &prepared, NULL);

            ODL_LL1("sqlRes <- ", sqlRes); //####
            ODL_S1("sqlRes <- ", mapStatusToStringForSQL(sqlRes)); //####
            if ((SQLITE_OK == sqlRes) && prepared)
            {
                if (doBinds)
                {
                    sqlRes = doBinds(prepared, data);
                    ODL_LL1("sqlRes <- ", sqlRes); //####
                    ODL_S1("sqlRes <- ", mapStatusToStringForSQL(sqlRes)); //####
                    okSoFar = (SQLITE_OK == sqlRes);
                }
                if (okSoFar)
                {
                    for (sqlRes = SQLITE_ROW; SQLITE_ROW == sqlRes; )
                    {
                        do
                        {
                            sqlRes = sqlite3_step(prepared);
                            ODL_LL1("sqlRes <- ", sqlRes); //####
                            ODL_S1("sqlRes <- ", mapStatusToStringForSQL(sqlRes)); //####
                            if (SQLITE_BUSY == sqlRes)
                            {
                                ConsumeSomeTime(10.0);
                            }
                        }
                        while (SQLITE_BUSY == sqlRes);
                        if (SQLITE_ROW == sqlRes)
                        {
                            // Gather the column data...
                            int colCount = sqlite3_column_count(prepared);

                            ODL_LL1("colCount <- ", colCount); //####
                            if ((0 < colCount) && (columnOfInterest < colCount))
                            {
                                const char * value =
                                    reinterpret_cast<const char *>(sqlite3_column_text(prepared,
                                                                                columnOfInterest));

                                ODL_S1("value <- ", value); //####
                                if (value)
                                {
                                    resultList.addString(value);
                                }
                            }
                        }
                    }
                    if (SQLITE_DONE != sqlRes)
                    {
                        ODL_LOG("(SQLITE_DONE != sqlRes)"); //####
                        okSoFar = false;
                    }
                }
                sqlite3_finalize(prepared);
            }
            else
            {
                ODL_LOG("! ((SQLITE_OK == sqlRes) && prepared)"); //####
                okSoFar = false;
            }
        }
        else
        {
            ODL_LOG("! (database && (0 <= columnOfInterest))"); //####
            okSoFar = false;
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }

    ODL_EXIT_B(okSoFar); //####
    return okSoFar;
} // performSQLstatementWithSingleColumnResults

/*! @brief Start a transaction.
 @param[in] database The database to be modified.
 @returns @c true if the transaction was initiated and @c false otherwise. */
static bool
doBeginTransaction(sqlite3 * database)
{
    ODL_ENTER(); //####
    ODL_P1("database = ", database); //####
    bool okSoFar = true;

    try
    {
        if (database)
        {
            static const char * beginTransaction = "BEGIN TRANSACTION";

            okSoFar = performSQLstatementWithNoResultsNoArgs(database, beginTransaction);
        }
        else
        {
            ODL_LOG("! (database)"); //####
            okSoFar = false;
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_EXIT_B(okSoFar); //####
    return okSoFar;
} // doBeginTransaction

/*! @brief End a transaction.
 @param[in] database The database to be modified.
 @param[in] wasOK @c true if the transaction was successful and @c false otherwise.
 @returns @c true if the transaction was closed successfully and @c false otherwise. */
static bool
doEndTransaction(sqlite3 *  database,
                 const bool wasOK)
{
    ODL_ENTER(); //####
    ODL_P1("database = ", database); //####
    ODL_B1("wasOK = ", wasOK); //####
    bool okSoFar = true;

    try
    {
        if (database)
        {
            static const char * abortTransaction = "ROLLBACK TRANSACTION";
            static const char * commitTransaction = "END TRANSACTION";

            okSoFar = performSQLstatementWithNoResultsNoArgs(database, wasOK ? commitTransaction :
                                                             abortTransaction);
        }
        else
        {
            ODL_LOG("! (database)"); //####
            okSoFar = false;
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_EXIT_B(okSoFar); //####
    return okSoFar;
} // doCommitTransaction

/*! @brief Construct the tables needed in the database.
 @param[in] database The database to be modified.
 @returns @c true if the tables were successfully built and @c false otherwise. */
static bool
constructTables(sqlite3 * database)
{
    ODL_ENTER(); //####
    ODL_P1("database = ", database); //####
    bool okSoFar = true;

    try
    {
        if (database)
        {
            if (doBeginTransaction(database))
            {
                static const char * tableSQL[] =
                {
#if defined(MpM_UseTestDatabase)
                    T_("DROP INDEX IF EXISTS " REQUESTS_REQUEST_I_),
                    T_("DROP INDEX IF EXISTS " REQUESTS_CHANNELNAME_I_),
                    T_("DROP INDEX IF EXISTS " REQUESTSKEYWORDS_KEYWORDS_ID_I_),
                    T_("DROP INDEX IF EXISTS " REQUESTSKEYWORDS_REQUESTS_ID_I_),
                    T_("DROP INDEX IF EXISTS " SERVICES_NAME_I_),
                    T_("DROP TABLE IF EXISTS " REQUESTSKEYWORDS_T_),
                    T_("DROP TABLE IF EXISTS " REQUESTS_T_),
                    T_("DROP TABLE IF EXISTS " KEYWORDS_T_),
                    T_("DROP TABLE IF EXISTS " SERVICES_T_),
#endif // defined(MpM_UseTestDatabase)
                    T_("CREATE TABLE IF NOT EXISTS " SERVICES_T_ "( " CHANNELNAME_C_ " "
                       TEXTNOTNULL_ " " BINARY_ " PRIMARY KEY ON CONFLICT REPLACE, " NAME_C_ " "
                       TEXTNOTNULL_ " " NOCASE_ ", " DESCRIPTION_C_ " " TEXTNOTNULL_ " " NOCASE_
                       ", " EXECUTABLE_C_ " " TEXTNOTNULL_ " " NOCASE_ ", " EXTRAINFO_C_ " Text "
                       NOCASE_ ", " REQUESTSDESCRIPTION_C_ " " TEXTNOTNULL_ " " NOCASE_ ", "
                       TAG_C_ " " TEXTNOTNULL_ " " BINARY_ ")"),
                    T_("CREATE INDEX IF NOT EXISTS " SERVICES_NAME_I_ " ON " SERVICES_T_ "("
                       NAME_C_ ")"),
                    T_("CREATE TABLE IF NOT EXISTS " KEYWORDS_T_ "( " KEYWORD_C_ " " TEXTNOTNULL_
                       " " BINARY_ " PRIMARY KEY ON CONFLICT IGNORE)"),
                    T_("CREATE TABLE IF NOT EXISTS " REQUESTS_T_ "( " CHANNELNAME_C_ " "
                       TEXTNOTNULL_ " " BINARY_ " REFERENCES " SERVICES_T_ "(" CHANNELNAME_C_ "), "
                       REQUEST_C_ " " TEXTNOTNULL_ " " BINARY_ ", " INPUT_C_ " Text " BINARY_ ", "
                       OUTPUT_C_ " Text " BINARY_ ", " VERSION_C_ " Text " NOCASE_ ", " DETAILS_C_
                       " Text " NOCASE_ ", " KEY_C_ " Integer PRIMARY KEY)"),
                    T_("CREATE INDEX IF NOT EXISTS " REQUESTS_REQUEST_I_ " ON " REQUESTS_T_ "("
                       REQUEST_C_ ")"),
                    T_("CREATE INDEX IF NOT EXISTS " REQUESTS_CHANNELNAME_I_ " ON " REQUESTS_T_ "("
                       CHANNELNAME_C_ ")"),
                    T_("CREATE TABLE IF NOT EXISTS " REQUESTSKEYWORDS_T_ "( " KEYWORDS_ID_C_
                       " Text " BINARY_ " REFERENCES " KEYWORDS_T_ "(" KEYWORD_C_ "), "
                       REQUESTS_ID_C_ " Integer REFERENCES " REQUESTS_T_ "(" KEY_C_ "))"),
                    T_("CREATE INDEX IF NOT EXISTS " REQUESTSKEYWORDS_KEYWORDS_ID_I_ " ON "
                       REQUESTSKEYWORDS_T_ "(" KEYWORDS_ID_C_ ")"),
                    T_("CREATE INDEX IF NOT EXISTS " REQUESTSKEYWORDS_REQUESTS_ID_I_ " ON "
                       REQUESTSKEYWORDS_T_ "(" REQUESTS_ID_C_ ")"),
                };
                static const size_t numTables = (sizeof(tableSQL) / sizeof(*tableSQL));

                okSoFar = true;
                for (size_t ii = 0; okSoFar && (ii < numTables); ++ii)
                {
                    okSoFar = performSQLstatementWithNoResultsNoArgs(database, tableSQL[ii]);
                }
                okSoFar = doEndTransaction(database, okSoFar);
            }
        }
        else
        {
            ODL_LOG("! (database)"); //####
            okSoFar = false;
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_EXIT_B(okSoFar); //####
    return okSoFar;
} // constructTables

/*! @brief Bind the values that are to be gathered from the Services table.
 @param[in] statement The prepared statement that is to be updated.
 @param[in] stuff The source of data that is to be bound.
 @returns The SQLite error from the bind operation. */
static int
setupCheckService(sqlite3_stmt * statement,
                  const void *   stuff)
{
    ODL_ENTER(); //####
    ODL_P2("statement = ", statement, "stuff = ", stuff); //####
    int result = SQLITE_MISUSE;

    try
    {
        int channelNameIndex = sqlite3_bind_parameter_index(statement, "@" CHANNELNAME_C_);

        if (0 < channelNameIndex)
        {
            const char * channelNameString = static_cast<const char *>(stuff);

            ODL_S1("channelNameString <- ", channelNameString); //####
            result = sqlite3_bind_text(statement, channelNameIndex, channelNameString,
                                       static_cast<int>(strlen(channelNameString)),
                                       SQLITE_TRANSIENT);
            if (SQLITE_OK != result)
            {
                ODL_S1("error description: ", sqlite3_errstr(result)); //####
            }
        }
        else
        {
            ODL_LOG("! (0 < channelNameIndex)"); //####
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_EXIT_LL(result);
    return result;
} // setupCheckService

/*! @brief Bind the values that are to be inserted into the Channels table.
 @param[in] statement The prepared statement that is to be updated.
 @param[in] stuff The source of data that is to be bound.
 @returns The SQLite error from the bind operation. */
static int
setupInsertIntoChannels(sqlite3_stmt * statement,
                        const void *   stuff)
{
    ODL_ENTER(); //####
    ODL_P2("statement = ", statement, "stuff = ", stuff); //####
    int result = SQLITE_MISUSE;

    try
    {
        int channelIndex = sqlite3_bind_parameter_index(statement, "@" CHANNELNAME_C_);

        if (0 < channelIndex)
        {
            const char * channelString = static_cast<const char *>(stuff);

            ODL_S1("channelString <- ", channelString); //####
            result = sqlite3_bind_text(statement, channelIndex, channelString,
                                       static_cast<int>(strlen(channelString)), SQLITE_TRANSIENT);
            if (SQLITE_OK != result)
            {
                ODL_S1("error description: ", sqlite3_errstr(result)); //####
            }
        }
        else
        {
            ODL_LOG("! (0 < channelIndex)"); //####
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_EXIT_LL(result);
    return result;
} // setupInsertIntoChannels

/*! @brief Bind the values that are to be inserted into the Keywords table.
 @param[in] statement The prepared statement that is to be updated.
 @param[in] stuff The source of data that is to be bound.
 @returns The SQLite error from the bind operation. */
static int
setupInsertIntoKeywords(sqlite3_stmt * statement,
                        const void *   stuff)
{
    ODL_ENTER(); //####
    ODL_P2("statement = ", statement, "stuff = ", stuff); //####
    int result = SQLITE_MISUSE;

    try
    {
        int keywordIndex = sqlite3_bind_parameter_index(statement, "@" KEYWORD_C_);

        if (0 < keywordIndex)
        {
            const char * nameString = static_cast<const char *>(stuff);

            ODL_S1("nameString <- ", nameString); //####
            result = sqlite3_bind_text(statement, keywordIndex, nameString,
                                       static_cast<int>(strlen(nameString)), SQLITE_TRANSIENT);
            if (SQLITE_OK != result)
            {
                ODL_S1("error description: ", sqlite3_errstr(result)); //####
            }
        }
        else
        {
            ODL_LOG("! (0 < keywordIndex)"); //####
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_EXIT_LL(result);
    return result;
} // setupInsertIntoKeywords

/*! @brief Bind the values that are to be inserted into the Requests table.
 @param[in] statement The prepared statement that is to be updated.
 @param[in] stuff The source of data that is to be bound.
 @returns The SQLite error from the bind operation. */
static int
setupInsertIntoRequests(sqlite3_stmt * statement,
                        const void *   stuff)
{
    ODL_ENTER(); //####
    ODL_P2("statement = ", statement, "stuff = ", stuff); //####
    int result = SQLITE_MISUSE;

    try
    {
        int channelNameIndex = sqlite3_bind_parameter_index(statement, "@" CHANNELNAME_C_);
        int detailsIndex = sqlite3_bind_parameter_index(statement, "@" DETAILS_C_);
        int inputIndex = sqlite3_bind_parameter_index(statement, "@" INPUT_C_);
        int outputIndex = sqlite3_bind_parameter_index(statement, "@" OUTPUT_C_);
        int requestIndex = sqlite3_bind_parameter_index(statement, "@" REQUEST_C_);
        int versionIndex = sqlite3_bind_parameter_index(statement, "@" VERSION_C_);

        if ((0 < channelNameIndex) && (0 < detailsIndex) && (0 < inputIndex) && (0 < outputIndex) &&
            (0 < requestIndex) && (0 < versionIndex))
        {
            const RequestDescription * descriptor = static_cast<const RequestDescription *>(stuff);
            const char *               channelName = descriptor->_channel.c_str();

            ODL_S1("channelName <- ", channelName); //####
            result = sqlite3_bind_text(statement, channelNameIndex, channelName,
                                       static_cast<int>(strlen(channelName)), SQLITE_TRANSIENT);
            if (SQLITE_OK == result)
            {
                const char * request = descriptor->_request.c_str();

                ODL_S1("request <- ", request); //####
                result = sqlite3_bind_text(statement, requestIndex, request,
                                           static_cast<int>(strlen(request)), SQLITE_TRANSIENT);
            }
            if (SQLITE_OK == result)
            {
                const char * input = descriptor->_inputs.c_str();

                ODL_S1("input <- ", input); //####
                result = sqlite3_bind_text(statement, inputIndex, input,
                                           static_cast<int>(strlen(input)), SQLITE_TRANSIENT);
            }
            if (SQLITE_OK == result)
            {
                const char * output = descriptor->_outputs.c_str();

                ODL_S1("output <- ", output); //####
                result = sqlite3_bind_text(statement, outputIndex, output,
                                           static_cast<int>(strlen(output)), SQLITE_TRANSIENT);
            }
            if (SQLITE_OK == result)
            {
                const char * version = descriptor->_version.c_str();

                ODL_S1("version <- ", version); //####
                result = sqlite3_bind_text(statement, versionIndex, version,
                                           static_cast<int>(strlen(version)), SQLITE_TRANSIENT);
            }
            if (SQLITE_OK == result)
            {
                const char * details = descriptor->_details.c_str();

                ODL_S1("details <- ", details); //####
                result = sqlite3_bind_text(statement, detailsIndex, details,
                                           static_cast<int>(strlen(details)), SQLITE_TRANSIENT);
            }
            if (SQLITE_OK != result)
            {
                ODL_S1("error description: ", sqlite3_errstr(result)); //####
            }
        }
        else
        {
            ODL_LOG("! ((0 < channelNameIndex) && (0 < detailsIndex) && (0 < inputIndex) && " //####
                    "(0 < outputIndex) && (0 < requestIndex) && (0 < versionIndex))"); //####
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_EXIT_LL(result);
    return result;
} // setupInsertIntoRequests

/*! @brief Bind the values that are to be inserted into the RequestsKeywords table.
 @param[in] statement The prepared statement that is to be updated.
 @param[in] stuff The source of data that is to be bound.
 @returns The SQLite error from the bind operation. */
static int
setupInsertIntoRequestsKeywords(sqlite3_stmt * statement,
                                const void *   stuff)
{
    ODL_ENTER(); //####
    ODL_P2("statement = ", statement, "stuff = ", stuff); //####
    int result = SQLITE_MISUSE;

    try
    {
        int channelNameIndex = sqlite3_bind_parameter_index(statement, "@" CHANNELNAME_C_);
        int keywordIndex = sqlite3_bind_parameter_index(statement, "@" KEYWORD_C_);
        int requestIndex = sqlite3_bind_parameter_index(statement, "@" REQUEST_C_);

        if ((0 < channelNameIndex) && (0 < keywordIndex) && (0 < requestIndex))
        {
            const RequestKeywordData * descriptor = static_cast<const RequestKeywordData *>(stuff);
            const char *               keyword = descriptor->_key.c_str();

            ODL_S1("keyword <- ", keyword); //####
            result = sqlite3_bind_text(statement, keywordIndex, keyword,
                                       static_cast<int>(strlen(keyword)), SQLITE_TRANSIENT);
            if (SQLITE_OK == result)
            {
                const char * request = descriptor->_request.c_str();

                ODL_S1("request <- ", request); //####
                result = sqlite3_bind_text(statement, requestIndex, request,
                                           static_cast<int>(strlen(request)), SQLITE_TRANSIENT);
            }
            if (SQLITE_OK == result)
            {
                const char * channelName = descriptor->_channel.c_str();

                ODL_S1("channelName <- ", channelName); //####
                result = sqlite3_bind_text(statement, channelNameIndex, channelName,
                                           static_cast<int>(strlen(channelName)),
                                           SQLITE_TRANSIENT);
            }
            if (SQLITE_OK != result)
            {
                ODL_S1("error description: ", sqlite3_errstr(result)); //####
            }
        }
        else
        {
            ODL_LOG("! ((0 < channelNameIndex) && (0 < keywordIndex) && " //####
                    "(0 < requestIndex))"); //####
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_EXIT_LL(result);
    return result;
} // setupInsertIntoRequestsKeywords

/*! @brief Bind the values that are to be inserted into the Services table.
 @param[in] statement The prepared statement that is to be updated.
 @param[in] stuff The source of data that is to be bound.
 @returns The SQLite error from the bind operation. */
static int
setupInsertIntoServices(sqlite3_stmt * statement,
                        const void *   stuff)
{
    ODL_ENTER(); //####
    ODL_P2("statement = ", statement, "stuff = ", stuff); //####
    int result = SQLITE_MISUSE;

    try
    {
        int channelNameIndex = sqlite3_bind_parameter_index(statement, "@" CHANNELNAME_C_);
        int descriptionIndex = sqlite3_bind_parameter_index(statement, "@" DESCRIPTION_C_);
        int executableIndex = sqlite3_bind_parameter_index(statement, "@" EXECUTABLE_C_);
        int extraInfoIndex = sqlite3_bind_parameter_index(statement, "@" EXTRAINFO_C_);
        int nameIndex = sqlite3_bind_parameter_index(statement, "@" NAME_C_);
        int requestsDescriptionIndex = sqlite3_bind_parameter_index(statement,
                                                                    "@" REQUESTSDESCRIPTION_C_);
        int tagIndex = sqlite3_bind_parameter_index(statement, "@" TAG_C_);

        if ((0 < channelNameIndex) && (0 < descriptionIndex) && (0 < executableIndex) &&
            (0 < extraInfoIndex) && (0 < nameIndex) && (0 < requestsDescriptionIndex) &&
            (0 < tagIndex))
        {
            const ServiceData * descriptor = static_cast<const ServiceData *>(stuff);
            const char *        channelName = descriptor->_channel.c_str();

            ODL_S1("channelName <- ", channelName); //####
            result = sqlite3_bind_text(statement, channelNameIndex, channelName,
                                       static_cast<int>(strlen(channelName)), SQLITE_TRANSIENT);
            if (SQLITE_OK == result)
            {
                const char * name = descriptor->_name.c_str();

                ODL_S1("name <- ", name); //####
                result = sqlite3_bind_text(statement, nameIndex, name,
                                           static_cast<int>(strlen(name)), SQLITE_TRANSIENT);
            }
            if (SQLITE_OK == result)
            {
                const char * description = descriptor->_description.c_str();

                ODL_S1("description <- ", description); //####
                result = sqlite3_bind_text(statement, descriptionIndex, description,
                                           static_cast<int>(strlen(description)),
                                           SQLITE_TRANSIENT);
            }
            if (SQLITE_OK == result)
            {
                const char * executable = descriptor->_executable.c_str();

                ODL_S1("executable <- ", executable); //####
                result = sqlite3_bind_text(statement, executableIndex, executable,
                                           static_cast<int>(strlen(executable)),
                                           SQLITE_TRANSIENT);
            }
            if (SQLITE_OK == result)
            {
                const char * extraInfo = descriptor->_extraInfo.c_str();

                ODL_S1("extraInfo <- ", extraInfo); //####
                result = sqlite3_bind_text(statement, extraInfoIndex, extraInfo,
                                           static_cast<int>(strlen(extraInfo)), SQLITE_TRANSIENT);
            }
            if (SQLITE_OK == result)
            {
                const char * requestsDescription = descriptor->_requestsDescription.c_str();

                ODL_S1("requestsDescription <- ", requestsDescription); //####
                result = sqlite3_bind_text(statement, requestsDescriptionIndex, requestsDescription,
                                           static_cast<int>(strlen(requestsDescription)),
                                           SQLITE_TRANSIENT);
            }
            if (SQLITE_OK == result)
            {
                const char * tag = descriptor->_tag.c_str();

                ODL_S1("tag <- ", tag); //####
                result = sqlite3_bind_text(statement, tagIndex, tag, static_cast<int>(strlen(tag)),
                                           SQLITE_TRANSIENT);
            }
            if (SQLITE_OK != result)
            {
                ODL_S1("error description: ", sqlite3_errstr(result)); //####
            }
        }
        else
        {
            ODL_LOG("! ((0 < channelNameIndex) && (0 < descriptionIndex) && " //####
                    "(0 < executableIndex) && (0 < extraInfoIndex) && (0 < nameIndex) && " //####
                    "(0 < requestsDescriptionIndex) && (0 < tagIndex))"); //####
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_EXIT_LL(result);
    return result;
} // setupInsertIntoServices

/*! @brief Bind the values that are to be removed from the Channels table.
 @param[in] statement The prepared statement that is to be updated.
 @param[in] stuff The source of data that is to be bound.
 @returns The SQLite error from the bind operation. */
static int
setupRemoveFromChannels(sqlite3_stmt * statement,
                        const void *   stuff)
{
    ODL_ENTER(); //####
    ODL_P2("statement = ", statement, "stuff = ", stuff); //####
    int result = SQLITE_MISUSE;

    try
    {
        int channelNameIndex = sqlite3_bind_parameter_index(statement, "@" CHANNELNAME_C_);

        if (0 < channelNameIndex)
        {
            const char * channelName = static_cast<const char *>(stuff);

            result = sqlite3_bind_text(statement, channelNameIndex, channelName,
                                       static_cast<int>(strlen(channelName)), SQLITE_TRANSIENT);
            if (SQLITE_OK != result)
            {
                ODL_S1("error description: ", sqlite3_errstr(result)); //####
            }
        }
        else
        {
            ODL_LOG("! (0 < channelNameIndex)"); //####
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_EXIT_LL(result);
    return result;
} // setupRemoveFromChannels

/*! @brief Bind the values that are to be removed from the Requests table.
 @param[in] statement The prepared statement that is to be updated.
 @param[in] stuff The source of data that is to be bound.
 @returns The SQLite error from the bind operation. */
static int
setupRemoveFromRequests(sqlite3_stmt * statement,
                        const void *   stuff)
{
    ODL_ENTER(); //####
    ODL_P2("statement = ", statement, "stuff = ", stuff); //####
    int result = SQLITE_MISUSE;

    try
    {
        int channelNameIndex = sqlite3_bind_parameter_index(statement, "@" CHANNELNAME_C_);

        if (0 < channelNameIndex)
        {
            const char * channelName = static_cast<const char *>(stuff);

            result = sqlite3_bind_text(statement, channelNameIndex, channelName,
                                       static_cast<int>(strlen(channelName)), SQLITE_TRANSIENT);
            if (SQLITE_OK != result)
            {
                ODL_S1("error description: ", sqlite3_errstr(result)); //####
            }
        }
        else
        {
            ODL_LOG("! (0 < channelNameIndex)"); //####
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_EXIT_LL(result);
    return result;
} // setupRemoveFromRequests

/*! @brief Bind the values that are to be removed from the RequestsKeywords table.
 @param[in] statement The prepared statement that is to be updated.
 @param[in] stuff The source of data that is to be bound.
 @returns The SQLite error from the bind operation. */
static int
setupRemoveFromRequestsKeywords(sqlite3_stmt * statement,
                                const void *   stuff)
{
    ODL_ENTER(); //####
    ODL_P2("statement = ", statement, "stuff = ", stuff); //####
    int result = SQLITE_MISUSE;

    try
    {
        int channelNameIndex = sqlite3_bind_parameter_index(statement, "@" CHANNELNAME_C_);

        if (0 < channelNameIndex)
        {
            const char * channelName = static_cast<const char *>(stuff);

            result = sqlite3_bind_text(statement, channelNameIndex, channelName,
                                       static_cast<int>(strlen(channelName)), SQLITE_TRANSIENT);
            if (SQLITE_OK != result)
            {
                ODL_S1("error description: ", sqlite3_errstr(result)); //####
            }
        }
        else
        {
            ODL_LOG("! (0 < channelNameIndex)"); //####
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_EXIT_LL(result);
    return result;
} // setupRemoveFromRequestsKeywords

/*! @brief Bind the values that are to be removed from the Services table.
 @param[in] statement The prepared statement that is to be updated.
 @param[in] stuff The source of data that is to be bound.
 @returns The SQLite error from the bind operation. */
static int
setupRemoveFromServices(sqlite3_stmt * statement,
                        const void *   stuff)
{
    ODL_ENTER(); //####
    ODL_P2("statement = ", statement, "stuff = ", stuff); //####
    int result = SQLITE_MISUSE;

    try
    {
        int channelNameIndex = sqlite3_bind_parameter_index(statement, "@" CHANNELNAME_C_);

        if (0 < channelNameIndex)
        {
            const char * channelName = static_cast<const char *>(stuff);

            result = sqlite3_bind_text(statement, channelNameIndex, channelName,
                                       static_cast<int>(strlen(channelName)), SQLITE_TRANSIENT);
            if (SQLITE_OK != result)
            {
                ODL_S1("error description: ", sqlite3_errstr(result)); //####
            }
        }
        else
        {
            ODL_LOG("! (0 < channelNameIndex)"); //####
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_EXIT_LL(result);
    return result;
} // setupRemoveFromServices

#if defined(__APPLE__)
# pragma mark Class methods
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Constructors and Destructors
#endif // defined(__APPLE__)

RegistryService::RegistryService(const YarpString & launchPath,
                                 const int          argc,
                                 char * *           argv,
                                 const bool         useInMemoryDb,
                                 const YarpString & servicePortNumber) :
    inherited(kServiceKindRegistry, launchPath, argc, argv, "", true, MpM_REGISTRY_CANONICAL_NAME_,
              REGISTRY_SERVICE_DESCRIPTION_,
              "match - return the channels for services matching the criteria provided\n"
              "ping - update the last-pinged information for a channel or record the information "
              "for a service on the given channel\n"
              "register - record the information for a service on the given channel\n"
              "unregister - remove the information for a service on the given channel",
              MpM_REGISTRY_ENDPOINT_NAME_, servicePortNumber), _db(NULL),
    _validator(new ColumnNameValidator), _matchHandler(NULL), _pingHandler(NULL),
    _statusChannel(NULL), _registerHandler(NULL), _unregisterHandler(NULL),
    _checker(NULL), _inMemory(useInMemoryDb), _isActive(false)
{
    ODL_ENTER(); //####
    ODL_S2s("launchPath = ", launchPath, "servicePortNumber = ", servicePortNumber); //####
    ODL_LL1("argc = ", argc); //####
    ODL_P1("argv = ", argv); //####
    ODL_B1("useInMemoryDb = ", useInMemoryDb); //####
    setExtraInformation(_inMemory ? "Using in-memory database" : "Using disk-based database");
    attachRequestHandlers();
    ODL_EXIT_P(this); //####
} // RegistryService::RegistryService

RegistryService::~RegistryService(void)
{
    ODL_OBJENTER(); //####
    detachRequestHandlers();
    _lastCheckedTime.clear();
    if (_db)
    {
        sqlite3_close(_db);
    }
    delete _validator;
    if (_statusChannel)
    {
#if defined(MpM_DoExplicitClose)
        _statusChannel->close();
#endif // defined(MpM_DoExplicitClose)
        BaseChannel::RelinquishChannel(_statusChannel);
        _statusChannel = NULL;
    }
    ODL_OBJEXIT(); //####
} // RegistryService::~RegistryService

#if defined(__APPLE__)
# pragma mark Actions and Accessors
#endif // defined(__APPLE__)

bool
RegistryService::addRequestRecord(const yarp::os::Bottle &   keywordList,
                                  const RequestDescription & description)
{
    ODL_OBJENTER(); //####
    bool okSoFar = false;

    try
    {
        if (doBeginTransaction(_db))
        {
            static const char * insertIntoRequests = T_("INSERT INTO " REQUESTS_T_ "("
                                                        CHANNELNAME_C_ "," REQUEST_C_ "," INPUT_C_
                                                        "," OUTPUT_C_ "," VERSION_C_ "," DETAILS_C_
                                                        ") VALUES(@" CHANNELNAME_C_ ",@" REQUEST_C_
                                                        ",@" INPUT_C_ ",@" OUTPUT_C_ ",@" VERSION_C_
                                                        ",@" DETAILS_C_ ")");

            // Add the request.
            okSoFar = performSQLstatementWithNoResults(_db, insertIntoRequests,
                                                       setupInsertIntoRequests,
                                                       static_cast<const void *>(&description));
            if (okSoFar)
            {
                // Add the keywords.
                int                numKeywords = keywordList.size();
                RequestKeywordData reqKeyData;

                reqKeyData._request = description._request;
                reqKeyData._channel = description._channel;
                for (int ii = 0; okSoFar && (ii < numKeywords); ++ii)
                {
                    yarp::os::Value & aKeyword(keywordList.get(ii));

                    if (aKeyword.isString())
                    {
                        static const char * insertIntoKeywords = T_("INSERT INTO " KEYWORDS_T_ "("
                                                                    KEYWORD_C_ ") VALUES(@"
                                                                    KEYWORD_C_ ")");
                        static const char * insertIntoRequestsKeywords = T_("INSERT INTO "
                                                                            REQUESTSKEYWORDS_T_ "("
                                                                            KEYWORDS_ID_C_ ","
                                                                            REQUESTS_ID_C_
                                                                            ") SELECT @" KEYWORD_C_
                                                                            ", " KEY_C_ " FROM "
                                                                            REQUESTS_T_ " WHERE "
                                                                            REQUEST_C_ " = @"
                                                                            REQUEST_C_ " AND "
                                                                            CHANNELNAME_C_ " = @"
                                                                            CHANNELNAME_C_);

                        reqKeyData._key = aKeyword.toString();
                        okSoFar = performSQLstatementWithNoResults(_db, insertIntoKeywords,
                                                                   setupInsertIntoKeywords,
                                               static_cast<const void *>(reqKeyData._key.c_str()));
                        if (okSoFar)
                        {
                            okSoFar = performSQLstatementWithNoResults(_db,
                                                                       insertIntoRequestsKeywords,
                                                                   setupInsertIntoRequestsKeywords,
                                                                       &reqKeyData);
                        }
                    }
                    else
                    {
                        ODL_LOG("! (aKeyword.isString())"); //####
                        okSoFar = false;
                    }
                }
            }
            okSoFar = doEndTransaction(_db, okSoFar);
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    if (! okSoFar)
    {
        reportStatusChange(description._channel, kRegistryProblemAddingRequest,
                           description._request);
    }
    ODL_OBJEXIT_B(okSoFar); //####
    return okSoFar;
} // RegistryService::addRequestRecord

bool
RegistryService::addServiceRecord(const YarpString & channelName,
                                  const YarpString & name,
                                  const YarpString & tag,
                                  const YarpString & description,
                                  const YarpString & extraInfo,
                                  const YarpString & executable,
                                  const YarpString & requestsDescription)
{
    ODL_OBJENTER(); //####
    ODL_S4s("channelName = ", channelName, "name = ", name, "description = ", description, //####
            "extraInfo = ", extraInfo); //####
    ODL_S2s("executable = ", executable, "requestsDescription = ", requestsDescription); //####
    bool okSoFar = false;

    try
    {
        if (doBeginTransaction(_db))
        {
            // Add the service channel name.
            static const char * insertIntoServices = T_("INSERT INTO " SERVICES_T_ "("
                                                        CHANNELNAME_C_ "," NAME_C_ ","
                                                        DESCRIPTION_C_ "," EXECUTABLE_C_ ","
                                                        EXTRAINFO_C_ "," REQUESTSDESCRIPTION_C_ ","
                                                        TAG_C_ ") VALUES(@" CHANNELNAME_C_ ",@"
                                                        NAME_C_ ",@" DESCRIPTION_C_ ",@"
                                                        EXECUTABLE_C_ ",@" EXTRAINFO_C_ ",@"
                                                        REQUESTSDESCRIPTION_C_ ",@" TAG_C_ ")");
            ServiceData         servData;

            servData._channel = channelName;
            servData._name = name;
            servData._description = description;
            servData._executable = executable;
            servData._extraInfo = extraInfo;
            servData._requestsDescription = requestsDescription;
            servData._tag = tag;
            okSoFar = performSQLstatementWithNoResults(_db, insertIntoServices,
                                                       setupInsertIntoServices,
                                                       static_cast<const void *>(&servData));
            okSoFar = doEndTransaction(_db, okSoFar);
            reportStatusChange(channelName, kRegistryAddService,
                               Utilities::GetPortLocation(channelName));
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    if (! okSoFar)
    {
        reportStatusChange(channelName, kRegistryProblemAddingService, name);
    }
    ODL_OBJEXIT_B(okSoFar); //####
    return okSoFar;
} // RegistryService::addServiceRecord

void
RegistryService::attachRequestHandlers(void)
{
    ODL_OBJENTER(); //####
    try
    {
        ODL_LOG("got here");
        _matchHandler = new MatchRequestHandler(*this, _validator);
        ODL_LOG("got here");
        _pingHandler = new PingRequestHandler(*this);
        ODL_LOG("got here");
        _registerHandler = new RegisterRequestHandler(*this);
        ODL_LOG("got here");
        _unregisterHandler = new UnregisterRequestHandler(*this);
        if (_matchHandler && _pingHandler && _registerHandler && _unregisterHandler)
        {
            ODL_LOG("got here");
            registerRequestHandler(_matchHandler);
            ODL_LOG("got here");
            registerRequestHandler(_pingHandler);
            ODL_LOG("got here");
            registerRequestHandler(_registerHandler);
            ODL_LOG("got here");
            registerRequestHandler(_unregisterHandler);
        }
        else
        {
            ODL_LOG("! (_matchHandler && _pingHandler && _registerHandler && " //####
                    "_unregisterHandler)"); //####
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_OBJEXIT(); //####
} // RegistryService::attachRequestHandlers

bool
RegistryService::checkForExistingService(const YarpString & channelName)
{
    ODL_OBJENTER(); //####
    ODL_S1s("channelName = ", channelName); //####
    bool okSoFar = false;

    try
    {
        if (doBeginTransaction(_db))
        {
            yarp::os::Bottle    dummy;
            static const char * checkService = T_("SELECT DISTINCT " NAME_C_ " FROM " SERVICES_T_
                                                  " WHERE " CHANNELNAME_C_ " = @" CHANNELNAME_C_);

            okSoFar = performSQLstatementWithSingleColumnResults(_db, dummy, checkService, 0,
                                                                 setupCheckService,
                                                 static_cast<const void *>(channelName.c_str()));
            okSoFar = doEndTransaction(_db, okSoFar);
            if (okSoFar)
            {
                okSoFar = (1 <= dummy.size());
                if (! okSoFar)
                {
                    reportStatusChange(channelName, kRegistryNotAnExistingService);
                }
            }
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_OBJEXIT_B(okSoFar); //####
    return okSoFar;
} // RegistryService::checkForExistingService

void
RegistryService::checkServiceTimes(void)
{
    ODL_OBJENTER(); //####
    double           now = yarp::os::Time::now();
    YarpStringVector expired;

    // Build a list of expired services.
    _checkedTimeLock.lock();
    if (0 < _lastCheckedTime.size())
    {
        for (TimeMap::const_iterator walker(_lastCheckedTime.begin());
             _lastCheckedTime.end() != walker; ++walker)
        {
            double check = walker->second;

            if (now > check)
            {
                expired.push_back(walker->first);
            }
        }
    }
    _checkedTimeLock.unlock();
    if (0 < expired.size())
    {
        for (YarpStringVector::const_iterator walker(expired.begin()); expired.end() != walker;
             ++walker)
        {
            YarpString channelName(*walker);

            reportStatusChange(channelName, kRegistryStaleService);
            if (removeServiceRecord(channelName))
            {
                removeCheckedTimeForChannel(channelName);
            }
        }
    }
    ODL_OBJEXIT(); //####
} // RegistryService::checkServiceTimes

void
RegistryService::detachRequestHandlers(void)
{
    ODL_OBJENTER(); //####
    try
    {
        if (_matchHandler)
        {
            unregisterRequestHandler(_matchHandler);
            delete _matchHandler;
            _matchHandler = NULL;
        }
        if (_pingHandler)
        {
            unregisterRequestHandler(_pingHandler);
            delete _pingHandler;
            _pingHandler = NULL;
        }
        if (_registerHandler)
        {
            unregisterRequestHandler(_registerHandler);
            delete _registerHandler;
            _registerHandler = NULL;
        }
        if (_unregisterHandler)
        {
            unregisterRequestHandler(_unregisterHandler);
            delete _unregisterHandler;
            _unregisterHandler = NULL;
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_OBJEXIT(); //####
} // RegistryService::detachRequestHandlers

void
RegistryService::disableMetrics(void)
{
    ODL_OBJENTER(); //####
    inherited::disableMetrics();
    if (_statusChannel)
    {
        _statusChannel->disableMetrics();
    }
    ODL_OBJEXIT(); //####
} // RegistryService::disableMetrics

void
RegistryService::enableMetrics(void)
{
    ODL_OBJENTER(); //####
    inherited::enableMetrics();
    if (_statusChannel)
    {
        _statusChannel->enableMetrics();
    }
    ODL_OBJEXIT(); //####
} // RegistryService::enableMetrics

void
RegistryService::fillInSecondaryOutputChannelsList(ChannelVector & channels)
{
    ODL_OBJENTER(); //####
    ODL_P1("channels = ", &channels); //####
    inherited::fillInSecondaryOutputChannelsList(channels);
    if (_statusChannel)
    {
        ChannelDescription descriptor;

        descriptor._portName = _statusChannel->name();
        descriptor._portProtocol = _statusChannel->protocol();
        descriptor._portMode = kChannelModeTCP;
        descriptor._protocolDescription = _statusChannel->protocolDescription();
        channels.push_back(descriptor);
    }
    ODL_OBJEXIT(); //####
} // RegistryService::fillInSecondaryOutputChannelsList

void
RegistryService::gatherMetrics(yarp::os::Bottle & metrics)
{
    ODL_OBJENTER(); //####
    ODL_P1("metrics = ", &metrics); //####
    inherited::gatherMetrics(metrics);
    if (_statusChannel)
    {
        SendReceiveCounters counters;

        _statusChannel->getSendReceiveCounters(counters);
        counters.addToList(metrics, _statusChannel->name());
    }
    ODL_OBJEXIT(); //####
} // RegistryService::gatherMetrics

bool
RegistryService::processDictionaryEntry(yarp::os::Property & asDict,
                                        const YarpString &   channelName)
{
    ODL_ENTER(); //####
    ODL_P1("asDict = ", &asDict); //####
    ODL_S1s("channelName = ", channelName); //####
    bool result = true;

    if (asDict.check(MpM_REQREP_DICT_REQUEST_KEY_))
    {
        YarpString         theRequest(asDict.find(MpM_REQREP_DICT_REQUEST_KEY_).asString());
        yarp::os::Bottle   keywordList;
        RequestDescription requestDescriptor;

        ODL_S1s("theRequest <- ", theRequest); //####
        if (asDict.check(MpM_REQREP_DICT_DETAILS_KEY_))
        {
            yarp::os::Value theDetails = asDict.find(MpM_REQREP_DICT_DETAILS_KEY_);

            ODL_S1s("theDetails <- ", theDetails.toString()); //####
            if (theDetails.isString())
            {
                requestDescriptor._details = theDetails.toString();
            }
            else
            {
                ODL_LOG("! (theDetails.isString())"); //####
                // The details field is present, but it's not a string.
                result = false;
            }
        }
        if (asDict.check(MpM_REQREP_DICT_INPUT_KEY_))
        {
            yarp::os::Value theInputs(asDict.find(MpM_REQREP_DICT_INPUT_KEY_));

            ODL_S1s("theInputs <- ", theInputs.toString()); //####
            if (theInputs.isString())
            {
                requestDescriptor._inputs = theInputs.toString();
            }
            else
            {
                ODL_LOG("! (theInputs.isString())"); //####
                // The inputs descriptor is present, but it's not a string
                result = false;
            }
        }
        if (asDict.check(MpM_REQREP_DICT_KEYWORDS_KEY_))
        {
            yarp::os::Value theKeywords(asDict.find(MpM_REQREP_DICT_KEYWORDS_KEY_));

            ODL_S1s("theKeywords <- ", theKeywords.toString()); //####
            if (theKeywords.isList())
            {
                keywordList = *theKeywords.asList();
            }
            else
            {
                ODL_LOG("! (theKeywords.isList())"); //####
                // The keywords entry is present, but it's not a list
                result = false;
            }
        }
        if (asDict.check(MpM_REQREP_DICT_OUTPUT_KEY_))
        {
            yarp::os::Value theOutputs(asDict.find(MpM_REQREP_DICT_OUTPUT_KEY_));

            ODL_S1s("theOutputs <- ", theOutputs.toString()); //####
            if (theOutputs.isString())
            {
                requestDescriptor._outputs = theOutputs.toString();
            }
            else
            {
                ODL_LOG("! (theOutputs.isString())"); //####
                // The outputs descriptor is present, but it's not a string
                result = false;
            }
        }
        if (asDict.check(MpM_REQREP_DICT_VERSION_KEY_))
        {
            yarp::os::Value theVersion(asDict.find(MpM_REQREP_DICT_VERSION_KEY_));

            ODL_S1s("theVersion <- ", theVersion.toString()); //####
            if (theVersion.isString() || theVersion.isInt() || theVersion.isDouble())
            {
                requestDescriptor._version = theVersion.toString();
            }
            else
            {
                ODL_LOG("! (theVersion.isString() || theVersion.isInt() || " //####
                        "theVersion.isDouble())"); //####
                // The version entry is present, but it's not a simple value
                result = false;
            }
        }
        if (result)
        {
            requestDescriptor._channel = channelName;
            requestDescriptor._request = theRequest;
            result = addRequestRecord(keywordList, requestDescriptor);
            ODL_B1("result <- ", result); //####
            if (! result)
            {
                // We need to remove any values that we've recorded for this channel!
                removeServiceRecord(channelName);
            }
        }
    }
    else
    {
        ODL_LOG("! (asDict.check(MpM_REQREP_DICT_REQUEST_KEY_))"); //####
        // There is no 'name' entry in this dictionary
        result = false;
    }
    ODL_EXIT_B(result); //####
    return result;
} // RegistryService::processDictionaryEntry

bool
RegistryService::processListResponse(const YarpString &      channelName,
                                     const ServiceResponse & response)
{
    ODL_OBJENTER(); //####
    ODL_S2s("channelName = ", channelName, "response = ", response.asString()); //####
    bool result = false;

    try
    {
        int count = response.count();

        if (0 < count)
        {
            result = true;
            for (int ii = 0; result && (ii < count); ++ii)
            {
                yarp::os::Value anElement(response.element(ii));

                if (anElement.isDict())
                {
                    yarp::os::Property * asDict = anElement.asDict();

                    if (asDict)
                    {
                        result = processDictionaryEntry(*asDict, channelName);
                    }
                }
                else if (anElement.isList())
                {
                    yarp::os::Bottle * asList = anElement.asList();

                    if (asList)
                    {
                        yarp::os::Property asDict;

                        if (ListIsReallyDictionary(*asList, asDict))
                        {
                            result = processDictionaryEntry(asDict, channelName);;
                        }
                        else
                        {
                            result = false;
                        }
                    }
                    else
                    {
                        result = false;
                    }
                }
                else
                {
                    ODL_LOG("! (anElement.isList())"); //####
                    result = false;
                }
            }
        }
        else
        {
            ODL_LOG("! (0 < count)"); //####
            // Wrong number of values in the response.
            result = false;
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_OBJEXIT_B(result); //####
    return result;
} // RegistryService::processListResponse

bool
RegistryService::processMatchRequest(Parser::MatchExpression * matcher,
                                     const bool                getNames,
                                     yarp::os::Bottle &        reply)
{
    ODL_OBJENTER(); //####
    ODL_P1("matcher = ", matcher); //####
    ODL_B1("getNames = ", getNames); //####
    bool okSoFar = false;

    try
    {
        if (matcher)
        {
            if (doBeginTransaction(_db))
            {
                yarp::os::Bottle &  subList = reply.addList();
                static const char * sqlStartGetNames = T_("SELECT DISTINCT " NAME_C_ " FROM "
                                                          SERVICES_T_ " WHERE " CHANNELNAME_C_
                                                          " IN (SELECT DISTINCT " CHANNELNAME_C_
                                                          " FROM " REQUESTS_T_ " WHERE ");
                static const char * sqlStartNoNames = T_("SELECT DISTINCT " CHANNELNAME_C_
                                                         " FROM " REQUESTS_T_ " WHERE ");
                const char *        sqlStart = (getNames ?  sqlStartGetNames : sqlStartNoNames);
                const char *        sqlEnd = (getNames ? T_(")") : T_(""));
                YarpString          requestAsSQL(matcher->asSQLString(sqlStart, sqlEnd));

                ODL_S1s("requestAsSQL <- ", requestAsSQL); //####
                okSoFar = performSQLstatementWithSingleColumnResults(_db, subList,
                                                                     requestAsSQL.c_str());
                okSoFar = doEndTransaction(_db, okSoFar);
            }
        }
        else
        {
            ODL_LOG("! (matcher)"); //####
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_OBJEXIT_B(okSoFar); //####
    return okSoFar;
} // RegistryService::processMatchRequest

bool
RegistryService::processNameResponse(const YarpString &      channelName,
                                     const ServiceResponse & response)
{
    ODL_OBJENTER(); //####
    ODL_S2s("channelName = ", channelName, "response = ", response.asString()); //####
    bool result = false;

    try
    {
        if (MpM_EXPECTED_NAME_RESPONSE_SIZE_ == response.count())
        {
            yarp::os::Value theCanonicalName(response.element(0));
            yarp::os::Value theDescription(response.element(1));
            yarp::os::Value theExtraInformation(response.element(2));
            yarp::os::Value theKind(response.element(3));
            yarp::os::Value thePath(response.element(4));
            yarp::os::Value theRequestsDescription(response.element(5));
            yarp::os::Value theTag(response.element(6));

            if (theCanonicalName.isString() && theDescription.isString() &&
                theExtraInformation.isString() && theKind.isString() && thePath.isString() &&
                theRequestsDescription.isString())
            {
                result = addServiceRecord(channelName, theCanonicalName.toString(),
                                          theTag.toString(), theDescription.toString(),
                                          theExtraInformation.toString(), thePath.toString(),
                                          theRequestsDescription.toString());
                if (! result)
                {
                    // We need to remove any values that we've recorded for this channel!
                    removeServiceRecord(channelName);
                }
            }
            else
            {
                ODL_LOG("! (theCanonicalName.isString() && theDescription.isString() && " //####
                        "theExtraInformation.isString() && theKind.isString() && " //####
                        "thePath.isString() && theRequestsDescription.isString())"); //####
                result = false;
            }
        }
        else
        {
            ODL_LOG("! (MpM_EXPECTED_NAME_RESPONSE_SIZE_ == response.count())"); //####
            ODL_S1s("response = ", response.asString()); //####
            // Wrong number of values in the response.
            result = false;
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_OBJEXIT_B(result); //####
    return result;
} // RegistryService::processNameResponse

void
RegistryService::removeCheckedTimeForChannel(const YarpString & serviceChannelName)
{
    ODL_OBJENTER(); //####
    _checkedTimeLock.lock();
    _lastCheckedTime.erase(serviceChannelName);
    _checkedTimeLock.unlock();
    ODL_OBJEXIT(); //####
} // RegistryService::removeCheckedTimeForChannel

bool
RegistryService::removeServiceRecord(const YarpString & serviceChannelName)
{
    ODL_OBJENTER(); //####
    ODL_S1s("serviceChannelName = ", serviceChannelName); //####
    bool okSoFar = false;

    try
    {
        if (doBeginTransaction(_db))
        {
            // Remove the service channel requests.
            static const char * removeFromRequestsKeywords = T_("DELETE FROM " REQUESTSKEYWORDS_T_
                                                                " WHERE " REQUESTS_ID_C_
                                                                " IN (SELECT " KEY_C_ " FROM "
                                                                REQUESTS_T_ " WHERE " CHANNELNAME_C_
                                                                " = @" CHANNELNAME_C_ ")");

            okSoFar = performSQLstatementWithNoResults(_db, removeFromRequestsKeywords,
                                                       setupRemoveFromRequestsKeywords,
                                           static_cast<const void *>(serviceChannelName.c_str()));
            if (okSoFar)
            {
                // Remove the service channel requests.
                static const char * removeFromRequests = T_("DELETE FROM " REQUESTS_T_ " WHERE "
                                                            CHANNELNAME_C_ " = @" CHANNELNAME_C_);

                okSoFar = performSQLstatementWithNoResults(_db, removeFromRequests,
                                                           setupRemoveFromRequests,
                                           static_cast<const void *>(serviceChannelName.c_str()));
            }
            if (okSoFar)
            {
                // Remove the service channel name.
                static const char * removeFromServices = T_("DELETE FROM " SERVICES_T_ " WHERE "
                                                            CHANNELNAME_C_ " = @" CHANNELNAME_C_);

                okSoFar = performSQLstatementWithNoResults(_db, removeFromServices,
                                                           setupRemoveFromServices,
                                           static_cast<const void *>(serviceChannelName.c_str()));
            }
            okSoFar = doEndTransaction(_db, okSoFar);
            reportStatusChange(serviceChannelName, kRegistryRemoveService);
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_OBJEXIT_B(okSoFar); //####
    return okSoFar;
} // RegistryService::removeServiceRecord

void
RegistryService::reportStatusChange(const YarpString &  channelName,
                                    const ServiceStatus newStatus,
                                    const YarpString &  details)
{
    ODL_OBJENTER(); //####
    ODL_S2s("channelName = ", channelName, "details = ", details); //####
    if (_statusChannel)
    {
        char             buffer1[DATE_TIME_BUFFER_SIZE_];
        char             buffer2[DATE_TIME_BUFFER_SIZE_];
        yarp::os::Bottle message;

        Utilities::GetDateAndTime(buffer1, sizeof(buffer1), buffer2, sizeof(buffer2));
        message.addString(buffer1);
        message.addString(buffer2);
        message.addString(serviceName());
        switch (newStatus)
        {
            case kRegistryAddService :
                message.addString(MpM_REGISTRY_STATUS_ADDING_);
                message.addString(channelName);
                message.addString("at");
                message.addString(details);
                break;

            case kRegistryNotAnExistingService :
                message.addString(MpM_REGISTRY_STATUS_UNRECOGNIZED_);
                message.addString(channelName);
                break;

            case kRegistryPingFromService :
                message.addString(MpM_REGISTRY_STATUS_PINGED_);
                message.addString("by");
                message.addString(channelName);
                break;

            case kRegistryProblemAddingRequest :
                message.addString(MpM_REGISTRY_STATUS_PROBLEM_);
                message.addString("adding");
                message.addString("request");
                message.addString(details);
                message.addString("for");
                message.addString(channelName);
                break;

            case kRegistryProblemAddingService :
                message.addString(MpM_REGISTRY_STATUS_PROBLEM_);
                message.addString("adding");
                message.addString("service");
                message.addString(details);
                message.addString("for");
                message.addString(channelName);
                break;

            case kRegistryRegisterService :
                message.addString(MpM_REGISTRY_STATUS_REGISTERING_);
                message.addString(channelName);
                break;

            case kRegistryRemoveService :
                message.addString(MpM_REGISTRY_STATUS_REMOVING_);
                message.addString(channelName);
                break;

            case kRegistryStaleService :
                message.addString(MpM_REGISTRY_STATUS_STALE_);
                message.addString(channelName);
                message.addString("detected");
                break;

            case kRegistryStarted :
                message.addString(MpM_REGISTRY_STATUS_STARTING_);
                break;

            case kRegistryStopped :
                message.addString(MpM_REGISTRY_STATUS_STOPPING_);
                break;

            case kRegistryUnregisterService :
                message.addString(MpM_REGISTRY_STATUS_UNREGISTERING_);
                message.addString(channelName);
                break;

            default :
                break;

        }
        if (! _statusChannel->writeBottle(message))
        {
            ODL_LOG("(! _statusChannel->writeBottle(message))"); //####
#if defined(MpM_StallOnSendProblem)
            Stall();
#endif // defined(MpM_StallOnSendProblem)
        }
    }
    ODL_OBJEXIT(); //####
} // RegistryService::reportStatusChange

bool
RegistryService::setUpDatabase(void)
{
    ODL_OBJENTER(); //####
    bool okSoFar = true;

    try
    {
        int sqlRes;

        if (! _db)
        {
            const char * dbFileName;

#if defined(MpM_UseTestDatabase)
            dbFileName = "/tmp/test.db";
#else // ! defined(MpM_UseTestDatabase)
            if (_inMemory)
            {
                dbFileName = ":memory:";
            }
            else
            {
                dbFileName = "";
            }
#endif // ! defined(MpM_UseTestDatabase)
            sqlRes = sqlite3_open_v2(dbFileName, &_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                                     NULL);
            if (SQLITE_OK != sqlRes)
            {
                ODL_LOG("(SQLITE_OK != sqlRes)"); //####
                okSoFar = false;
                sqlite3_close(_db);
                _db = NULL;
            }
        }
        if (_db)
        {
            okSoFar = constructTables(_db);
            if (! okSoFar)
            {
                ODL_LOG("(! okSoFar)"); //####
                sqlite3_close(_db);
                _db = NULL;
            }
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_OBJEXIT_B(okSoFar); //####
    return okSoFar;
} // RegistryService::setUpDatabase

bool
RegistryService::setUpStatusChannel(void)
{
    ODL_OBJENTER(); //####
    bool okSoFar = false;

    try
    {
        _statusChannel = new GeneralChannel(true);
        if (_statusChannel)
        {
            YarpString              outputName(MpM_REGISTRY_STATUS_NAME_);
#if defined(MpM_ReportOnConnections)
            ChannelStatusReporter * reporter = Utilities::GetGlobalStatusReporter();
#endif // defined(MpM_ReportOnConnections)

#if defined(MpM_ReportOnConnections)
            _statusChannel->setReporter(*reporter);
            _statusChannel->getReport(*reporter);
#endif // defined(MpM_ReportOnConnections)
            if (metricsAreEnabled())
            {
                _statusChannel->enableMetrics();
            }
            else
            {
                _statusChannel->disableMetrics();
            }
            if (_statusChannel->openWithRetries(outputName, STANDARD_WAIT_TIME_))
            {
                _statusChannel->setProtocol("s+", "One or more strings");
                okSoFar = true;
            }
            else
            {
                ODL_LOG("! (_statusChannel->openWithRetries(outputName, " //####
                        "STANDARD_WAIT_TIME_))"); //####
            }
        }
        else
        {
            ODL_LOG("! (_statusChannel)"); //####
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_OBJEXIT_B(okSoFar); //####
    return okSoFar;
} // RegistryService::setUpStatusChannel

bool
RegistryService::startService(void)
{
    ODL_OBJENTER(); //####
    bool result = false;

    try
    {
        if ((! isActive()) && (! isStarted()))
        {
            inherited::startService();
            if (isStarted() && setUpDatabase() && setUpStatusChannel())
            {
                // Register ourselves!!!
                YarpString      aName(GetRandomChannelName(HIDDEN_CHANNEL_PREFIX_ "temp_"
                                                           MpM_REGISTRY_ENDPOINT_NAME_));
                ClientChannel * newChannel = new ClientChannel;

                if (newChannel)
                {
                    if (metricsAreEnabled())
                    {
                        newChannel->enableMetrics();
                    }
                    else
                    {
                        newChannel->disableMetrics();
                    }
                    if (newChannel->openWithRetries(aName, STANDARD_WAIT_TIME_))
                    {
                        if (Utilities::NetworkConnectWithRetries(aName, MpM_REGISTRY_ENDPOINT_NAME_,
                                                                 STANDARD_WAIT_TIME_))
                        {
                            yarp::os::Bottle parameters(MpM_REGISTRY_ENDPOINT_NAME_);
                            ServiceRequest   request(MpM_REGISTER_REQUEST_, parameters);
                            ServiceResponse  response;

                            if (request.send(*newChannel, response))
                            {
                                // Check that we got a successful self-registration!
                                if (MpM_EXPECTED_REGISTER_RESPONSE_SIZE_ == response.count())
                                {
                                    yarp::os::Value theValue = response.element(0);

                                    ODL_S1s("theValue <- ", theValue.toString()); //####
                                    if (theValue.isString())
                                    {
                                        _isActive = (theValue.toString() == MpM_OK_RESPONSE_);
                                        ODL_B1("_isActive <- ", _isActive); //####
                                    }
                                    else
                                    {
                                        ODL_LOG("! (theValue.isString())"); //####
                                    }
                                }
                                else
                                {
                                    ODL_LOG("! (MpM_EXPECTED_REGISTER_RESPONSE_SIZE_ == " //####
                                            "response.count())"); //####
                                    ODL_S1s("response = ", response.asString()); //####
                                }
                            }
                            else
                            {
                                ODL_LOG("! (request.send(*newChannel, response))"); //####
                            }
                        }
                        else
                        {
                            ODL_LOG("! (Utilities::NetworkConnectWithRetries(aName, " //####
                                    "MpM_REGISTRY_ENDPOINT_NAME_, STANDARD_WAIT_TIME_))"); //####
                        }
#if defined(MpM_DoExplicitClose)
                        newChannel->close();
#endif // defined(MpM_DoExplicitClose)
                    }
                    else
                    {
                        ODL_LOG("! (newChannel->openWithRetries(aName, " //####
                                "STANDARD_WAIT_TIME_))"); //####
                    }
                    BaseChannel::RelinquishChannel(newChannel);
                }
                else
                {
                    ODL_LOG("! (newChannel)"); //####
                }
            }
            else
            {
                ODL_LOG("! (isStarted() && setUpDatabase() && setUpStatusChannel())"); //####
            }
        }
        result = isStarted();
        if (result)
        {
            reportStatusChange("", kRegistryStarted);
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_OBJEXIT_B(result); //####
    return result;
} // RegistryService::startService

void
RegistryService::startChecker(void)
{
    ODL_OBJENTER(); //####
    if (! _checker)
    {
        _checker = new RegistryCheckThread(*this);
        ODL_P1("_checker <- ", _checker); //####
        if (! _checker->start())
        {
            ODL_LOG("(! _checker->start())"); //####
            delete _checker;
            _checker = NULL;
        }
    }
    ODL_OBJEXIT(); //####
} // RegistryService::startChecker

bool
RegistryService::stopService(void)
{
    ODL_OBJENTER(); //####
    bool result = false;

    try
    {
        if (_checker)
        {
            ODL_P1("_checker = ", _checker); //####
            _checker->stop();
            for ( ; _checker->isRunning(); )
            {
                yarp::os::Time::delay(PING_CHECK_INTERVAL_ / 3.1);
            }
            delete _checker;
            _checker = NULL;
        }
        if (isActive())
        {
            reportStatusChange("", kRegistryStopped);
        }
        result = inherited::stopService();
        _isActive = false;
        ODL_B1("_isActive <- ", _isActive); //####
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_OBJEXIT_B(result); //####
    return result;
} // RegistryService::stopService

void
RegistryService::updateCheckedTimeForChannel(const YarpString & serviceChannelName)
{
    ODL_OBJENTER(); //####
    _checkedTimeLock.lock();
    _lastCheckedTime[serviceChannelName] = yarp::os::Time::now() +
                                            (PING_COUNT_MAX_ * PING_INTERVAL_);
    _checkedTimeLock.unlock();
    ODL_OBJEXIT(); //####
} // RegistryService::updateCheckedTimeForChannel

#if defined(__APPLE__)
# pragma mark Global functions
#endif // defined(__APPLE__)
