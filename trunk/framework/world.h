#ifndef _MAP_H
#define _MAP_H
#include <include\box2d.h>
#include "b2obj.h"

#define WORLDDEF_NAME  32
typedef struct _WorldDef
{
    CHAR name[WORLDDEF_NAME];
    FLOAT w;
    FLOAT h;
    FLOAT g;
    BOOL BoundSide;
}WorldDef;

typedef struct _Objdef
{
    FLOAT w;
    FLOAT h;
	FLOAT x;
	FLOAT y;
}ObjDef;



#define WORLD_FLAG_ITRN     0x00000001

typedef struct _WorldState
{
    DWORD flag;
	DWORD itrn;

	
	_WorldState()
    {
        flag = 0;
		itrn = 10;
	}

}WorldState;

class world
{
private:
    b2World *m_b2world;

    DWORD m_itrn;
	
public:
    world();
    ~world();

    BOOL InitWorld(WorldDef&);
	BOOL Update(FLOAT);
    BOOL CreateObj(ObjDef&,b2obj&);

	void DrawWorld();


};


#endif
