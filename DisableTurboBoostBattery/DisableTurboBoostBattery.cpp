//Copyright (C) 2013 qwerty12
//License: http://www.gnu.org/licenses/gpl.html GPL version 2
//Note, the GPLv2.html file inside NoSleep's installer suggests it is, in fact, a GPLv2'd project. However, the Google Code project page says it is under the New BSD License.
//Sticking with the GPLv2 license for now, but will change if it is actually under the BSD license.

//Butchered work pretty much entirely taken from/based on code from:
//
//  NoSleepExtensionPower.cpp
//  NoSleepKext
//
//  Created by Pavel Prokofiev on 2/17/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#include "DisableTurboBoostBattery.h"
#include "DisableTurboBoost.h"

#define super IOService

#include <IOKit/IOLib.h>

OSDefineMetaClassAndStructors(DisableTurboBoostBattery, IOService)

void DisableTurboBoostBattery::startPM(IOService *provider)
{
	static const int kMyNumberOfStates = 2;
	static IOPMPowerState myPowerStates[kMyNumberOfStates] = {
		{kIOPMPowerStateVersion1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{kIOPMPowerStateVersion1, kIOPMPowerOn, kIOPMPowerOn, kIOPMPowerOn, 0, 0, 0, 0, 0, 0, 0, 0}
	};

	PMinit();
	provider->joinPMtree(this);
	registerPowerDriver(this, myPowerStates, kMyNumberOfStates);

	if (OSDictionary *tmpDict = serviceMatching("IOPMPowerSource"))
	{
		addMatchingNotification(gIOFirstPublishNotification, tmpDict,
								&DisableTurboBoostBattery::_powerSourcePublished,
								this, this);

		tmpDict->release();
	}
}

void DisableTurboBoostBattery::stopPM()
{
	if(powerStateNotifier){
		powerStateNotifier->remove();
		powerStateNotifier = NULL;
	}

	PMstop();
}

bool DisableTurboBoostBattery::_powerSourcePublished(void * target, __unused void * refCon,
													 IOService * newService, IONotifier * notifier)
{
	return ((DisableTurboBoostBattery *)target)->powerSourcePublished(newService, notifier);
}

bool DisableTurboBoostBattery::powerSourcePublished(IOService *newService, IONotifier *notifier)
{
	pPowerSource = (IOPMPowerSource *)newService;

	this->powerStateNotifier = pPowerSource->registerInterest(gIOGeneralInterest, DisableTurboBoostBattery::_powerSourceStateChanged, this);
	notifier->remove();
	actOnChangedPowerState();

	return true;
}

IOReturn DisableTurboBoostBattery::setPowerState (unsigned long whichState, __unused IOService * whatDevice)
{
	if (whichState == 0) {
		enable_tb();
        isOnAC = !isOnAC;
	}

	return kIOPMAckImplied;
}

IOReturn DisableTurboBoostBattery::_powerSourceStateChanged(void * target, __unused void * refCon,
															UInt32 messageType, IOService * provider,
															void * messageArgument, vm_size_t argSize)
{
	return ((DisableTurboBoostBattery *)target)->powerSourceStateChanged(messageType, provider,
																		 messageArgument, argSize);
}

IOReturn DisableTurboBoostBattery::powerSourceStateChanged(UInt32 messageType, __unused IOService * provider,
														   __unused void * messageArgument, __unused vm_size_t argSize)
{
	if (messageType == kIOPMMessageBatteryStatusHasChanged)
		actOnChangedPowerState();

	return kIOReturnSuccess;
}

void DisableTurboBoostBattery::actOnChangedPowerState()
{
	if(pPowerSource && isOnAC != pPowerSource->externalChargeCapable()) {
		isOnAC = pPowerSource->externalChargeCapable();
		startOrStop();
	}
}

void DisableTurboBoostBattery::startOrStop()
{
	if (isOnAC)
		enable_tb();
	else
		disable_tb();
}

bool DisableTurboBoostBattery::start(IOService *provider)
{
	if (!super::start(provider))
		return false;

	isOnAC = true;
	enable_tb();

	startPM(provider);

	return true;
}

void DisableTurboBoostBattery::stop(IOService *provider)
{
	stopPM();

	enable_tb();

	super::stop(provider);
}
