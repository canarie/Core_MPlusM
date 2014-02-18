//
//  YPPInputHandlerCreator.h
//  YarpPlusPlus
//
//  Created by Norman Jaffe on 2014-02-012.
//  Copyright (c) 2014 OpenDragon. All rights reserved.
//

#if (! defined(YPPINPUTHANDLERCREATOR_H_))
# define YPPINPUTHANDLERCREATOR_H_ /* */

# include "YPPInputHandler.h"
# include <yarp/os/PortReaderCreator.h>

namespace YarpPlusPlus
{
    /*! @brief A factory for InputHandler objects. */
    class InputHandlerCreator : public yarp::os::PortReaderCreator
    {
    public:

        /*! @brief The constructor. */
        InputHandlerCreator(void);
        
        /*! @brief The destructor. */
        virtual ~InputHandlerCreator(void);
        
        /*! @brief Create a new InputHandler object to process input data.
         @returns A new InputHandler or @c NULL if one cannot be created. */
        virtual InputHandler * create(void) = 0;
        
    protected:

    private:
        
    }; // InputHandlerCreator
    
} // YarpPlusPlus

#endif // ! defined(YPPINPUTHANDLERCREATOR_H_)
