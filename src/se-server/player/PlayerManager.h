#ifndef PLAYER_MANAGER_H
#define PLAYER_MANAGER_H

class PlayerManager
{
public:
    static PlayerManager* GetInstance();
private:
    static PlayerManager* ms_instance;
};

#endif
