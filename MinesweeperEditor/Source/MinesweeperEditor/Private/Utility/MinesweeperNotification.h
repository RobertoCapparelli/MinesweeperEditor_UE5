#pragma once
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

namespace FMinesweeperNotification
{
	inline void Show(const FText& Message, SNotificationItem::ECompletionState State = SNotificationItem::CS_None, float FadeOutTime = 2.5f)
	{
		FNotificationInfo Info(Message);
		Info.bFireAndForget = true;
		Info.FadeOutDuration = FadeOutTime;
		Info.ExpireDuration = 2.5f;
		Info.bUseLargeFont = true;
		auto Notification = FSlateNotificationManager::Get().AddNotification(Info);
		if (Notification.IsValid() && State != SNotificationItem::CS_None)
		{
			Notification->SetCompletionState(State);
		}
	}
}