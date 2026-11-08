// Calculates active contact bonuses, including the disabled-duration rule.
// All arguments and the int result are copied values; no pointers, references, or owned state.
int PlayerSpeedBonusCount(int PreviousBonuses, bool StackBonuses, float Duration)
{
    if (Duration <= 0.0f)
    {
        return 0;
    }
    return StackBonuses ? PreviousBonuses + 1 : 1;
}

// Calculates baseline or boosted walking speed from the character's current rule data.
// Float and int values are copied across the bridge; the script retains no configuration or state.
float PlayerWalkSpeed(float NativeWalkSpeed, float BaseMultiplier, float ContactMultiplier, int BonusCount)
{
    const float BaseWalkSpeed = NativeWalkSpeed * BaseMultiplier;
    if (BonusCount == 0)
    {
        return BaseWalkSpeed;
    }
    const float Multiplier = 1.0f + float(BonusCount) * (ContactMultiplier - 1.0f);
    return BaseWalkSpeed * (Multiplier < 0.0f ? 0.0f : Multiplier);
}
