//--------------------------------------------------------------------------------------------------
//
//  File:       m+mFormFieldErrorResponder.hpp
//
//  Project:    m+m
//
//  Contains:   The class declaration for an error reporting abstraction.
//
//  Written by: Norman Jaffe
//
//  Copyright:  (c) 2016 by H Plus Technologies Ltd. and Simon Fraser University.
//
//              All rights reserved. Redistribution and use in source and binary forms, with or
//              without modification, are permitted provided that the following conditions are met:
//                * Redistributions of source code must retain the above copyright notice, this list
//                  of conditions and the following disclaimer.
//                * Redistributions in binary form must reproduce the above copyright notice, this
//                  list of conditions and the following disclaimer in the documentation and/or
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
//  Created:    2016-05-12
//
//--------------------------------------------------------------------------------------------------

#if (! defined(mpmFormFieldErrorResponder_HPP_))
# define mpmFormFieldErrorResponder_HPP_ /* Header guard */

# include "m+mCommonVisuals.hpp"

# if defined(__APPLE__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunknown-pragmas"
#  pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
# endif // defined(__APPLE__)
/*! @file

 @brief The class declaration for an error reporting abstraction. */
# if defined(__APPLE__)
#  pragma clang diagnostic pop
# endif // defined(__APPLE__)

namespace CommonVisuals
{
    class FormField;

    /*! @brief An error reporting abstraction. */
    class FormFieldErrorResponder
    {
    public :

    protected :

    private :

    public :

        /*! @brief The constructor. */
        FormFieldErrorResponder(void);

        /*! @brief The destructor. */
        virtual
        ~FormFieldErrorResponder(void);

        /*! @brief Report an error in a field.
         @param[in] fieldOfInterest The field to be reported. */
        virtual void
        reportErrorInField(FormField & fieldOfInterest) = 0;

    protected :

    private :

    public :

    protected :

    private :

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FormFieldErrorResponder)

    }; // FormFieldErrorResponder

} // CommonVisuals

#endif // ! defined(mpmFormFieldErrorResponder_HPP_)
