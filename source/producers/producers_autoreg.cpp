#include <producers/producers_autoreg.h>
#include <core/producer_factory.h>

#include <producers/DetectorIndex/DetectorIndex.h>
#include <producers/DummyDevice/DummyDevice.h>
#include <producers/MockProducer/MockProducer.h>
#include <producers/ESSStream/ESSStream.h>

void producers_autoreg()
{
    DAQUIRI_REGISTER_PRODUCER(DetectorIndex)
    DAQUIRI_REGISTER_PRODUCER(DummyDevice)
    DAQUIRI_REGISTER_PRODUCER(ESSStream)
    DAQUIRI_REGISTER_PRODUCER(MockProducer)
}
