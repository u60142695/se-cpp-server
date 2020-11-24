#include "PlayerManager.h"

PlayerManager* PlayerManager::ms_instance = 0;

PlayerManager* PlayerManager::GetInstance()
{
    if (!ms_instance)
    {
        ms_instance = new PlayerManager();
    }

    return ms_instance;
}
