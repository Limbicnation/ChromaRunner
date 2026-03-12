#pragma once

#include "CoreMinimal.h"

/**
 * Generic actor pool template for efficient object reuse.
 * Extracted from CoinPickup.h for shared use across procedural generation systems.
 *
 * PERFORMANCE: Eliminates SpawnActor/DestroyActor overhead by reusing deactivated actors.
 */
template<class T>
class FActorPool
{
public:
    /**
     * Retrieves an actor from the pool, or nullptr if pool is empty.
     *
     * @param Tag - Optional tag to retrieve from a specific sub-pool
     * @return Pooled actor or nullptr
     */
    T* GetActor(FName Tag = NAME_None)
    {
        TArray<T*>& Pool = PooledActors.FindOrAdd(Tag);
        if (Pool.Num() > 0)
        {
            T* Actor = Pool.Pop();
            ActiveActors.Add(Actor);
            return Actor;
        }
        return nullptr;
    }

    /**
     * Returns an actor to the pool for future reuse.
     *
     * @param Actor - Actor to return
     * @param Tag - Optional tag for the sub-pool
     */
    void ReturnActor(T* Actor, FName Tag = NAME_None)
    {
        if (Actor)
        {
            TArray<T*>& Pool = PooledActors.FindOrAdd(Tag);
            Pool.Add(Actor);
            ActiveActors.Remove(Actor);
        }
    }

    /** Returns total number of pooled (inactive) actors across all tags. */
    int32 GetPooledCount() const
    {
        int32 Count = 0;
        for (const auto& Pair : PooledActors)
        {
            Count += Pair.Value.Num();
        }
        return Count;
    }

    /** Returns number of currently active (checked-out) actors. */
    int32 GetActiveCount() const
    {
        return ActiveActors.Num();
    }

    /** Clears all pool data. Does NOT destroy actors. */
    void Clear()
    {
        PooledActors.Empty();
        ActiveActors.Empty();
    }

private:
    TMap<FName, TArray<T*>> PooledActors;
    TArray<T*> ActiveActors;
};
