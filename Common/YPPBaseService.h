//--------------------------------------------------------------------------------------
//
//  File:       YPPBaseService.h
//
//  Project:    YarpPlusPlus
//
//  Contains:   The class declaration for the minimal functionality required for a Yarp++
//              service.
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
//  Created:    2014-02-06
//
//--------------------------------------------------------------------------------------

#if (! defined(YPPBASESERVICE_H_))
# define YPPBASESERVICE_H_ /* */

# include "YPPRequestMap.h"

namespace YarpPlusPlus
{
    class BaseContext;
    class BaseRequestHandler;
    class BaseServiceInputHandler;
    class BaseServiceInputHandlerCreator;
    class Endpoint;
    
    /*! @brief The minimal functionality required for a Yarp++ service. */
    class BaseService
    {
    public:
        
        /*! @brief The constructor.
         @param useMultipleHandlers @c true if simultaneous handlers are allowed, @c false if one handler is used.
         @param canonicalName The port-independent name of the service.
         @param description The description of the service.
         @param serviceEndpointName The YARP name to be assigned to the new service.
         @param serviceHostName The name or IP address of the machine running the service.
         @param servicePortNumber The port being used by the service. */
        BaseService(const bool                    useMultipleHandlers,
                    const yarp::os::ConstString & canonicalName,
                    const yarp::os::ConstString & description,
                    const yarp::os::ConstString & serviceEndpointName,
                    const yarp::os::ConstString & serviceHostName = "",
                    const yarp::os::ConstString & servicePortNumber = "");
        
        /*! @brief The constructor.
         @param useMultipleHandlers @c true if simultaneous handlers are allowed, @c false if one handler is used.
         @param canonicalName The port-independent name of the service.
         @param description The description of the service.
         @param argc The number of arguments in 'argv'.
         @param argv The arguments to be used to specify the new service. */
        BaseService(const bool                    useMultipleHandlers,
                    const yarp::os::ConstString & canonicalName,
                    const yarp::os::ConstString & description,
                    const int                     argc,
                    char **                       argv);
        
        /*! @brief The destructor. */
        virtual ~BaseService(void);
        
        /*! @brief Return the standard name of the service.
         @returns The standard name of the service. */
        inline const yarp::os::ConstString & canonicalName(void)
        const
        {
            return _canonicalName;
        } // canonicalName
        
        /*! @brief Return the description of the service.
         @returns The standard name of the service. */
        inline const yarp::os::ConstString & description(void)
        const
        {
            return _description;
        } // description
        
        /*! @brief Return the associated endpoint.
         @returns The associated endpoint. */
        inline Endpoint & getEndpoint(void)
        const
        {
            return *_endpoint;
        } // getEndpoint
        
        /*! @brief Return the state of the service.
         @returns @c true if the service has been started and @c false otherwise. */
        inline bool isStarted(void)
        const
        {
            return _started;
        } // isStarted
        
        /*! @brief Process partially-structured input data.
         @param request The requested operation.
         @param restOfInput The arguments for the operation.
         @param senderPort The name of the port used to send the input data.
         @param replyMechanism @c NULL if no reply is expected and non-@c NULL otherwise.
         @returns @c true if the input was correctly structured and successfully processed. */
        bool processRequest(const yarp::os::ConstString & request,
                            const yarp::os::Bottle &      restOfInput,
                            const yarp::os::ConstString & senderPort,
                            yarp::os::ConnectionWriter *  replyMechanism);
        
        /*! @brief Start processing requests.
         @returns @c true if the service was started and @c false if it was not. */
        virtual bool start(void);
        
        /*! @brief Stop processing requests.
         @returns @c true if the service was stopped and @c false it if was not. */
        virtual bool stop(void);
        
    protected:
        
        /*! @brief Add a context for a persistent connection.
         @param key The name for the context.
         @param context The context to be remembered. */
        void addContext(const yarp::os::ConstString & key,
                        BaseContext *                 context);
        
        /*! @brief Remove all contexts. */
        void clearContexts(void);
        
        /*! @brief Locate the context corresponding to a name.
         @param key The name of the context.
         @returns @c NULL if the named context could not be found or a pointer to the context if found. */
        BaseContext * findContext(const yarp::os::ConstString & key);
        
        /*! @brief Remove a context.
         @param key The name of the context. */
        void removeContext(const yarp::os::ConstString & key);
        
        /*! @brief The map between requests and request handlers. */
        RequestMap _requestHandlers;
        
    private:
        
        /*! @brief A mapping from strings to contexts. */
        typedef std::map<std::string, BaseContext *> ContextMap;
        
        /*! @brief The entry-type for the mapping. */
        typedef ContextMap::value_type               ContextMapValue;
        
        /*! @brief Copy constructor.

         Note - not implemented and private, to prevent unexpected copying.
         @param other Another object to construct from. */
        BaseService(const BaseService & other);
        
        /*! @brief Assignment operator.

         Note - not implemented and private, to prevent unexpected copying.
         @param other Another object to construct from. */
        BaseService & operator=(const BaseService & other);
        
        /*! @brief Set up the standard request handlers. */
        void setUpRequestHandlers(void);
        
        /*! @brief The map between requests and request handlers. */
        ContextMap                       _contexts;
        
        /*! @brief The port-independent name of the service. */
        yarp::os::ConstString            _canonicalName;
        
        /*! @brief The description of the service. */
        yarp::os::ConstString            _description;
        
        /*! @brief The connection point for the service. */
        Endpoint *                       _endpoint;
        
        /*! @brief The input handler for the service. */
        BaseServiceInputHandler *        _handler;

        /*! @brief The input handler creator for the service. */
        BaseServiceInputHandlerCreator * _handlerCreator;
        
        /*! @brief The current state of the service - @c true if active and @c false otherwise. */
        bool                             _started;
        
        /*! @brief Whether to use a handler creator or a handler - @c true for a creator and @c false otherwise. */
        bool                             _useMultipleHandlers;
        
# if defined(__APPLE__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunused-private-field"
# endif // defined(__APPLE__)
        /*! @brief Filler to pad to alignment boundary */
        char                             _filler[6];
# if defined(__APPLE__)
#  pragma clang diagnostic pop
# endif // defined(__APPLE__)
        
    }; // BaseService
    
    /*! @brief Register a local service with a running Service Registry service.
     @param portName The port provided by the service.
     @returns @c true if the service was successfully registered and @c false otherwise. */
    bool RegisterLocalService(const yarp::os::ConstString & portName);
    
    /*! @brief Unregister a local service with a running Service Registry service.
     @param portName The port provided by the service.
     @returns @c true if the service was successfully unregistered and @c false otherwise. */
    bool UnregisterLocalService(const yarp::os::ConstString & portName);
    
} // YarpPlusPlus

#endif // ! defined(YPPBASESERVICE_H_)
