#ifndef EMULATE_SERVICE_HPP
#define EMULATE_SERVICE_HPP


enum
{
  PROPERTY_READONLY  = 0,
  PROPERTY_READWRITE = 1
};

int emulate_service(const char *filename, int serviceMethod);


#endif // EMULATE_SERVICE_HPP
