/* Copyright (c) 2011, NEC Europe Ltd, Consorzio Nazionale
 * Interuniversitario per le Telecomunicazioni, Institut
 * Telecom/Telecom Bretagne, ETH Zürich, INVEA-TECH a.s. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the names of NEC Europe Ltd, Consorzio Nazionale
 *      Interuniversitario per le Telecomunicazioni, Institut Telecom/Telecom
 *      Bretagne, ETH Zürich, INVEA-TECH a.s. nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT
 * HOLDERBE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 */

#ifndef _CORE_MPMCQUEUE_H_
#define _CORE_MPMCQUEUE_H_

#include <list>
#include <iostream>
#include <memory>
#include <thread>
#include <algorithm>

namespace blockmon {


 /** Simple multi-consumer multi-producer template queue.
  *
  * It is basically made up of a list plus a mutex The queue accepts
  * and returns shared_pointers and retains shared ownership of the
  * enqueued objects.
  */
    template<typename T>
    class MpMcQueue
    {
        std::list<std::shared_ptr<T>> m_queue;
        std::mutex m_mutex;

        /**
          * helper callable object for removing an object bases in its address
          */
        struct remover
        {
            const T& cmp;
            remover( const T & r): cmp(r)
            {}
            bool operator ()(const std::shared_ptr<T>& in)
            {
                return (&cmp==in.get());
            }
        };

    public:

        MpMcQueue()
        : m_queue(),
          m_mutex()
        {}

        ~MpMcQueue()
        {}
        /**
          * non-copiable
          */
        MpMcQueue(const MpMcQueue&) = delete;
        MpMcQueue& operator=(const MpMcQueue&) = delete;

        void swap(MpMcQueue &other)
        {
            std::swap(m_queue, other.m_queue);
            std::swap(m_mutex, other.m_mutex);
        }

        MpMcQueue(MpMcQueue&& other)
        : m_queue(std::move(other)),
          m_mutex(std::move(m_mutex))
        {}

        MpMcQueue& operator=(MpMcQueue&& other)
        {
            MpMcQueue tmp(std::move(other));
            tmp.swap(*this);
            return *this;
        }

        /**
          * returns the object at the head of the queue
          * @return if a queue is not empty a shared pointer to the head object, otherwise a default-constructed shared pointer
          */

        std::shared_ptr<T> pop()
        {
            std::lock_guard<std::mutex> _lock_(m_mutex);
            if(m_queue.empty())
            {
                return std::shared_ptr<T>();
            }
            else
            {
                std::shared_ptr<T> tmpm(std::move(m_queue.front()));
                m_queue.pop_front();

                return tmpm;
            }
        }


        /**
          * removes all of the pointers to a given object
          * @param in const reference to the object to be removed (for computing the address only)
          */
        void remove(const T& in)
        {
            std::lock_guard<std::mutex> _lock_(m_mutex);
            m_queue.erase(std::remove_if(m_queue.begin(),m_queue.end(),remover(in)), m_queue.end());
        }
        /**
          * pushes a new object to the back of the queue
          * @param in a shared pointer to the object (as r-value reference)
         */
        void push(std::shared_ptr<T>&& in)
        {
            std::lock_guard<std::mutex> _lock_(m_mutex);
            m_queue.push_back(std::move(in));
        }

    };


} // namespace blockmon


#endif /* _CORE_MPMCQUEUE_H_ */
