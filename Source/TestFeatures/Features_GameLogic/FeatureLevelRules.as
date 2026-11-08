// Calculates the player-contact target from the C++ configuration and saved level.
// All int arguments and the return value are copied; no pointers, references, or state ownership.
int PlayerContactsToWin(int Level, int BaseContacts, int IncreasePerLevel)
{
    return BaseContacts + IncreasePerLevel * (Level - 1);
}

// Calculates the non-player bounce limit, stopping at the configured minimum.
// All int arguments and the return value are copied; no pointers, references, or state ownership.
int SessionBouncesToLose(int Level, int BaseBounces, int DecreasePerLevel, int MinimumBounces)
{
    int Limit = BaseBounces - DecreasePerLevel * (Level - 1);
    return Limit < MinimumBounces ? MinimumBounces : Limit;
}

// Chooses victory before defeat when both configured limits have been reached.
// Counts, limits, and the C++-registered SessionResult enum are copied values; no pointers or references.
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
// Level, the native result enum, and the returned int are copied values without pointers or references.
int NextLevel(int Level, SessionResult Result)
{
    return Result == SessionResult::Victory ? Level + 1 : Level;
}
