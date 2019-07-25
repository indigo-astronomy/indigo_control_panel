// Copyright (c) 2019 Rumen G.Bogdanovski & David Hulse
// All rights reserved.
//
// You can use this software under the terms of 'INDIGO Astronomy
// open-source license' (see LICENSE.md).
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS 'AS IS' AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef INDIGOCLIENT_H
#define INDIGOCLIENT_H

#include <QObject>
#include <indigo/indigo_bus.h>
#include "logger.h"


class IndigoClient : public QObject
{
    Q_OBJECT
public:
    static IndigoClient& instance();

public:
    IndigoClient();

    void start();

	Logger* m_logger;
signals:
    void property_defined(indigo_property* property, const char *message);
    void property_changed(indigo_property* property, const char *message);
    void property_deleted(indigo_property* property, const char *message);
};

inline IndigoClient&
IndigoClient::instance()
{
   static IndigoClient* me = nullptr;
   if (!me)
       me = new IndigoClient();
   return *me;
}

#endif // INDIGOCLIENT_H
