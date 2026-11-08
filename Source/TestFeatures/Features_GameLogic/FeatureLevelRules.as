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
