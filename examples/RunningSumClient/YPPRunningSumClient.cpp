//--------------------------------------------------------------------------------------
//
//  File:       YPPRunningSumClient.cpp
//
//  Project:    YarpPlusPlus
//
//  Contains:   The class definition for the client of a simple Yarp++ service with context.
//
//  Written by: Norman Jaffe
//
//  Copyright:  (c) 2014 by HPlus Technologies Ltd. and Simon Fraser University.
//
//              All rights reserved. Redistribution and use in source and binary forms,
//              with or without modification, are permitted provided that the following
//              conditions are met:
//                * Redistributions of source code must retain the above copyright
//                  notice, this list of conditions and the following disclaimer.
//                * Redistributions in binary form must reproduce the above copyright
//                  notice, this list of conditions and the following disclaimer in the
//                  documentation and/or other materials provided with the
//                  distribution.
//                * Neither the name of the copyright holders nor the names of its
//                  contributors may be used to endorse or promote products derived
//                  from this software without specific prior written permission.
//
//              THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//              "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//              LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
//              PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//              OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//              SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//              LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//              DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//              THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//              (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//              OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//  Created:    2014-03-18
//
//--------------------------------------------------------------------------------------

#include "YPPRunningSumClient.h"
//#define OD_ENABLE_LOGGING /* */
#include "ODLogging.h"
#include "YPPEndpoint.h"
#include "YPPException.h"
#include "YPPRunningSumRequests.h"
#include "YPPServiceResponse.h"
#if defined(__APPLE__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wc++11-extensions"
# pragma clang diagnostic ignored "-Wdocumentation"
# pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
# pragma clang diagnostic ignored "-Wpadded"
# pragma clang diagnostic ignored "-Wshadow"
# pragma clang diagnostic ignored "-Wunused-parameter"
# pragma clang diagnostic ignored "-Wweak-vtables"
#endif // defined(__APPLE__)
#include <yarp/os/Time.h>
#if defined(__APPLE__)
# pragma clang diagnostic pop
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#endif // defined(__APPLE__)
/*! @file
 
 @brief The class definition for the client of a simple Yarp++ service with context. */
#if defined(__APPLE__)
# pragma clang diagnostic pop
#endif // defined(__APPLE__)

using namespace YarpPlusPlusExample;

#if defined(__APPLE__)
# pragma mark Private structures, constants and variables
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Local functions
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Class methods
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Constructors and destructors
#endif // defined(__APPLE__)

RunningSumClient::RunningSumClient(void) :
        inherited("example/runningsum_")
{
    OD_LOG_ENTER();//####
    OD_LOG_EXIT_P(this);//####
} // RunningSumClient::RunningSumClient

RunningSumClient::~RunningSumClient(void)
{
    OD_LOG_OBJENTER();//####
    OD_LOG_OBJEXIT();//####
} // RunningSumClient::~RunningSumClient

#if defined(__APPLE__)
# pragma mark Actions
#endif // defined(__APPLE__)

bool RunningSumClient::addToSum(const double value,
                                double &     newSum)
{
    OD_LOG_OBJENTER();//####
    bool okSoFar = false;
    
    try
    {
        yarp::os::Bottle              parameters;
        YarpPlusPlus::ServiceResponse response;
        
        parameters.addDouble(value);
        if (send(YPP_ADD_REQUEST, parameters, &response))
        {
            if (1 == response.count())
            {
                yarp::os::Value retrieved(response.element(0));
                
                if (retrieved.isDouble())
                {
                    newSum = retrieved.asDouble();
                    okSoFar = true;
                }
                else if (retrieved.isInt())
                {
                    newSum = retrieved.asInt();
                    okSoFar = true;
                }
                else
                {
                    OD_LOG("! (retrieved.isInt())");//####
                }
            }
            else
            {
                OD_LOG("! (1 == response.count())");//####
            }
        }
        else
        {
            OD_LOG("! (send(YPP_ADD_REQUEST, parameters, &response))");//####
        }
    }
    catch (...)
    {
        OD_LOG("Exception caught");//####
        throw;
    }
    OD_LOG_OBJEXIT_B(okSoFar);
    return okSoFar;
} // RunningSumClient::addToSum

bool RunningSumClient::resetSum(void)
{
    OD_LOG_OBJENTER();//####
    bool okSoFar = false;

    try
    {
        yarp::os::Bottle parameters;
        
        if (send(YPP_RESET_REQUEST, parameters))
        {
            okSoFar = true;
        }
        else
        {
            OD_LOG("! (send(YPP_RESET_REQUEST, parameters))");//####
        }
    }
    catch (...)
    {
        OD_LOG("Exception caught");//####
        throw;
    }
    OD_LOG_OBJEXIT_B(okSoFar);
    return okSoFar;
} // RunningSumClient::resetSum

bool RunningSumClient::startSum(void)
{
    OD_LOG_OBJENTER();//####
    bool okSoFar = false;

    try
    {
        yarp::os::Bottle parameters;
        
        if (send(YPP_START_REQUEST, parameters))
        {
            okSoFar = true;
        }
        else
        {
            OD_LOG("! (send(YPP_START_REQUEST, parameters))");//####
        }
    }
    catch (...)
    {
        OD_LOG("Exception caught");//####
        throw;
    }
    OD_LOG_OBJEXIT_B(okSoFar);
    return okSoFar;
} // RunningSumClient::startSum

bool RunningSumClient::stopSum(void)
{
    OD_LOG_OBJENTER();//####
    bool okSoFar = false;
    
    try
    {
        yarp::os::Bottle parameters;
        
        if (send(YPP_STOP_REQUEST, parameters))
        {
            okSoFar = true;
        }
        else
        {
            OD_LOG("! (send(YPP_STOP_REQUEST, parameters))");//####
        }
    }
    catch (...)
    {
        OD_LOG("Exception caught");//####
        throw;
    }
    OD_LOG_OBJEXIT_B(okSoFar);
    return okSoFar;
} // RunningSumClient::stopSum

#if defined(__APPLE__)
# pragma mark Accessors
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Global functions
#endif // defined(__APPLE__)
