// Classifies a contact into bounce-count and retirement effects for the native ball.
// Inputs and the returned action flags are copied values; BallContactAction constants are registered from C++.
int EvaluateBallContact(bool PlayerContact, bool HasSolverImpulse, int BounceCount, int ReboundLimit)
{
    if (PlayerContact)
    {
        return BallContactAction::Retire;
    }
    if (!HasSolverImpulse)
    {
        return BallContactAction::None;
    }

    int Action = BallContactAction::CountBounce;
    if (BounceCount + 1 >= ReboundLimit)
    {
        Action |= BallContactAction::Retire;
    }
    return Action;
}

// Allows retired balls to be replaced while their gameplay session is active.
// SessionPlaying and the bool result are copied values without pointers, references, or owned state.
bool ShouldReplaceBall(bool SessionPlaying)
{
    return SessionPlaying;
}
