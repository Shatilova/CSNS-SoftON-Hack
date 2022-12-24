#pragma once

// All interfaces derive from this
class IBaseInterface {
public:
	virtual ~IBaseInterface() {}
};

#define CREATEINTERFACE_PROCNAME	"CreateInterface"

typedef IBaseInterface* (*CreateInterfaceFn)(const char *pName, int *pReturnCode);

CreateInterfaceFn CaptureFactory(char* FactoryModule);
PVOID CaptureInterface(CreateInterfaceFn Interface, char* InterfaceName);