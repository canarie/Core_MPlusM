//
//  YPPExampleEchoService.cpp
//  YarpPlusPlus
//
//  Created by Norman Jaffe on 2014-02-06.
//  Copyright (c) 2014 OpenDragon. All rights reserved.
//

#if (! defined(YPPEXAMPLEECHOSERVICE_H_))
# define YPPEXAMPLEECHOSERVICE_H_ /* */

# include "YPPBaseService.h"

/*! @brief The port name to use for the service if not provided. */
# define DEFAULT_ECHO_SERVICE_NAME "/service/echo"

namespace YarpPlusPlusExample
{
    /*! @brief An example Yarp++ service, handling 'echo' requests. */
    class ExampleEchoService : public YarpPlusPlus::BaseService
    {
    public:
        
        /*! @brief The constructor.
         @param serviceEndpointName The YARP name to be assigned to the new service.
         @param serviceHostName The name or IP address of the machine running the service.
         @param servicePortNumber The port being used by the service. */
        ExampleEchoService(const yarp::os::ConstString & serviceEndpointName,
                           const yarp::os::ConstString & serviceHostName = "",
                           const yarp::os::ConstString & servicePortNumber = "");
        
        /*! @brief The destructor. */
        virtual ~ExampleEchoService(void);
        
        /*! @brief Start processing requests.
         @returns @c true if the service was started and @c false if it was not. */
        virtual bool start(void);
        
        /*! @brief Stop processing requests.
         @returns @c true if the service was stopped and @c false it if was not. */
        virtual bool stop(void);

    protected:
        
    private:
        
        /*! @brief The class that this class is derived from. */
        typedef BaseService inherited;

        /*! @brief Copy constructor.
         
         Note - not implemented and private, to prevent unexpected copying.
         @param other Another object to construct from. */
        ExampleEchoService(const ExampleEchoService & other);
        
        /*! @brief Assignment operator.
         
         Note - not implemented and private, to prevent unexpected copying.
         @param other Another object to construct from. */
        ExampleEchoService & operator=(const ExampleEchoService & other);
        
        /*! @brief Set up the standard request handlers. */
        void setUpRequestHandlers(void);
        
    }; // ExampleEchoService
    
} // YarpPlusPlusExample

#endif // ! defined(YPPEXAMPLEECHOSERVICE_H_)
