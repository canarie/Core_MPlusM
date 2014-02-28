//
//  YPPExampleService.cpp
//  YarpPlusPlus
//
//  Created by Norman Jaffe on 2014-02-06.
//  Copyright (c) 2014 OpenDragon. All rights reserved.
//

#include "YPPExampleService.h"
#define ENABLE_OD_SYSLOG /* */
#include "ODSyslog.h"
#include "YPPEchoRequestHandler.h"
#include "YPPRequests.h"

using namespace YarpPlusPlusExample;

#pragma mark Private structures and constants

ExampleService::ExampleService(const yarp::os::ConstString & serviceEndpointName,
                               const yarp::os::ConstString & serviceHostName,
                               const yarp::os::ConstString & servicePortNumber) :
        inherited(true, serviceEndpointName, serviceHostName, servicePortNumber)
{
    OD_SYSLOG_ENTER();//####
    registerRequestHandler(YPP_ECHO_REQUEST, new EchoRequestHandler(*this));
    OD_SYSLOG_EXIT();//####
} // ExampleService::ExampleService

ExampleService::ExampleService(const int argc,
                               char **   argv) :
        BaseService(true, argc, argv)
{
    OD_SYSLOG_ENTER();//####
    registerRequestHandler(YPP_ECHO_REQUEST, new EchoRequestHandler(*this));
    OD_SYSLOG_EXIT();//####
} // ExampleService::ExampleService

ExampleService::~ExampleService(void)
{
    OD_SYSLOG_ENTER();//####
    OD_SYSLOG_EXIT();//####
} // ExampleService::~ExampleService

#pragma mark Actions

bool ExampleService::start(void)
{
    OD_SYSLOG_ENTER();//####
    if (! isStarted())
    {
        BaseService::start();
        if (isStarted())
        {
            
        }
    }
    OD_SYSLOG_EXIT_B(isStarted());//####
    return isStarted();
} // ExampleService::start

bool ExampleService::stop(void)
{
    OD_SYSLOG_ENTER();//####
    bool result = BaseService::stop();
    
    OD_SYSLOG_EXIT_B(result);//####
    return result;
} // ExampleService::stop
