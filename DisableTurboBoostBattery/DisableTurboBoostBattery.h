/*
 *
 * Copyright (C) 2013 Faheem Pervez. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

//Based heavily on code by Pavel Prokofiev's NoSleep. Copyright 2012. All rights reserved.

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

	virtual IOReturn setPowerState(unsigned long whichState, __unused IOService * whatDevice);
};

#endif //__DisableTurboBoostBattery__