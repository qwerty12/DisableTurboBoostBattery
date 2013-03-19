//Copyright (C) 2013 qwerty12
//License: http://www.gnu.org/licenses/gpl.html GPL version 2

//Butchered work pretty much entirely taken from/based on code from:
//
//  NoSleepExtensionPower.cpp
//  NoSleepKext
//
//  Created by Pavel Prokofiev on 2/17/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#ifndef __DisableTurboBoostBattery__
#define __DisableTurboBoostBattery__

#include <IOKit/IOService.h>
#include <IOKit/pwr_mgt/IOPMPowerSource.h>

class DisableTurboBoostBattery : public IOService
{
	OSDeclareDefaultStructors(DisableTurboBoostBattery);

public:
	bool start(IOService *provider);
	void stop(IOService *provider);

protected:
	virtual bool powerSourcePublished(IOService * newService, IONotifier * notifier);
	virtual IOReturn powerSourceStateChanged(UInt32 messageType, IOService * provider,
                                             void * messageArgument, vm_size_t argSize);

private:
	IONotifier *powerStateNotifier;
	IOPMPowerSource *pPowerSource;

	bool isOnAC;

	void startPM(IOService *provider);
	void stopPM();
	void actOnChangedPowerState();
	void startOrStop();

    static bool _powerSourcePublished(void * target, void * refCon,
                                      IOService * newService, IONotifier * notifier);

    static IOReturn _powerSourceStateChanged(void * target, void * refCon,
                                             UInt32 messageType, IOService * provider,
                                             void * messageArgument, vm_size_t argSize);

	virtual IOReturn setPowerState (unsigned long whichState, __unused IOService * whatDevice);

};

#endif //__DisableTurboBoostBattery__