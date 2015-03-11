/*--------------------------------------------------------------- 
 * Copyright (c) 1999,2000,2001,2002,2003                              
 * The Board of Trustees of the University of Illinois            
 * All Rights Reserved.                                           
 *--------------------------------------------------------------- 
 * Permission is hereby granted, free of charge, to any person    
 * obtaining a copy of this software (Iperf) and associated       
 * documentation files (the "Software"), to deal in the Software  
 * without restriction, including without limitation the          
 * rights to use, copy, modify, merge, publish, distribute,        
 * sublicense, and/or sell copies of the Software, and to permit     
 * persons to whom the Software is furnished to do
 * so, subject to the following conditions: 
 *
 *     
 * Redistributions of source code must retain the above 
 * copyright notice, this list of conditions and 
 * the following disclaimers. 
 *
 *     
 * Redistributions in binary form must reproduce the above 
 * copyright notice, this list of conditions and the following 
 * disclaimers in the documentation and/or other materials 
 * provided with the distribution. 
 * 
 *     
 * Neither the names of the University of Illinois, NCSA, 
 * nor the names of its contributors may be used to endorse 
 * or promote products derived from this Software without
 * specific prior written permission. 
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE CONTIBUTORS OR COPYRIGHT 
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 * ________________________________________________________________
 * National Laboratory for Applied Network Research 
 * National Center for Supercomputing Applications 
 * University of Illinois at Urbana-Champaign 
 * http://www.ncsa.uiuc.edu
 * ________________________________________________________________ 
 *
 * Thread.h
 * by Kevin Gibbs <kgibbs@nlanr.net>
 *
 * Based on:
 * Thread.hpp
 * by Mark Gates <mgates@nlanr.net>
 * -------------------------------------------------------------------
 * The thread subsystem is responsible for all thread functions. It
 * provides a thread implementation agnostic interface to Iperf. If 
 * threads are not available (HAVE_THREAD is undefined).
 * ------------------------------------------------------------------- */

#ifndef THREAD_H
#define THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "headers.h"

#if   defined( HAVE_POSIX_THREAD )

/* Definitions for Posix Threads (pthreads) */
    #include <pthread.h>

typedef pthread_t nthread_t;

    #define HAVE_THREAD 1

#elif defined( HAVE_WIN32_THREAD )

/* Definitions for Win32 NT Threads */
typedef DWORD nthread_t;

    #define HAVE_THREAD 1

#else

/* Definitions for no threads */
typedef int nthread_t;

    #undef HAVE_THREAD

#endif

#include "Condition.h"

  /* -------------------------------------------------------------------
     * Return the current thread's ID.
     * ------------------------------------------------------------------- */
    #if   defined( HAVE_POSIX_THREAD )
        #define thread_getid() pthread_self()
    #elif defined( HAVE_WIN32_THREAD )
        #define thread_getid() GetCurrentThreadId()
    #else
        #define thread_getid() 0
    #endif

    int thread_equalid( nthread_t inLeft, nthread_t inRight );

    nthread_t thread_zeroid( void );
    

#ifdef __cplusplus
} /* end extern "C" */
#endif
#endif // THREAD_H
