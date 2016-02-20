//--------------------------------------------------------------------------------------------------
//
//  File:       m+m/m+mBaseService.cpp
//
//  Project:    m+m
//
//  Contains:   The class definition for the minimal functionality required for an m+m service.
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

#include "m+mBaseService.h"

#include "m+mArgumentsRequestHandler.h"
#include "m+mChannelsRequestHandler.h"
#include "m+mClientsRequestHandler.h"
#include "m+mDetachRequestHandler.h"
#include "m+mExtraInfoRequestHandler.h"
#include "m+mInfoRequestHandler.h"
#include "m+mListRequestHandler.h"
#include "m+mMetricsRequestHandler.h"
#include "m+mMetricsStateRequestHandler.h"
#include "m+mNameRequestHandler.h"
#include "m+mPingThread.h"
#include "m+mSetMetricsStateRequestHandler.h"
#include "m+mStopRequestHandler.h"

#include <m+m/m+mBaseContext.h>
#include <m+m/m+mClientChannel.h>
#include <m+m/m+mEndpoint.h>
#include <m+m/m+mException.h>
#include <m+m/m+mRequests.h>
#include <m+m/m+mServiceInputHandler.h>
#include <m+m/m+mServiceInputHandlerCreator.h>
#include <m+m/m+mServiceRequest.h>
#include <m+m/m+mServiceResponse.h>
#include <m+m/m+mUtilities.h>
#if (! MAC_OR_LINUX_)
# pragma warning(push)
# pragma warning(disable: 4512)
#endif // ! MAC_OR_LINUX_
#include <m+m/optionparser.h>
#if (! MAC_OR_LINUX_)
# pragma warning(pop)
#endif // ! MAC_OR_LINUX_

//#include <odl/ODEnableLogging.h>
#include <odl/ODLogging.h>

#if defined(__APPLE__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wunknown-pragmas"
# pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#endif // defined(__APPLE__)
/*! @file
 @brief The class definition for the minimal functionality required for an m+m service. */
#if defined(__APPLE__)
# pragma clang diagnostic pop
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Namespace references
#endif // defined(__APPLE__)

using namespace MplusM;
using namespace MplusM::Common;
using std::cout;
using std::endl;

#if defined(__APPLE__)
# pragma mark Private structures, constants and variables
#endif // defined(__APPLE__)

/*! @brief Whether or not metrics are initially being gathered. */
#if defined(MpM_MetricsInitiallyOn)
# define MEASUREMENTS_ON_ true
#else // ! defined(MpM_MetricsInitiallyOn)
# define MEASUREMENTS_ON_ false
#endif // ! defined(MpM_MetricsInitiallyOn)

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

BaseService::BaseService(const ServiceKind  theKind,
                         const YarpString & launchPath,
                         const int          argc,
                         char * *           argv,
                         const YarpString & tag,
                         const bool         useMultipleHandlers,
                         const YarpString & canonicalName,
                         const YarpString & description,
                         const YarpString & requestsDescription,
                         const YarpString & serviceEndpointName,
                         const YarpString & servicePortNumber) :
    _launchPath(launchPath), _contextsLock(), _requestHandlers(*this), _contexts(),
    _description(description), _requestsDescription(requestsDescription), _tag(tag),
    _auxCounters(), _argumentsHandler(NULL), _channelsHandler(NULL), _clientsHandler(NULL),
    _detachHandler(NULL), _extraInfoHandler(NULL), _infoHandler(NULL), _listHandler(NULL),
    _metricsHandler(NULL), _metricsStateHandler(NULL), _nameHandler(NULL),
    _setMetricsStateHandler(NULL), _stopHandler(NULL), _endpoint(NULL), _handler(NULL),
    _handlerCreator(NULL), _pinger(NULL), _kind(theKind), _metricsEnabled(MEASUREMENTS_ON_),
    _started(false), _useMultipleHandlers(useMultipleHandlers)
{
    OD_LOG_ENTER(); //####
    OD_LOG_S4s("launchPath = ", launchPath, "canonicalName = ", canonicalName, //####
               "description = ", description, "requestsDescription = ", requestsDescription); //####
    OD_LOG_S2s("serviceEndpointName = ", serviceEndpointName, "servicePortNumber = ", //####
               servicePortNumber); //####
    OD_LOG_LL1("argc = ", argc); //####
    OD_LOG_P1("argv = ", argv); //####
    OD_LOG_B1("useMultipleHandlers = ", useMultipleHandlers); //####
    if (0 < _tag.length())
    {
        _serviceName = canonicalName + " " + _tag;
    }
    else
    {
        _serviceName = canonicalName;
    }
    _endpoint = new Endpoint(serviceEndpointName, servicePortNumber);
    if (_metricsEnabled)
    {
        _endpoint->enableMetrics();
    }
    else
    {
        _endpoint->disableMetrics();
    }
    for (int ii = 0; argc > ii; ++ii)
    {
        _originalArguments.push_back(argv[ii]);
    }
    attachRequestHandlers();
    OD_LOG_EXIT_P(this); //####
} // BaseService::BaseService

#if (! MAC_OR_LINUX_)
# pragma warning(push)
# pragma warning(disable: 4100)
#endif // ! MAC_OR_LINUX_
BaseService::BaseService(const ServiceKind  theKind,
                         const YarpString & launchPath,
                         const int          argc,
                         char * *           argv,
                         const bool         useMultipleHandlers,
                         const YarpString & canonicalName,
                         const YarpString & description,
                         const YarpString & requestsDescription) :
    _launchPath(launchPath), _contextsLock(), _requestHandlers(*this), _contexts(),
    _description(description), _requestsDescription(requestsDescription),
    _serviceName(canonicalName), _tag(), _auxCounters(), _argumentsHandler(NULL),
    _channelsHandler(NULL), _clientsHandler(NULL), _detachHandler(NULL), _infoHandler(NULL),
    _listHandler(NULL), _metricsHandler(NULL), _metricsStateHandler(NULL), _nameHandler(NULL),
    _setMetricsStateHandler(NULL), _stopHandler(NULL), _endpoint(NULL), _handler(NULL),
    _handlerCreator(NULL), _pinger(NULL), _kind(theKind), _metricsEnabled(MEASUREMENTS_ON_),
    _started(false), _useMultipleHandlers(useMultipleHandlers)
{
#if (! defined(OD_ENABLE_LOGGING_))
# if MAC_OR_LINUX_
#  pragma unused(requestsDescription)
# endif // MAC_OR_LINUX_
#endif // ! defined(OD_ENABLE_LOGGING_)
    OD_LOG_ENTER(); //####
    OD_LOG_S4s("launchPath = ", launchPath, "canonicalName = ", canonicalName, //####
               "description = ", description, "requestsDescription = ", requestsDescription); //####
    OD_LOG_LL1("argc = ", argc); //####
    OD_LOG_P1("argv = ", argv); //####
    OD_LOG_B1("useMultipleHandlers = ", useMultipleHandlers); //####
    switch (argc)
    {
            // Argument order = endpoint name [, port]
        case 1 :
            _endpoint = new Endpoint(*argv);
            break;
            
        case 2 :
            _endpoint = new Endpoint(*argv, argv[1]);
            break;
            
        default :
            OD_LOG_EXIT_THROW_S("Invalid parameters for service endpoint"); //####
            throw new Exception("Invalid parameters for service endpoint");
            
    }
    if (_metricsEnabled)
    {
        _endpoint->enableMetrics();
    }
    else
    {
        _endpoint->disableMetrics();
    }
    for (int ii = 0; argc > ii; ++ii)
    {
        _originalArguments.push_back(argv[ii]);
    }
    attachRequestHandlers();
    OD_LOG_EXIT_P(this); //####
} // BaseService::BaseService
#if (! MAC_OR_LINUX_)
# pragma warning(pop)
#endif // ! MAC_OR_LINUX_

BaseService::~BaseService(void)
{
    OD_LOG_OBJENTER(); //####
    stopService();
    detachRequestHandlers();
    delete _endpoint;
    delete _handler;
    delete _handlerCreator;
    clearContexts();
    OD_LOG_OBJEXIT(); //####
} // BaseService::~BaseService

#if defined(__APPLE__)
# pragma mark Actions and Accessors
#endif // defined(__APPLE__)

void
BaseService::addContext(const YarpString & key,
                        BaseContext *      context)
{
    OD_LOG_OBJENTER(); //####
    OD_LOG_S1s("key = ", key); //####
    OD_LOG_P1("context = ", context); //####
    try
    {
        if (context)
        {
            lockContexts();
            _contexts.insert(ContextMapValue(key, context));
            unlockContexts();
        }
    }
    catch (...)
    {
        OD_LOG("Exception caught"); //####
        throw;
    }
    OD_LOG_OBJEXIT(); //####
} // BaseService::addContext

void
BaseService::attachRequestHandlers(void)
{
    OD_LOG_OBJENTER(); //####
    try
    {
        _argumentsHandler = new ArgumentsRequestHandler(*this);
        _channelsHandler = new ChannelsRequestHandler(*this);
        _clientsHandler = new ClientsRequestHandler(*this);
        _detachHandler = new DetachRequestHandler(*this);
        _extraInfoHandler = new ExtraInfoRequestHandler(*this);
        _infoHandler = new InfoRequestHandler(*this);
        _listHandler = new ListRequestHandler(*this);
        _metricsHandler = new MetricsRequestHandler(*this);
        _metricsStateHandler = new MetricsStateRequestHandler(*this);
        _nameHandler = new NameRequestHandler(*this);
        _setMetricsStateHandler = new SetMetricsStateRequestHandler(*this);
        _stopHandler = new StopRequestHandler(*this);
        if (_argumentsHandler && _channelsHandler && _clientsHandler && _detachHandler &&
            _extraInfoHandler && _infoHandler && _listHandler && _metricsHandler &&
            _metricsStateHandler && _nameHandler && _setMetricsStateHandler && _stopHandler)
        {
            _requestHandlers.registerRequestHandler(_argumentsHandler);
            _requestHandlers.registerRequestHandler(_channelsHandler);
            _requestHandlers.registerRequestHandler(_clientsHandler);
            _requestHandlers.registerRequestHandler(_detachHandler);
            _requestHandlers.registerRequestHandler(_extraInfoHandler);
            _requestHandlers.registerRequestHandler(_infoHandler);
            _requestHandlers.registerRequestHandler(_listHandler);
            _requestHandlers.registerRequestHandler(_metricsHandler);
            _requestHandlers.registerRequestHandler(_metricsStateHandler);
            _requestHandlers.registerRequestHandler(_nameHandler);
            _requestHandlers.registerRequestHandler(_setMetricsStateHandler);
            _requestHandlers.registerRequestHandler(_stopHandler);
        }
        else
        {
            OD_LOG("! (_argumentsHandler && _channelsHandler && _clientsHandler && " //####
                   "_detachHandler && _extraInfoHandler && _infoHandler && _listHandler && " //####
                   "_metricsHandler && _metricsStateHandler && _nameHandler && " //####
                   "_setMetricsStateHandler && _stopHandler)"); //####
        }
    }
    catch (...)
    {
        OD_LOG("Exception caught"); //####
        throw;
    }
    OD_LOG_OBJEXIT(); //####
} // BaseService::attachRequestHandlers

void
BaseService::clearContexts(void)
{
    OD_LOG_OBJENTER(); //####
    lockContexts();
    if (0 < _contexts.size())
    {
        for (ContextMap::const_iterator walker(_contexts.begin()); _contexts.end() != walker;
             ++walker)
        {
            BaseContext * value = walker->second;
            
            if (value)
            {
                delete value;
            }
        }
        _contexts.clear();
    }
    unlockContexts();
    OD_LOG_OBJEXIT(); //####
} // BaseService::clearContexts

void
BaseService::detachClient(const YarpString & key)
{
    OD_LOG_OBJENTER(); //####
    OD_LOG_S1s("key = ", key); //####
    try
    {
        removeContext(key);
    }
    catch (...)
    {
        OD_LOG("Exception caught"); //####
        throw;
    }
    OD_LOG_OBJEXIT(); //####
} // BaseService::detachClient

void
BaseService::detachRequestHandlers(void)
{
    OD_LOG_OBJENTER(); //####
    try
    {
        if (_argumentsHandler)
        {
            _requestHandlers.unregisterRequestHandler(_argumentsHandler);
            delete _argumentsHandler;
            _argumentsHandler = NULL;
        }
        if (_channelsHandler)
        {
            _requestHandlers.unregisterRequestHandler(_channelsHandler);
            delete _channelsHandler;
            _channelsHandler = NULL;
        }
        if (_clientsHandler)
        {
            _requestHandlers.unregisterRequestHandler(_clientsHandler);
            delete _clientsHandler;
            _clientsHandler = NULL;
        }
        if (_detachHandler)
        {
            _requestHandlers.unregisterRequestHandler(_detachHandler);
            delete _detachHandler;
            _detachHandler = NULL;
        }
        if (_extraInfoHandler)
        {
            _requestHandlers.unregisterRequestHandler(_extraInfoHandler);
            delete _extraInfoHandler;
            _extraInfoHandler = NULL;
        }
        if (_infoHandler)
        {
            _requestHandlers.unregisterRequestHandler(_infoHandler);
            delete _infoHandler;
            _infoHandler = NULL;
        }
        if (_listHandler)
        {
            _requestHandlers.unregisterRequestHandler(_listHandler);
            delete _listHandler;
            _listHandler = NULL;
        }
        if (_metricsHandler)
        {
            _requestHandlers.unregisterRequestHandler(_metricsHandler);
            delete _metricsHandler;
            _metricsHandler = NULL;
        }
        if (_metricsStateHandler)
        {
            _requestHandlers.unregisterRequestHandler(_metricsStateHandler);
            delete _metricsStateHandler;
            _metricsStateHandler = NULL;
        }
        if (_nameHandler)
        {
            _requestHandlers.unregisterRequestHandler(_nameHandler);
            delete _nameHandler;
            _nameHandler = NULL;
        }
        if (_setMetricsStateHandler)
        {
            _requestHandlers.unregisterRequestHandler(_setMetricsStateHandler);
            delete _setMetricsStateHandler;
            _setMetricsStateHandler = NULL;
        }
        if (_stopHandler)
        {
            _requestHandlers.unregisterRequestHandler(_stopHandler);
            delete _stopHandler;
            _stopHandler = NULL;
        }
    }
    catch (...)
    {
        OD_LOG("Exception caught"); //####
        throw;
    }
    OD_LOG_OBJEXIT(); //####
} // BaseService::detachRequestHandlers

DEFINE_DISABLEMETRICS_(BaseService)
{
    OD_LOG_OBJENTER(); //####
    _metricsEnabled = false;
    if (_endpoint)
    {
        _endpoint->disableMetrics();
    }
    OD_LOG_OBJEXIT(); //####
} // BaseService::disableMetrics

DEFINE_ENABLEMETRICS_(BaseService)
{
    OD_LOG_OBJENTER(); //####
    if (_endpoint)
    {
        _endpoint->enableMetrics();
    }
    _metricsEnabled = true;
    OD_LOG_OBJEXIT(); //####
} // BaseService::enableMetrics

void
BaseService::fillInClientList(YarpStringVector & clients)
{
    OD_LOG_OBJENTER(); //####
    OD_LOG_P1("clients = ", &clients); //####
    lockContexts();
    if (0 < _contexts.size())
    {
        for (ContextMap::const_iterator walker(_contexts.begin()); _contexts.end() != walker;
             ++walker)
        {
            clients.push_back(walker->first.c_str());
        }
    }
    unlockContexts();
    OD_LOG_OBJEXIT(); //####
} // BaseService::fillInClientList

DEFINE_FILLINSECONDARYCLIENTCHANNELSLIST_(BaseService)
{
    OD_LOG_OBJENTER(); //####
    OD_LOG_P1("channels = ", &channels); //####
    channels.clear();
    OD_LOG_OBJEXIT(); //####
} // BaseService::fillInSecondaryClientChannelsList

DEFINE_FILLINSECONDARYINPUTCHANNELSLIST_(BaseService)
{
    OD_LOG_OBJENTER(); //####
    OD_LOG_P1("channels = ", &channels); //####
    channels.clear();
    OD_LOG_OBJEXIT(); //####
} // BaseService::fillInSecondaryInputChannelsList

DEFINE_FILLINSECONDARYOUTPUTCHANNELSLIST_(BaseService)
{
    OD_LOG_OBJENTER(); //####
    OD_LOG_P1("channels = ", &channels); //####
    channels.clear();
    OD_LOG_OBJEXIT(); //####
} // BaseService::fillInSecondaryOutputChannelsList

BaseContext *
BaseService::findContext(const YarpString & key)
{
    OD_LOG_OBJENTER(); //####
    OD_LOG_S1s("key = ", key); //####
    BaseContext * result = NULL;
    
    try
    {
        lockContexts();
        ContextMap::iterator match(_contexts.find(key));
        
        if (_contexts.end() != match)
        {
            result = match->second;
        }
        unlockContexts();
    }
    catch (...)
    {
        OD_LOG("Exception caught"); //####
        throw;
    }
    OD_LOG_OBJEXIT_P(result); //####
    return result;
} // BaseService::findContext

DEFINE_GATHERMETRICS_(BaseService)
{
    OD_LOG_OBJENTER(); //####
    OD_LOG_P1("metrics = ", &metrics); //####
    if (_endpoint)
    {
        SendReceiveCounters counters;
    
        _endpoint->getSendReceiveCounters(counters);
        counters.addToList(metrics, _endpoint->getName());
    }
    _auxCounters.addToList(metrics, "auxiliary");
    OD_LOG_OBJEXIT(); //####
} // BaseService::gatherMetrics

void
BaseService::incrementAuxiliaryCounters(const SendReceiveCounters & additionalCounters)
{
    OD_LOG_OBJENTER(); //####
    OD_LOG_P1("additionalCounters = ", &additionalCounters); //####
    _auxCounters += additionalCounters;
    OD_LOG_OBJEXIT(); //####
} // BaseService::incrementAuxiliaryCounters

bool
BaseService::processRequest(const YarpString &           request,
                            const yarp::os::Bottle &     restOfInput,
                            const YarpString &           senderChannel,
                            yarp::os::ConnectionWriter * replyMechanism)
{
    OD_LOG_OBJENTER(); //####
    OD_LOG_S3s("request = ", request, "restOfInput = ", restOfInput.toString(), //####
               "senderChannel = ", senderChannel); //####
    OD_LOG_P1("replyMechanism = ", replyMechanism); //####
    bool result = true;
    
    try
    {
        BaseRequestHandler * handler = _requestHandlers.lookupRequestHandler(request);
        
        if (handler)
        {
            OD_LOG("(handler)"); //####
            result = handler->processRequest(request, restOfInput, senderChannel, replyMechanism);
        }
        else
        {
            OD_LOG("! (handler)"); //####
            if (replyMechanism)
            {
                OD_LOG("(replyMechanism)"); //####
                yarp::os::Bottle errorMessage(MpM_UNRECOGNIZED_REQUEST_);
                
                OD_LOG_S1s("errorMessage <- ", errorMessage.toString()); //####
                if (! errorMessage.write(*replyMechanism))
                {
                    OD_LOG("(! errorMessage.write(*replyMechanism))"); //####
#if defined(MpM_StallOnSendProblem)
                    Stall();
#endif // defined(MpM_StallOnSendProblem)
                }
            }
            result = false;
        }
    }
    catch (...)
    {
        OD_LOG("Exception caught"); //####
        throw;
    }
    OD_LOG_OBJEXIT_B(result); //####
    return result;
} // BaseService::processRequest

void
BaseService::registerRequestHandler(BaseRequestHandler * handler)
{
    OD_LOG_OBJENTER(); //####
    OD_LOG_P1("handler = ", handler); //####
    _requestHandlers.registerRequestHandler(handler);
    OD_LOG_OBJEXIT(); //####
} // BaseService::registerRequestHandler

void
BaseService::removeContext(const YarpString & key)
{
    OD_LOG_OBJENTER(); //####
    OD_LOG_S1s("key = ", key); //####
    try
    {
        lockContexts();
        ContextMap::iterator match(_contexts.find(key));
        
        if (_contexts.end() != match)
        {
            BaseContext * value = match->second;
            
            if (value)
            {
                delete value;
            }
            _contexts.erase(match);
        }
        unlockContexts();
    }
    catch (...)
    {
        OD_LOG("Exception caught"); //####
        throw;
    }
    OD_LOG_OBJEXIT(); //####
} // BaseService::removeContext

bool
BaseService::sendPingForChannel(const YarpString & channelName,
                                CheckFunction      checker,
                                void *             checkStuff)
{
    OD_LOG_OBJENTER(); //####
    OD_LOG_S1s("channelName = ", channelName); //####
    OD_LOG_P1("checkStuff = ", checkStuff); //####
    bool result = false;
    
    try
    {
        YarpString              aName(GetRandomChannelName(HIDDEN_CHANNEL_PREFIX_
                                                           BUILD_NAME_("ping_",
                                                                       DEFAULT_CHANNEL_ROOT_)));
        ClientChannel *         newChannel = new ClientChannel;
#if defined(MpM_ReportOnConnections)
        ChannelStatusReporter * reporter = Utilities::GetGlobalStatusReporter();
#endif // defined(MpM_ReportOnConnections)
        
        if (newChannel)
        {
            if (_metricsEnabled)
            {
                newChannel->enableMetrics();
            }
            else
            {
                newChannel->disableMetrics();
            }
#if defined(MpM_ReportOnConnections)
            newChannel->setReporter(*reporter);
            newChannel->getReport(*reporter);
#endif // defined(MpM_ReportOnConnections)
            if (newChannel->openWithRetries(aName, STANDARD_WAIT_TIME_))
            {
                if (Utilities::NetworkConnectWithRetries(aName, MpM_REGISTRY_ENDPOINT_NAME_,
                                                         STANDARD_WAIT_TIME_, false, checker,
                                                         checkStuff))
                {
                    yarp::os::Bottle parameters(channelName);
                    ServiceRequest   request(MpM_PING_REQUEST_, parameters);
                    ServiceResponse  response;
                    
                    if (request.send(*newChannel, response))
                    {
                        // Check that we got a successful ping!
                        if (MpM_EXPECTED_PING_RESPONSE_SIZE_ == response.count())
                        {
                            yarp::os::Value theValue = response.element(0);
                            
                            if (theValue.isString())
                            {
                                result = (theValue.toString() == MpM_OK_RESPONSE_);
                            }
                            else
                            {
                                OD_LOG("! (theValue.isString())"); //####
                            }
                        }
                        else
                        {
                            OD_LOG("! (MpM_EXPECTED_PING_RESPONSE_SIZE_ == " //####
                                   "response.count())"); //####
                            OD_LOG_S1s("response = ", response.asString()); //####
                        }
                    }
                    else
                    {
                        OD_LOG("! (request.send(*newChannel, response))"); //####
                    }
#if defined(MpM_DoExplicitDisconnect)
                    if (! Utilities::NetworkDisconnectWithRetries(aName,
                                                                  MpM_REGISTRY_ENDPOINT_NAME_,
                                                                  STANDARD_WAIT_TIME_, checker,
                                                                  checkStuff))
                    {
                        OD_LOG("(! Utilities::NetworkDisconnectWithRetries(aName, " //####
                               "MpM_REGISTRY_ENDPOINT_NAME_, STANDARD_WAIT_TIME_, checker, " //####
                               "checkStuff))"); //####
                    }
#endif // defined(MpM_DoExplicitDisconnect)
                }
                else
                {
                    OD_LOG("! (Utilities::NetworkConnectWithRetries(aName, " //####
                           "MpM_REGISTRY_ENDPOINT_NAME_, STANDARD_WAIT_TIME_, false, " //####
                           "checker, checkStuff))"); //####
                }
#if defined(MpM_DoExplicitClose)
                newChannel->close();
#endif // defined(MpM_DoExplicitClose)
            }
            else
            {
                OD_LOG("! (newChannel->openWithRetries(aName, STANDARD_WAIT_TIME_))"); //####
            }
            SendReceiveCounters newCounters;
            
            newChannel->getSendReceiveCounters(newCounters);
            incrementAuxiliaryCounters(newCounters);
            BaseChannel::RelinquishChannel(newChannel);
        }
        else
        {
            OD_LOG("! (newChannel)"); //####
        }
    }
    catch (...)
    {
        OD_LOG("Exception caught"); //####
        throw;
    }
    OD_LOG_OBJEXIT_B(result); //####
    return result;
} // BaseService::sendPingForChannel

void
BaseService::setDefaultRequestHandler(BaseRequestHandler * handler)
{
    OD_LOG_OBJENTER(); //####
    OD_LOG_P1("handler = ", handler); //####
    _requestHandlers.setDefaultRequestHandler(handler);
    OD_LOG_OBJEXIT(); //####
} // BaseService::setDefaultRequestHandler

void
BaseService::setExtraInformation(const YarpString & extraInfo)
{
    OD_LOG_OBJENTER(); //####
    OD_LOG_S1s("extraInfo = ", extraInfo); //####
    _extraInfo = extraInfo;
    OD_LOG_OBJEXIT(); //####
} // BaseService::setExtraInformation

DEFINE_STARTSERVICE_(BaseService)
{
    OD_LOG_OBJENTER(); //####
    try
    {
        if (! _started)
        {
            if (_useMultipleHandlers)
            {
                _handlerCreator = new ServiceInputHandlerCreator(*this);
                if (_handlerCreator)
                {
                    if (_endpoint->setInputHandlerCreator(*_handlerCreator) &&
                        _endpoint->open(STANDARD_WAIT_TIME_))
                    {
                        _started = true;
                    }
                    else
                    {
                        OD_LOG("! (_endpoint->setInputHandlerCreator(*_handlerCreator) && " //####
                               "_endpoint->open(STANDARD_WAIT_TIME_))"); //####
                        delete _handlerCreator;
                        _handlerCreator = NULL;
                    }
                }
                else
                {
                    OD_LOG("! (_handlerCreator)"); //####
                }
            }
            else
            {
                _handler = new ServiceInputHandler(*this);
                if (_handler)
                {
                    if (_endpoint->setInputHandler(*_handler) &&
                        _endpoint->open(STANDARD_WAIT_TIME_))
                    {
                        _started = true;
                    }
                    else
                    {
                        OD_LOG("! (_endpoint->setInputHandler(*_handler) && " //####
                               "_endpoint->open(STANDARD_WAIT_TIME_))"); //####
                        delete _handler;
                        _handler = NULL;
                    }
                }
                else
                {
                    OD_LOG("! (_handler)"); //####
                }
            }
        }
    }
    catch (...)
    {
        OD_LOG("Exception caught"); //####
        throw;
    }
    OD_LOG_OBJEXIT_B(_started); //####
    return _started;
} // BaseService::startService

void
BaseService::startPinger(void)
{
    OD_LOG_OBJENTER(); //####
    if ((! _pinger) && _endpoint)
    {
        _pinger = new PingThread(_endpoint->getName(), *this);
        if (! _pinger->start())
        {
            OD_LOG("(! _pinger->start())"); //####
            delete _pinger;
            _pinger = NULL;
        }
    }
    OD_LOG_OBJEXIT(); //####
} // BaseService::startPinger

DEFINE_STOPSERVICE_(BaseService)
{
    OD_LOG_OBJENTER(); //####
    if (_pinger)
    {
        _pinger->stop();
        delete _pinger;
        _pinger = NULL;
    }
    _started = false;
    OD_LOG_OBJEXIT_B(! _started); //####
    return (! _started);
} // BaseService::stopService

void
BaseService::unregisterRequestHandler(BaseRequestHandler * handler)
{
    OD_LOG_OBJENTER(); //####
    OD_LOG_P1("handler = ", handler); //####
    _requestHandlers.unregisterRequestHandler(handler);
    OD_LOG_OBJEXIT(); //####
} // BaseService::unregisterRequestHandler

void
BaseService::updateResponseCounters(const size_t numBytes)
{
    OD_LOG_OBJENTER(); //####
    OD_LOG_LL1("numBytes = ", numBytes); //####
    if (_endpoint && _metricsEnabled)
    {
        _endpoint->updateSendCounters(numBytes);
    }
    OD_LOG_OBJEXIT(); //####
} // BaseService::updateResponseCounters

#if defined(__APPLE__)
# pragma mark Global functions
#endif // defined(__APPLE__)

bool
Common::AdjustEndpointName(const YarpString &       defaultEndpointNameRoot,
                           const AddressTagModifier modFlag,
                           YarpString &             tag,
                           YarpString &             serviceEndpointName,
                           const YarpString &       tagModifier)
{
    OD_LOG_ENTER(); //####
    OD_LOG_S4s("defaultEndpointNameRoot = ", defaultEndpointNameRoot, //####
               "tag = ", tag, "serviceEndpointName = ", serviceEndpointName, //####
               "tagModifier = ", tagModifier); //####
    bool       nameWasSet = false;
    YarpString trimmedModifier;
    
    if (kModificationNone != modFlag)
    {
        NetworkAddress    ourAddress;
        std::stringstream buff;
        YarpString        addressModifier;
        
        GetOurEffectiveAddress(ourAddress);
        switch (modFlag)
        {
            case kModificationBottomByte :
                buff << ourAddress._ipBytes[3];
                break;
                
            case kModificationBottomTwoBytes :
                buff << ourAddress._ipBytes[2] << "." << ourAddress._ipBytes[3];
                break;
                
            case kModificationBottomThreeBytes :
                buff << ourAddress._ipBytes[1] << "." << ourAddress._ipBytes[2] << "." <<
                        ourAddress._ipBytes[3];
                break;
                
            case kModificationAllBytes :
                buff << ourAddress._ipBytes[0] << "." << ourAddress._ipBytes[1] << "." <<
                        ourAddress._ipBytes[2] << "." << ourAddress._ipBytes[3];
                break;
                
            default :
                break;            
                
        }
        addressModifier = YarpString(buff.str());
        if (0 < tag.length())
        {
            tag += YarpString(".") + addressModifier;
        }
        else
        {
            tag = addressModifier;
        }
    }
    if (0 < tagModifier.length())
    {
        char lastChar = tagModifier[tagModifier.length() - 1];
        
        // Drop a trailing period, if present.
        if ('.' == lastChar)
        {
            trimmedModifier = tagModifier.substr(0, tagModifier.length() - 1);
        }
        else
        {
            trimmedModifier = tagModifier;
        }
    }
    if (0 < serviceEndpointName.length())
    {
        nameWasSet = true;
    }
    else
    {
        if (0 < tag.length())
        {
            serviceEndpointName = defaultEndpointNameRoot + "/" + tag;
        }
        else
        {
            serviceEndpointName = defaultEndpointNameRoot;
        }
        if (0 < trimmedModifier.length())
        {
            serviceEndpointName += YarpString("/") + trimmedModifier;
        }
    }
    if (0 < tag.length())
    {
        if (0 < trimmedModifier.length())
        {
            tag += YarpString(":") + trimmedModifier;
        }
    }
    else
    {
        tag = trimmedModifier;
    }
    OD_LOG_EXIT_B(nameWasSet); //####
    return nameWasSet;
} // Common::AdjustEndpointName

bool
Common::GetOurEffectiveAddress(NetworkAddress & ourAddress)
{
    OD_LOG_ENTER(); //####
    OD_LOG_P1("convertedAddress = ", &convertedAddress); //####
    bool okSoFar = false;
    
    try
    {
        YarpString    aName(GetRandomChannelName(HIDDEN_CHANNEL_PREFIX_
                                                 BUILD_NAME_("whereAmI_",
                                                             DEFAULT_CHANNEL_ROOT_)));
        BaseChannel * newChannel = new BaseChannel;
        
        if (newChannel)
        {
            if (newChannel->openWithRetries(aName, STANDARD_WAIT_TIME_))
            {
                yarp::os::Contact aContact(newChannel->where());
                YarpString        hostName(aContact.getHost());
                struct in_addr    rawAddress;
                
                if (hostName == SELF_ADDRESS_NAME_)
                {
                    hostName = SELF_ADDRESS_IPADDR_;
                }
#if MAC_OR_LINUX_
                int res = inet_pton(AF_INET, hostName.c_str(), &rawAddress);
#else // ! MAC_OR_LINUX_
                int res = InetPton(AF_INET, hostName.c_str(), &rawAddress);
#endif // ! MAC_OR_LINUX_
                
                if (0 < res)
                {
                    char              buffer[INET_ADDRSTRLEN + 5];
#if MAC_OR_LINUX_
                    const char *      converted = inet_ntop(AF_INET, &rawAddress, buffer,
                                                            sizeof(buffer));
#else // ! MAC_OR_LINUX_
                    const char *      converted = InetNtop(AF_INET, &rawAddress, buffer,
                                                           sizeof(buffer));
#endif // ! MAC_OR_LINUX_
                    std::stringstream buff(converted);
                    char              sep_0_1;
                    char              sep_1_2;
                    char              sep_2_3;
                    
                    ourAddress._ipPort = aContact.getPort();
                    buff >> ourAddress._ipBytes[0] >> sep_0_1 >> ourAddress._ipBytes[1] >>
                    sep_1_2 >> ourAddress._ipBytes[2] >> sep_2_3 >> ourAddress._ipBytes[3];
                    okSoFar = (! buff.fail());
                }
#if defined(MpM_DoExplicitClose)
                newChannel->close();
#endif // defined(MpM_DoExplicitClose)
            }
            else
            {
                OD_LOG("! (newChannel->openWithRetries(aName, STANDARD_WAIT_TIME_))"); //####
            }
            BaseChannel::RelinquishChannel(newChannel);
        }
        else
        {
            OD_LOG("! (newChannel)"); //####
        }
    }
    catch (...)
    {
        OD_LOG("Exception caught"); //####
        throw;
    }
    OD_LOG_OBJEXIT_B(okSoFar); //####
    return okSoFar;
} // Common::GetOurEffectiveAddress

bool
Common::ProcessStandardServiceOptions(const int                     argc,
                                      char * *                      argv,
                                      Utilities::DescriptorVector & argumentDescriptions,
                                      const YarpString &            serviceDescription,
                                      const YarpString &            matchingCriteria,
                                      const int                     year,
                                      const char *                  copyrightHolder,
                                      bool &                        goWasSet,
                                      bool &                        reportEndpoint,
                                      bool &                        reportOnExit,
                                      YarpString &                  tag,
                                      YarpString &                  serviceEndpointName,
                                      YarpString &                  servicePortNumber,
                                      AddressTagModifier &          modFlag,
                                      const OptionsMask             skipOptions,
                                      YarpStringVector *            arguments)
{
    OD_LOG_ENTER(); //####
    OD_LOG_L2("argc = ", argc, "year = ", year); //####
    OD_LOG_P4("argv = ", argv, "argumentDescriptions = ", &argumentDescriptions, //####
              "reportEndpoint = ", &reportEndpoint, "reportOnExit = ", &reportOnExit); //####
    OD_LOG_P2("modFlag = ", &modFlag, "arguments = ", arguments); //####
    OD_LOG_S2s("serviceDescription = ", serviceDescription, "matchingCriteria = ", //####
               matchingCriteria); //####
    OD_LOG_S1("copyrightHolder = ", copyrightHolder); //####
    enum optionIndex
    {
        kOptionUNKNOWN,
        kOptionARGS,
        kOptionCHANNEL,
        kOptionENDPOINT,
        kOptionGO,
        kOptionHELP,
        kOptionINFO,
        kOptionMOD,
        kOptionPORT,
        kOptionREPORT,
        kOptionTAG,
        kOptionVERSION
    }; // optionIndex
    
    bool       isAdapter = (0 < matchingCriteria.length());
    bool       keepGoing = true;
    YarpString serviceKindName(isAdapter ? "adapter" : "service");
    YarpString goPartText("  --go, -g          Start the ");
    YarpString infoPartText("  --info, -i        Print executable type, supported ");
    YarpString reportPartText("  --report, -r      Report the ");
    YarpString tagPartText("  --tag, -t         Specify the tag to be used as part of the ");
    
    goPartText += serviceKindName + " immediately";
    infoPartText += serviceKindName + " options";
    if (isAdapter)
    {
        infoPartText += ", matching criteria";
    }
    infoPartText += " and description and exit";
    reportPartText += serviceKindName + " metrics when the application exits";
    tagPartText += serviceKindName + " name";
    Option_::Descriptor   firstDescriptor(kOptionUNKNOWN, 0, "", "", Option_::Arg::None, NULL);
    Option_::Descriptor   argsDescriptor(kOptionARGS, 0, "a", "args", Option_::Arg::None,
                                         T_("  --args, -a        Report the argument formats"));
    Option_::Descriptor   channelDescriptor(kOptionCHANNEL, 0, "c", "channel", Option_::Arg::None,
                                            T_("  --channel, -c     Report the actual endpoint "
                                               "name"));
    Option_::Descriptor   endpointDescriptor(kOptionENDPOINT, 0, "e", "endpoint",
                                             Option_::Arg::Required,
                                             T_("  --endpoint, -e    Specify an alternative "
                                                "endpoint name to be used"));
    Option_::Descriptor   goDescriptor(kOptionGO, 0, "g", "go", Option_::Arg::None,
                                       goPartText.c_str());
    Option_::Descriptor   helpDescriptor(kOptionHELP, 0, "h", "help", Option_::Arg::None,
                                         T_("  --help, -h        Print usage and exit"));
    Option_::Descriptor   infoDescriptor(kOptionINFO, 0, "i", "info", Option_::Arg::None,
                                         infoPartText.c_str());
    Option_::Descriptor   modDescriptor(kOptionMOD, 0, "m", "mod", Option_::Arg::Required,
                                        "  --mod, -m         Use the IP address as a modifier for "
                                        "the tag");
    Option_::Descriptor   portDescriptor(kOptionPORT, 0, "p", "port", Option_::Arg::Required,
                                         T_("  --port, -p        Specify a non-default port to be "
                                            "used"));
    Option_::Descriptor   reportDescriptor(kOptionREPORT, 0, "r", "report", Option_::Arg::None,
                                           reportPartText.c_str());
    Option_::Descriptor   tagDescriptor(kOptionTAG, 0, "t", "tag", Option_::Arg::Required,
                                        tagPartText.c_str());
    Option_::Descriptor   versionDescriptor(kOptionVERSION, 0, "v", "vers", Option_::Arg::None,
                                            T_("  --vers, -v        Print version information and "
                                               "exit"));
    Option_::Descriptor   lastDescriptor(0, 0, NULL, NULL, NULL, NULL);
    int                   argcWork = argc;
    char * *              argvWork = argv;
    YarpString            usageString("USAGE: ");
    YarpString            argList(ArgumentsToArgString(argumentDescriptions));

    reportEndpoint = reportOnExit = goWasSet = false;
    tag = serviceEndpointName = serviceEndpointName = "";
    modFlag = kModificationNone;
    if (arguments)
    {
        arguments->clear();
    }
    usageString += *argv;
    usageString += " [options]";
    if (0 < argList.length())
    {
        YarpStringVector descriptions;

        Utilities::ArgumentsToDescriptionArray(argumentDescriptions, descriptions, 2);
        usageString += " ";
        usageString += argList + "\n\n";
        for (size_t ii = 0, mm = descriptions.size(); mm > ii; ++ii)
        {
            if (0 < ii)
            {
                usageString += "\n";
            }
            usageString += "  ";
            usageString += descriptions[ii];
        }
    }
    usageString += "\n\nOptions:";
    // firstDescriptor, helpDescriptor, versionDescriptor, lastDescriptor
    size_t descriptorCount = 4;
    
    if (! (skipOptions & kSkipArgsOption))
    {
        ++descriptorCount;
    }
    if (! (skipOptions & kSkipChannelOption))
    {
        ++descriptorCount;
    }
    if (! (skipOptions & kSkipEndpointOption))
    {
        ++descriptorCount;
    }
    if (! (skipOptions & kSkipGoOption))
    {
        ++descriptorCount;
    }
    if (! (skipOptions & kSkipInfoOption))
    {
        ++descriptorCount;
    }
    if (! (skipOptions & kSkipModOption))
    {
        ++descriptorCount;
    }
    if (! (skipOptions & kSkipPortOption))
    {
        ++descriptorCount;
    }
    if (! (skipOptions & kSkipReportOption))
    {
        ++descriptorCount;
    }
    if (! (skipOptions & kSkipTagOption))
    {
        ++descriptorCount;
    }
    Option_::Descriptor * usage = new Option_::Descriptor[descriptorCount];
    Option_::Descriptor * usageWalker = usage;
    
#if MAC_OR_LINUX_
    firstDescriptor.help = strdup(usageString.c_str());
#else // ! MAC_OR_LINUX_
    firstDescriptor.help = _strdup(usageString.c_str());
#endif // ! MAC_OR_LINUX_
    memcpy(usageWalker++, &firstDescriptor, sizeof(firstDescriptor));
    if (! (skipOptions & kSkipArgsOption))
    {
        memcpy(usageWalker++, &argsDescriptor, sizeof(argsDescriptor));
    }
    if (! (skipOptions & kSkipChannelOption))
    {
        memcpy(usageWalker++, &channelDescriptor, sizeof(channelDescriptor));
    }
    if (! (skipOptions & kSkipEndpointOption))
    {
        memcpy(usageWalker++, &endpointDescriptor, sizeof(endpointDescriptor));
    }
    if (! (skipOptions & kSkipGoOption))
    {
        memcpy(usageWalker++, &goDescriptor, sizeof(goDescriptor));
    }
    memcpy(usageWalker++, &helpDescriptor, sizeof(helpDescriptor));
    if (! (skipOptions & kSkipInfoOption))
    {
        memcpy(usageWalker++, &infoDescriptor, sizeof(infoDescriptor));
    }
    if (! (skipOptions & kSkipModOption))
    {
        memcpy(usageWalker++, &modDescriptor, sizeof(modDescriptor));
    }
    if (! (skipOptions & kSkipPortOption))
    {
        memcpy(usageWalker++, &portDescriptor, sizeof(portDescriptor));
    }
    if (! (skipOptions & kSkipReportOption))
    {
        memcpy(usageWalker++, &reportDescriptor, sizeof(reportDescriptor));
    }
    if (! (skipOptions & kSkipTagOption))
    {
        memcpy(usageWalker++, &tagDescriptor, sizeof(tagDescriptor));
    }
    memcpy(usageWalker++, &versionDescriptor, sizeof(versionDescriptor));
    memcpy(usageWalker++, &lastDescriptor, sizeof(lastDescriptor));
    argcWork -= (argc > 0);
    argvWork += (argc > 0); // skip program name argv[0] if present
    Option_::Stats    stats(usage, argcWork, argvWork);
    Option_::Option * options = new Option_::Option[stats.options_max];
    Option_::Option * buffer = new Option_::Option[stats.buffer_max];
    Option_::Parser   parse(usage, argcWork, argvWork, options, buffer, 1);
    YarpString        badArgs;
    
    if (parse.error())
    {
        OD_LOG("(parse.error())"); //####
        keepGoing = false;
    }
    else if (options[kOptionHELP] || options[kOptionUNKNOWN])
    {
        OD_LOG("(options[kOptionHELP] || options[kOptionUNKNOWN])"); //####
        Option_::printUsage(cout, usage, HELP_LINE_LENGTH_);
        keepGoing = false;
    }
    else if (options[kOptionVERSION])
    {
        OD_LOG("(options[kOptionVERSION])"); //####
        YarpString mpmVersionString(SanitizeString(MpM_VERSION_, true));
        
        cout << "Version " << mpmVersionString.c_str() << ": Copyright (c) " << year << " by " <<
                copyrightHolder << "." << endl;
        keepGoing = false;
    }
    else if (options[kOptionARGS])
    {
        OD_LOG("(options[kOptionARGS])"); //####
        for (size_t ii = 0, mm = argumentDescriptions.size(); mm > ii; ++ii)
        {
            Utilities::BaseArgumentDescriptor * anArg = argumentDescriptions[ii];

            if (0 < ii)
            {
                cout << ARGUMENT_SEPARATOR_;
            }
            if (anArg)
            {
                cout << anArg->toString().c_str();
            }
        }
        cout << endl;
        keepGoing = false;
    }
    else if (options[kOptionINFO])
    {
        OD_LOG("(options[kOptionINFO])"); //####
        bool needTab = true;
        
        // Note that we don't report the 'h' and 'v' options, as they are not involved in
        // determining what choices to offer when launching a service.
        cout << (isAdapter ? "Adapter" : "Service");
        if (! (skipOptions & kSkipArgsOption))
        {
            if (needTab)
            {
                cout << "\t";
                needTab = false;
            }
            cout << "a";
        }
        if (! (skipOptions & kSkipChannelOption))
        {
            if (needTab)
            {
                cout << "\t";
                needTab = false;
            }
            cout << "c";
        }
        if (! (skipOptions & kSkipEndpointOption))
        {
            if (needTab)
            {
                cout << "\t";
                needTab = false;
            }
            cout << "e";
        }
        if (! (skipOptions & kSkipGoOption))
        {
            if (needTab)
            {
                cout << "\t";
                needTab = false;
            }
            cout << "g";
        }
        if (! (skipOptions & kSkipInfoOption))
        {
            if (needTab)
            {
                cout << "\t";
                needTab = false;
            }
            cout << "i";
        }
        if (! (skipOptions & kSkipModOption))
        {
            if (needTab)
            {
                cout << "\t";
                needTab = false;
            }
            cout << "m";
        }
        if (! (skipOptions & kSkipPortOption))
        {
            if (needTab)
            {
                cout << "\t";
                needTab = false;
            }
            cout << "p";
        }
        if (! (skipOptions & kSkipReportOption))
        {
            if (needTab)
            {
                cout << "\t";
                needTab = false;
            }
            cout << "r";
        }
        if (! (skipOptions & kSkipTagOption))
        {
            if (needTab)
            {
                cout << "\t";
                needTab = false;
            }
            cout << "t";
        }
        if (needTab)
        {
            cout << "\t";
        }
        cout << "\t" << matchingCriteria.c_str() << "\t" << serviceDescription.c_str() << endl;
        keepGoing = false;
    }
    else if (ProcessArguments(argumentDescriptions, parse, badArgs))
    {
        OD_LOG("(ProcessArguments(argumentDescriptions, parse, badArgs))"); //####
        if (options[kOptionGO])
        {
            goWasSet = true;
        }
        if (options[kOptionCHANNEL])
        {
            reportEndpoint = true;
        }
        if (options[kOptionMOD])
        {
            YarpString modArg = options[kOptionMOD].arg;
            
            if (0 < modArg.length())
            {
                const char * startPtr = modArg.c_str();
                char *       endPtr;
                int          numBytes = static_cast<int>(strtol(startPtr, &endPtr, 10));
                
                if ((startPtr != endPtr) && (!*endPtr))
                {
                    switch (numBytes)
                    {
                        case 0 :
                            modFlag = kModificationNone;
                            break;
                            
                        case 1 :
                            modFlag = kModificationBottomByte;
                            break;
                            
                        case 2 :
                            modFlag = kModificationBottomTwoBytes;
                            break;
                            
                        case 3 :
                            modFlag = kModificationBottomThreeBytes;
                            break;
                            
                        case 4 :
                            modFlag = kModificationAllBytes;
                            break;
                            
                        default :
                            break;
                            
                    }
                }
            }
        }
        if (options[kOptionREPORT])
        {
            reportOnExit = true;
        }
        if (options[kOptionENDPOINT])
        {
            serviceEndpointName = options[kOptionENDPOINT].arg;
            OD_LOG_S1s("serviceEndpointName <- ", serviceEndpointName); //####
        }
        if (options[kOptionPORT])
        {
            servicePortNumber = options[kOptionPORT].arg;
            OD_LOG_S1s("servicePortNumber <- ", servicePortNumber); //####
            if (0 < servicePortNumber.length())
            {
                const char * startPtr = servicePortNumber.c_str();
                char *       endPtr;
                int          aPort = static_cast<int>(strtol(startPtr, &endPtr, 10));

                if ((startPtr == endPtr) || *endPtr || (! Utilities::ValidPortNumber(aPort)))
                {
                    cout << "Bad port number." << endl;
                    keepGoing = false;
                }
            }
        }
        if (options[kOptionTAG])
        {
            tag = options[kOptionTAG].arg;
            OD_LOG_S1s("tag <- ", tag); //####
        }
        if (arguments)
        {
            for (int ii = 0; ii < parse.nonOptionsCount(); ++ii)
            {
                arguments->push_back(parse.nonOption(ii));
            }
        }
    }
    else
    {
        OD_LOG("! (ProcessArguments(argumentDescriptions, parse, badArgs))"); //####
        cout << "One or more invalid or missing arguments (" << badArgs.c_str() << ")." << endl;
        keepGoing = false;
    }
    delete[] options;
    delete[] buffer;
    free(const_cast<char *>(firstDescriptor.help));
    delete[] usage;
    OD_LOG_EXIT_B(keepGoing); //####
    return keepGoing;
} // Common::ProcessStandardServiceOptions

bool
Common::RegisterLocalService(const YarpString & channelName,
                             BaseService &      service,
                             CheckFunction      checker,
                             void *             checkStuff)
{
    OD_LOG_ENTER(); //####
    OD_LOG_S1s("channelName = ", channelName); //####
    OD_LOG_P2("service = ", &service, "checkStuff = ", checkStuff); //####
    bool result = false;
    
    try
    {
        YarpString              aName(GetRandomChannelName(HIDDEN_CHANNEL_PREFIX_
                                                           BUILD_NAME_("registerlocal_",
                                                                       DEFAULT_CHANNEL_ROOT_)));
        ClientChannel *         newChannel = new ClientChannel;
#if defined(MpM_ReportOnConnections)
        ChannelStatusReporter * reporter = Utilities::GetGlobalStatusReporter();
#endif // defined(MpM_ReportOnConnections)
        
        if (newChannel)
        {
            if (service.metricsAreEnabled())
            {
                newChannel->enableMetrics();
            }
            else
            {
                newChannel->disableMetrics();
            }
#if defined(MpM_ReportOnConnections)
            newChannel->setReporter(*reporter);
            newChannel->getReport(*reporter);
#endif // defined(MpM_ReportOnConnections)
            if (newChannel->openWithRetries(aName, STANDARD_WAIT_TIME_))
            {
                if (Utilities::NetworkConnectWithRetries(aName, MpM_REGISTRY_ENDPOINT_NAME_,
                                                         STANDARD_WAIT_TIME_, false, checker,
                                                         checkStuff))
                {
                    yarp::os::Bottle parameters(channelName);
                    ServiceRequest   request(MpM_REGISTER_REQUEST_, parameters);
                    ServiceResponse  response;
                    
                    if (request.send(*newChannel, response))
                    {
                        // Check that we got a successful self-registration!
                        if (MpM_EXPECTED_REGISTER_RESPONSE_SIZE_ == response.count())
                        {
                            yarp::os::Value theValue = response.element(0);
                            
                            if (theValue.isString())
                            {
                                result = (theValue.toString() == MpM_OK_RESPONSE_);
                            }
                            else
                            {
                                OD_LOG("! (theValue.isString())"); //####
                            }
                        }
                        else
                        {
                            OD_LOG("! (MpM_EXPECTED_REGISTER_RESPONSE_SIZE_ == " //####
                                   "response.count())"); //####
                            OD_LOG_S1s("response = ", response.asString()); //####
                        }
                    }
                    else
                    {
                        OD_LOG("! (request.send(*newChannel, response))"); //####
                    }
#if defined(MpM_DoExplicitDisconnect)
                    if (! Utilities::NetworkDisconnectWithRetries(aName,
                                                                  MpM_REGISTRY_ENDPOINT_NAME_,
                                                                  STANDARD_WAIT_TIME_, checker,
                                                                  checkStuff))
                    {
                        OD_LOG("(! Utilities::NetworkDisconnectWithRetries(aName, " //####
                               "MpM_REGISTRY_ENDPOINT_NAME_, STANDARD_WAIT_TIME_, checker, " //####
                               "checkStuff))"); //####
                    }
#endif // defined(MpM_DoExplicitDisconnect)
                }
                else
                {
                    OD_LOG("! (Utilities::NetworkConnectWithRetries(aName, " //####
                           "MpM_REGISTRY_ENDPOINT_NAME_, STANDARD_WAIT_TIME_, false, " //####
                           "checker, checkStuff))"); //####
                }
#if defined(MpM_DoExplicitClose)
                newChannel->close();
#endif // defined(MpM_DoExplicitClose)
            }
            else
            {
                OD_LOG("! (newChannel->openWithRetries(aName, STANDARD_WAIT_TIME_))"); //####
            }
            SendReceiveCounters newCounters;
            
            newChannel->getSendReceiveCounters(newCounters);
            service.incrementAuxiliaryCounters(newCounters);
            BaseChannel::RelinquishChannel(newChannel);
        }
        else
        {
            OD_LOG("! (newChannel)"); //####
        }
    }
    catch (...)
    {
        OD_LOG("Exception caught"); //####
        throw;
    }
    OD_LOG_EXIT_B(result); //####
    return result;
} // Common::RegisterLocalService

bool
Common::UnregisterLocalService(const YarpString & channelName,
                               BaseService &      service,
                               CheckFunction      checker,
                               void *             checkStuff)
{
    OD_LOG_ENTER(); //####
    OD_LOG_S1s("channelName = ", channelName); //####
    OD_LOG_P2("service = ", &service, "checkStuff = ", checkStuff); //####
    bool result = false;
    
    try
    {
        YarpString              aName(GetRandomChannelName(HIDDEN_CHANNEL_PREFIX_
                                                           BUILD_NAME_("unregisterlocal_",
                                                                       DEFAULT_CHANNEL_ROOT_)));
        ClientChannel *         newChannel = new ClientChannel;
#if defined(MpM_ReportOnConnections)
        ChannelStatusReporter * reporter = Utilities::GetGlobalStatusReporter();
#endif // defined(MpM_ReportOnConnections)
        
        if (newChannel)
        {
            if (service.metricsAreEnabled())
            {
                newChannel->enableMetrics();
            }
            else
            {
                newChannel->disableMetrics();
            }
#if defined(MpM_ReportOnConnections)
            newChannel->setReporter(*reporter);
            newChannel->getReport(*reporter);
#endif // defined(MpM_ReportOnConnections)
            if (newChannel->openWithRetries(aName, STANDARD_WAIT_TIME_))
            {
                if (Utilities::NetworkConnectWithRetries(aName, MpM_REGISTRY_ENDPOINT_NAME_,
                                                         STANDARD_WAIT_TIME_, false, checker,
                                                         checkStuff))
                {
                    yarp::os::Bottle parameters(channelName);
                    ServiceRequest   request(MpM_UNREGISTER_REQUEST_, parameters);
                    ServiceResponse  response;
                    
                    if (request.send(*newChannel, response))
                    {
                        // Check that we got a successful self-deregistration!
                        if (MpM_EXPECTED_UNREGISTER_RESPONSE_SIZE_ == response.count())
                        {
                            yarp::os::Value theValue = response.element(0);
                            
                            if (theValue.isString())
                            {
                                result = (theValue.toString() == MpM_OK_RESPONSE_);
                            }
                            else
                            {
                                OD_LOG("! (theValue.isString())"); //####
                            }
                        }
                        else
                        {
                            OD_LOG("! (MpM_EXPECTED_UNREGISTER_RESPONSE_SIZE_ == " //####
                                   "response.count())"); //####
                            OD_LOG_S1s("response = ", response.asString()); //####
                        }
                    }
                    else
                    {
                        OD_LOG("! (request.send(*newChannel, response))"); //####
                    }
#if defined(MpM_DoExplicitDisconnect)
                    if (! Utilities::NetworkDisconnectWithRetries(aName,
                                                                  MpM_REGISTRY_ENDPOINT_NAME_,
                                                                  STANDARD_WAIT_TIME_, checker,
                                                                  checkStuff))
                    {
                        OD_LOG("(! Utilities::NetworkDisconnectWithRetries(aName, " //####
                               "MpM_REGISTRY_ENDPOINT_NAME_, STANDARD_WAIT_TIME_, checker, " //####
                               "checkStuff))"); //####
                    }
#endif // defined(MpM_DoExplicitDisconnect)
                }
                else
                {
                    OD_LOG("! (Utilities::NetworkConnectWithRetries(aName, " //####
                           "MpM_REGISTRY_ENDPOINT_NAME_, STANDARD_WAIT_TIME_, false, " //####
                           "checker, checkStuff))"); //####
                }
#if defined(MpM_DoExplicitClose)
                newChannel->close();
#endif // defined(MpM_DoExplicitClose)
            }
            else
            {
                OD_LOG("! (newChannel->openWithRetries(aName, STANDARD_WAIT_TIME_))"); //####
            }
            SendReceiveCounters newCounters;
            
            newChannel->getSendReceiveCounters(newCounters);
            service.incrementAuxiliaryCounters(newCounters);
            BaseChannel::RelinquishChannel(newChannel);
        }
        else
        {
            OD_LOG("! (newChannel)"); //####
        }
    }
    catch (...)
    {
        OD_LOG("Exception caught"); //####
        throw;
    }
    OD_LOG_EXIT_B(result); //####
    return result;
} // Common::UnregisterLocalService
