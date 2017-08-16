#include "producers_autoreg.h"

#include "producer_factory.h"

#include "DummyDevice.h"
#include "MockProducer.h"
#include "ESSStream.h"

void producers_autoreg()
{
  DAQUIRI_REGISTER_PRODUCER(DummyDevice)
  DAQUIRI_REGISTER_PRODUCER(MockProducer)
  DAQUIRI_REGISTER_PRODUCER(ESSStream)
}
