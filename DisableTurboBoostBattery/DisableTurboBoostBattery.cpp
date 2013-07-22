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

#include "DisableTurboBoostBattery.h"
#include "DisableTurboBoost.h"

#define super IOService

OSDefineMetaClassAndStructors(DisableTurboBoostBattery, IOService)

void DisableTurboBoostBattery::startPM(IOService *provider)
{
	const int kMyNumberOfStates = 2;
	static IOPMPowerState myPowerStates[kMyNumberOfStates] = {
		{kIOPMPowerStateVersion1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{kIOPMPowerStateVersion1, kIOPMPowerOn, kIOPMPowerOn, kIOPMPowerOn, 0, 0, 0, 0, 0, 0, 0, 0}
	};

	PMinit();
	provider->joinPMtree(this);
	registerPowerDriver(this, myPowerStates, kMyNumberOfStates);

	if (OSDictionary *tmpDict = serviceMatching("IOPMPowerSource")) {
		addMatchingNotification(gIOFirstPublishNotification, tmpDict,
								&DisableTurboBoostBattery::_powerSourcePublished,
								this, this);

		tmpDict->release();
	}
}

void DisableTurboBoostBattery::stopPM()
{
	if (powerStateNotifier) {
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

	this->powerStateNotifier = pPowerSource->registerInterest(gIOGeneralInterest, DisableTurboBoostBattery::_powerSourceStateChanged,
								  this);
	notifier->remove();
	actOnChangedPowerState();

	return true;
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
	if (pPowerSource && isOnAC != pPowerSource->externalChargeCapable() && pPowerSource->batteryInstalled()) {
		if ((isOnAC = pPowerSource->externalChargeCapable()))
			enable_tb();
		else
			disable_tb();
	}
}

IOReturn DisableTurboBoostBattery::setPowerState(unsigned long whichState, __unused IOService * whatDevice)
{
	// Workaround bug with ASUS laptops that enable Turbo Boost again when resuming from sleep by enabling it ourselves before going to sleep and getting the kext to re-apply the correct state
	if (whichState == 0) {
		enable_tb();
		isOnAC = !isOnAC;
	}

	return kIOPMAckImplied;
}

bool DisableTurboBoostBattery::start(IOService *provider)
{
	if (!super::start(provider))
		return false;

	isOnAC = true;
	startPM(provider);

	return true;
}

void DisableTurboBoostBattery::stop(IOService *provider)
{
	stopPM();

	enable_tb();

	super::stop(provider);
}
