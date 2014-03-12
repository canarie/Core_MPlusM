//
//  YPPMatchConstraint.cpp
//  YarpPlusPlus
//
//  Created by Norman Jaffe on 2014-03-10.
//  Copyright (c) 2014 OpenDragon. All rights reserved.
//

#include "YPPMatchConstraint.h"
//#define ENABLE_OD_SYSLOG /* */
#include "ODSyslog.h"
#include "YPPMatchExpression.h"
#include "YPPMatchFieldWithValues.h"

using namespace YarpPlusPlusParser;

#if defined(__APPLE__)
# pragma mark Private structures, constants and variables
#endif // defined(__APPLE__)

/*! @brief The character used to separate constraint list elements. */
static const char kAmpersand = '&';

#if defined(__APPLE__)
# pragma mark Local functions
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Class methods
#endif // defined(__APPLE__)

char MatchConstraint::ConstraintSeparatorCharacter(void)
{
    return kAmpersand;
} // MatchConstraint::ConstraintSeparatorCharacter

MatchConstraint * MatchConstraint::CreateMatcher(const yarp::os::ConstString & inString,
                                                 const int                     inLength,
                                                 const int                     startPos,
                                                 int &                         endPos,
                                                 FieldNameValidator            validator)
{
    OD_SYSLOG_ENTER();//####
    OD_SYSLOG_S1("inString = ", inString.c_str());//####
    OD_SYSLOG_LL2("inLength = ", inLength, "startPos = ", startPos);
    int               workPos = SkipWhitespace(inString, inLength, startPos);
    MatchConstraint * result = NULL;
    
    if (workPos < inLength)
    {
        // We potentially have a constraint list.
        bool       done = false;
        bool       okSoFar = true;
        const char expressionSeparator = MatchExpression::ExpressionSeparatorCharacter();
        
        result = new MatchConstraint();
        for ( ; okSoFar && (! done); )
        {
            int                    nextElementPos;
            MatchFieldWithValues * element = MatchFieldWithValues::CreateMatcher(inString, inLength, workPos,
                                                                                 nextElementPos, validator);
            
            if (element)
            {
                // Skip over any trailing whitespace, to find if the constraint list is complete or more coming.
                workPos = SkipWhitespace(inString, inLength, nextElementPos);
                if (workPos < inLength)
                {
                    char scanChar = inString[workPos];

                    if (kAmpersand == scanChar)
                    {
                        // We've got more elements to go.
                        result->_fieldsWithValues.push_back(element);
                        ++workPos;
                    }
                    else if (expressionSeparator == scanChar)
                    {
                        // This is the last element.
                        result->_fieldsWithValues.push_back(element);
                        done = true;
                    }
                    else
                    {
                        // Something unexpected has appeared.
                        okSoFar = false;
                    }
                }
                else
                {
                    // This is the last element.
                    result->_fieldsWithValues.push_back(element);
                    done = true;
                }
            }
            else
            {
                // We have a malformed constraint list.
                okSoFar = false;
            }
        }
        if (okSoFar)
        {
            endPos = workPos;
        }
        else
        {
            delete result;
            result = NULL;
        }
    }
    OD_SYSLOG_EXIT_P(result);//####
    return result;
} // MatchConstraint::CreateMatcher

#if defined(__APPLE__)
# pragma mark Constructors and destructors
#endif // defined(__APPLE__)

MatchConstraint::MatchConstraint(void) :
        inherited(), _fieldsWithValues()
{
    OD_SYSLOG_ENTER();//####
    OD_SYSLOG_EXIT_P(this);//####
} // MatchConstraint::MatchConstraint

MatchConstraint::~MatchConstraint(void)
{
    OD_SYSLOG_ENTER();//####
    empty();
    OD_SYSLOG_EXIT();//####
} // MatchConstraint::~MatchConstraint

#if defined(__APPLE__)
# pragma mark Actions
#endif // defined(__APPLE__)

yarp::os::ConstString MatchConstraint::asSQLString(void)
const
{
    OD_SYSLOG_ENTER();//####
    yarp::os::ConstString result;
    
    for (MatchConstraintListSize ii = 0, maxI = _fieldsWithValues.size(); ii < maxI; ++ii)
    {
        MatchFieldWithValues * element = _fieldsWithValues[ii];
        
        if (ii)
        {
            result += " AND ";
        }
        result += element->asSQLString();
    }
    OD_SYSLOG_EXIT_S(result.c_str());//####
    return result;
} // MatchConstraint::asSQLString

yarp::os::ConstString MatchConstraint::asString(void)
const
{
    yarp::os::ConstString result;
    
    for (MatchConstraintListSize ii = 0, maxI = _fieldsWithValues.size(); ii < maxI; ++ii)
    {
        MatchFieldWithValues * element = _fieldsWithValues[ii];
        
        if (ii)
        {
            result += " ";
            result += kAmpersand;
            result += " ";
        }
        result += element->asString();
    }
    return result;
} // MatchConstraint::asString

int MatchConstraint::count(void)
const
{
    return static_cast<int>(_fieldsWithValues.size());
} // MatchConstraint::count

const MatchFieldWithValues * MatchConstraint::element(const int index)
const
{
    MatchFieldWithValues * result;
    
    if ((index >= 0) && (index < static_cast<int>(_fieldsWithValues.size())))
    {
        result = _fieldsWithValues[static_cast<MatchConstraintListSize>(index)];
    }
    else
    {
        result = NULL;
    }
    return result;
} // MatchConstraint::element

void MatchConstraint::empty(void)
{
    OD_SYSLOG_ENTER();//####
    for (MatchConstraintListSize ii = 0, maxI = _fieldsWithValues.size(); ii < maxI; ++ii)
    {
        MatchFieldWithValues * element = _fieldsWithValues[ii];
        
        delete element;
    }
    _fieldsWithValues.clear();
    OD_SYSLOG_EXIT();//####
} // MatchConstraint::empty

#if defined(__APPLE__)
# pragma mark Accessors
#endif // defined(__APPLE__)

#if defined(__APPLE__)
# pragma mark Global functions
#endif // defined(__APPLE__)
