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
    // Box2D �л���Լ�������(constraint solver)��Լ����������ڽ��ģ���е�����
    // Լ����һ��һ����������Լ���ᱻ��������⣬Ȼ�����������һ��Լ����ʱ�����Ǿͻ���΢������
    // һ����Ҫ�õ����õĽ⣬������Ҫ��������Լ����Ρ������ Box2D ���������� 10 �Ρ�����԰��Լ�
    // ��ϲ��ȥ�������������Ҫ�ǵ������ٶ�������֮���ƽ�⡣���ٵĵ������������ܲ����;��ȣ�ͬ��
    // �أ�����ĵ�����������ܵ����ģ����������������ѡ��ĵ���������
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
