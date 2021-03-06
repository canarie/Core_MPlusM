//--------------------------------------------------------------------------------------------------
//
//  File:       m+mRequestCounterClient.cpp
//
//  Project:    m+m
//
//  Contains:   The class definition for the client of the Request Counter service.
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
//  Created:    2014-03-14
//
//--------------------------------------------------------------------------------------------------

#include "m+mRequestCounterClient.hpp"
#include "m+mRequestCounterRequests.hpp"

#include <m+m/m+mServiceResponse.hpp>

//#include <odlEnable.h>
#include <odlInclude.h>

#if defined(__APPLE__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wunknown-pragmas"
# pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#endif // defined(__APPLE__)
/*! @file
 @brief The class definition for the client of the Request Counter service. */
#if defined(__APPLE__)
# pragma clang diagnostic pop
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Namespace references
#endif // defined(__APPLE__)

using namespace MplusM;
using namespace MplusM::Common;
using namespace MplusM::RequestCounter;

#if defined(__APPLE__)
# pragma mark Private structures, constants and variables
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Global constants and variables
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Local functions
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Class methods
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Constructors and Destructors
#endif // defined(__APPLE__)

RequestCounterClient::RequestCounterClient(void) :
    inherited("requestcounter_")
{
    ODL_ENTER(); //####
    ODL_EXIT_P(this); //####
} // RequestCounterClient::RequestCounterClient

RequestCounterClient::~RequestCounterClient(void)
{
    ODL_OBJENTER(); //####
    ODL_OBJEXIT(); //####
} // RequestCounterClient::~RequestCounterClient

#if defined(__APPLE__)
# pragma mark Actions and Accessors
#endif // defined(__APPLE__)

bool
RequestCounterClient::getServiceStatistics(long &   counter,
                                           double & elapsedTime)
{
    ODL_OBJENTER(); //####
    ODL_P2("counter = ", &counter, "elapsedTime = ", &elapsedTime); //####
    bool okSoFar = false;

    try
    {
        yarp::os::Bottle parameters;
        ServiceResponse  response;

        reconnectIfDisconnected();
        if (send(MpM_STATS_REQUEST_, parameters, response))
        {
            if (MpM_EXPECTED_STATS_RESPONSE_SIZE_ == response.count())
            {
                yarp::os::Value retrievedCounter(response.element(0));
                yarp::os::Value retrievedElapsed(response.element(1));

                if (retrievedCounter.isInt() && retrievedElapsed.isDouble())
                {
                    counter = retrievedCounter.asInt();
                    elapsedTime = retrievedElapsed.asDouble();
                    okSoFar = true;
                }
                else
                {
                    ODL_LOG("! (retrievedCounter.isInt() && retrievedElapsed.isDouble())"); //####
                }
            }
            else
            {
                ODL_LOG("! (MpM_EXPECTED_STATS_RESPONSE_SIZE_ == response.count())"); //####
                ODL_S1s("response = ", response.asString()); //####
            }
        }
        else
        {
            ODL_LOG("! (send(MpM_STATS_REQUEST_, parameters, response))"); //####
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_OBJEXIT_B(okSoFar); //####
    return okSoFar;
} // RequestCounterClient::getServiceStatistics

bool
RequestCounterClient::pokeService(void)
{
    ODL_OBJENTER(); //####
    bool okSoFar = false;

    try
    {
        yarp::os::Bottle parameters;

        if (send("blarg_blerg_blirg_blorg_blurg", parameters))
        {
            okSoFar = true;
        }
        else
        {
            ODL_LOG("! (send(\"blarg_blerg_blirg_blorg_blurg\", parameters))"); //####
        }
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_OBJEXIT_B(okSoFar); //####
    return okSoFar;
} // RequestCounterClient::pokeService

bool
RequestCounterClient::resetServiceCounters(void)
{
    ODL_OBJENTER(); //####
    bool okSoFar = false;

    try
    {
        yarp::os::Bottle parameters;
#if defined(MpM_DoExplicitCheckForOK)
        ServiceResponse  response;
#endif // defined(MpM_DoExplicitCheckForOK)

        reconnectIfDisconnected();
#if defined(MpM_DoExplicitCheckForOK)
        if (send(MpM_RESETCOUNTER_REQUEST_, parameters, response))
        {
            if (MpM_EXPECTED_RESETCOUNTER_RESPONSE_SIZE_ == response.count())
            {
                yarp::os::Value retrieved(response.element(0));

                if (retrieved.isString())
                {
                    okSoFar = (retrieved.toString() == MpM_OK_RESPONSE_);
                }
                else
                {
                    ODL_LOG("! (retrieved.isString())"); //####
                }
            }
            else
            {
                ODL_LOG("! (MpM_EXPECTED_RESETCOUNTER_RESPONSE_SIZE_ == response.count())"); //####
                ODL_S1s("response = ", response.asString()); //####
            }
        }
        else
        {
            ODL_LOG("! (send(MpM_RESETCOUNTER_REQUEST_, parameters, response))"); //####
        }
#else // ! defined(MpM_DoExplicitCheckForOK)
        if (send(MpM_RESETCOUNTER_REQUEST_, parameters))
        {
            okSoFar = true;
        }
        else
        {
            ODL_LOG("! (send(MpM_RESETCOUNTER_REQUEST_, parameters))"); //####
        }
#endif // ! defined(MpM_DoExplicitCheckForOK)
    }
    catch (...)
    {
        ODL_LOG("Exception caught"); //####
        throw;
    }
    ODL_OBJEXIT_B(okSoFar); //####
    return okSoFar;
} // RequestCounterClient::resetServiceCounters

#if defined(__APPLE__)
# pragma mark Global functions
#endif // defined(__APPLE__)
