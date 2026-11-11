// Calculates the player-contact target from the C++ configuration and saved level.
int PlayerContactsToWin(int Level, int BaseContacts, int IncreasePerLevel)
{
    return BaseContacts + IncreasePerLevel * (Level - 1);
}

// Calculates the non-player bounce limit, stopping at the configured minimum.
int SessionBouncesToLose(int Level, int BaseBounces, int DecreasePerLevel, int MinimumBounces)
{
    int Limit = BaseBounces - DecreasePerLevel * (Level - 1);
    return Limit < MinimumBounces ? MinimumBounces : Limit;
}

// Chooses victory before defeat when both configured limits have been reached.
// SessionResult is registered from C++.
SessionResult SessionOutcome(int PlayerContacts, int Bounces, int WinLimit, int LossLimit)
{
    if (PlayerContacts >= WinLimit)
    {
        return SessionResult::Victory;
    }
    if (Bounces >= LossLimit)
    {
        return SessionResult::Defeat;
    }
    return SessionResult::InProgress;
}

// Advances after victory and retains the level for other session results.
int NextLevel(int Level, SessionResult Result)
{
    return Result == SessionResult::Victory ? Level + 1 : Level;
}
