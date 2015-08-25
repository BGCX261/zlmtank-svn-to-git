#include "def.h"
#include "protocol.h"
#include "tank_server.h"



int main(int argc, char **argv)
{
    BOOL bret = true;

    bret = InitGameDB();
    if (!bret)
    {
        return -1;
    }
    tank *ptank = new tank;
    bret = ptank->Init();
    if (bret)
    {
        RegisterUE(ptank);
        PRINTF(" registe class::tank\n");
    
        bret = StartServer();
    }

    bret = ShutdownServer();

    RegisterUE(NULL);
    PRINTF(" UnRegiste class::tank\n");
    
    bret = FreeGameDB();



    PRINTF("exit server...");
    
    
    return 0;
}