#include "types.h"
#include "world.h"

world::world()
{
    m_b2world = NULL;
	m_itrn = 10;
}

world::~world()
{
    if (m_b2world)delete m_b2world;
    m_b2world = NULL;
}

BOOL world::InitWorld(WorldDef &wd)
{
    if (NULL != m_b2world)
    {
        return false;
    }

    b2AABB worldAABB;
    worldAABB.lowerBound.Set(0, 0);
    worldAABB.upperBound.Set(wd.w, wd.h);
    b2Vec2 gravity(0.0f, wd.g);
    bool doSleep = true;
    m_b2world = new b2World(worldAABB, gravity, doSleep);
    if (NULL == m_b2world)
    {
        return false;
    }



    if (wd.BoundSide)
    {
        b2BodyDef sideDef;
        b2Body* sideBody;
        b2PolygonDef sideShapeDef;
        FLOAT sideh = 0.1f;
        
       //botton side
        sideDef.position.Set(wd.w/2, 0);
        sideBody = m_b2world->CreateBody(&sideDef);
        sideShapeDef.SetAsBox(wd.w, sideh);
        sideBody->CreateShape(&sideShapeDef);

       //top side
        sideDef.position.Set(wd.w/2, wd.h);
        sideBody = m_b2world->CreateBody(&sideDef);
        sideShapeDef.SetAsBox(wd.w, sideh);
        sideBody->CreateShape(&sideShapeDef);

       //left side
        sideDef.position.Set(0, wd.h/2);
        sideBody = m_b2world->CreateBody(&sideDef);
        sideShapeDef.SetAsBox(sideh, wd.h);
        sideBody->CreateShape(&sideShapeDef);

       //right side
        sideDef.position.Set(wd.w/2, wd.h);
        sideBody = m_b2world->CreateBody(&sideDef);
        sideShapeDef.SetAsBox(sideh, wd.h);
        sideBody->CreateShape(&sideShapeDef);
    }
    return true;
}

BOOL world::Update(FLOAT dt)
{
    // Box2D 中还有约束求解器(constraint solver)。约束求解器用于解决模拟中的所有
    // 约束，一次一个。单个的约束会被完美的求解，然而当我们求解一个约束的时候，我们就会稍微耽误另
    // 一个。要得到良好的解，我们需要迭代所有约束多次。建议的 Box2D 迭代次数是 10 次。你可以按自己
    // 的喜好去调整这个数，但要记得它是速度与质量之间的平衡。更少的迭代会增加性能并降低精度，同样
    // 地，更多的迭代会减少性能但提高模拟质量。这是我们选择的迭代次数：
    m_b2world->Step(1.0f/60.0f,m_itrn);
	return true;
}

BOOL world::CreateObj(ObjDef &objdef, b2obj &b2obj)
{
    BOOL bret = false;
	b2BodyDef b2bodydef;
    b2Body *pb2body = NULL;
	b2PolygonDef b2polygondef;
	b2Shape *pb2sharp = NULL;

    if (objdef.w == 0.0f || objdef.h == 0.0f)
    {
        goto EXIT_LABEL;
	}
	
	b2bodydef.position.Set(objdef.x,objdef.y);
	pb2body = m_b2world->CreateBody(&b2bodydef);
	if (!pb2body)
    {
        goto EXIT_LABEL;
	}

    b2polygondef.SetAsBox(objdef.w/2, objdef.h/2);
	b2polygondef.density = 1;
	b2polygondef.friction = 0.3f;
	pb2sharp = pb2body->CreateShape(&b2polygondef);
	if (!pb2sharp)
    {
        goto EXIT_LABEL;
	}

    pb2body->SetMassFromShapes();
	b2obj.m_b2body = pb2body;
	
    bret = true;
	
EXIT_LABEL:
    if (!bret)
    {
        if (pb2sharp)
        {
            pb2body->DestroyShape(pb2sharp);
			pb2sharp = NULL;
		}

		if (pb2body)
		{
            m_b2world->DestroyBody(pb2body);
			pb2body = NULL;
		}
	}
	
	return bret;
}

void world::DrawWorld()
{

}
