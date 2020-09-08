## from libmodbus manpage

The Modbus protocol contains many variants (eg. serial RTU or Ethernet TCP), to ease the implementation of a variant, the
       library was designed to use a backend for each variant. The backends are also a convenient way to fulfill other
       requirements (eg. real-time operations). Each backend offers a specific function to create a new modbus_t context. The
       modbus_t context is an opaque structure containing all necessary information to establish a connection with other Modbus
       devices according to the selected variant.


++++++++++++++++++++  end README.md
