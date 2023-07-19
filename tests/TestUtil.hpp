
#include <iostream>
#include <fstream>
#include <thread>
#include <exception>
#include <cassert>
#include <stdexcept>
#include <functional>

#ifndef _TEST_UTIL__
#define _TEST_UTIL__

/**
 * A helper function used to ensure that an exception is thrown by the passed in function and
 * also ensures the exception's type is what is specified in the template argument.
 * 
 * @param function the function to test
 * 
 * @return true if an exception was thrown and was of type T, otherwise false.
 */
template <typename T>
bool throwsException(const std::function<void()>& function, bool logException = false)
{
    try
    {
        function();
    }
    catch (T &ex)
    {
        if (logException)
        {
            std::cout << ex.what() << std::endl;
        }
        return true;
    }
    catch (...)
    {
        // Do nothing, will return false
    }
    return false;
}

/**
 * Print the function name to indicate which test function is currently running.
 * 
 * @param functionName the function name to be printed
 */
void preFunctionTest(const std::string& functionName)
{
    std::cout << "Running... " << functionName << "()... " << std::flush;
}

/**
 * Will run a specific function and print "PASS" once it is finished.
 * 
 * @param function the function to call
 */
void testFunction(const std::function<void()>& function)
{
    function();
    std::cout << "PASS" << std::endl;
}

#endif // _TEST_UTIL__
