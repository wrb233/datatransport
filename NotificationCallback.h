#ifndef NOTIFICATIONCALLBACK_H
#define NOTIFICATIONCALLBACK_H

#include "common.h"


void notificationOnlineStatus(const Notification* notif, void* clientdata);

void notificationChannelState(const Notification* notif, void* clientdata);

void notificationDPCAction(const Notification* notif, void* clientdata);

void notificationMVPointValue(const Notification* notif, void* clientdata);

void notificationIncrementCommit(const Notification* notif, void* clientdata);

#endif