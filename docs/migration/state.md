# State Flag Migration

All per-context capability flags now reside in `RenderContext` instead of the legacy `GLState` singleton. Each flag carries an atomic version counter so worker threads can cheaply detect updates.

This completes the state unification effort.
