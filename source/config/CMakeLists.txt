set(DAQuiri_gui TRUE PARENT_SCOPE)
set(DAQuiri_cmd TRUE PARENT_SCOPE)
list(APPEND DAQuiri_enabled_producers DummyDevice)
list(APPEND DAQuiri_enabled_producers MockProducer)
list(APPEND DAQuiri_enabled_producers ESSStream)
set(DAQuiri_enabled_producers ${DAQuiri_enabled_producers} PARENT_SCOPE)

