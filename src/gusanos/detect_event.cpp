#include "gusanos/detect_event.h"

#include "gusanos/events.h"
#include "CGameObject.h"
#include "gusanos/gusgame.h"
#include "util/macros.h"
#include "game/Game.h"

DetectEvent::DetectEvent( float range, bool detectOwner, int detectFilter)
: m_range(range), m_detectOwner(detectOwner), m_detectFilter(detectFilter)
{
	//m_event = new GameEvent;
}

DetectEvent::DetectEvent(Actions& actions_, float range, bool detectOwner, int detectFilter)
: GameEvent(actions_)
, m_range(range), m_detectOwner(detectOwner), m_detectFilter(detectFilter)
{
	
}

DetectEvent::~DetectEvent()
{
	//delete m_event;
}

void DetectEvent::check( CGameObject* ownerObject )
{
#ifdef USE_GRID
	// TODO: Detect event
	
	int x = int(ownerObject->pos().x);
	int y = int(ownerObject->pos().y);
	int radius = int(m_range);
	int x1 = x - radius;
	int y1 = y - radius;
	int x2 = x + radius;
	int y2 = y + radius;
	
	if ( m_detectFilter & 1 ) // 1 is the worm collision layer flag
	{
/*
		for ( Grid::area_iterator worm = game.objects.beginArea(x1, y1, x2, y2, Grid::WormColLayer); worm; ++worm)
		{
			if(&*worm != ownerObject)
			{
				if ( m_detectOwner || worm->getOwner() != ownerObject->getOwner() )
				if ( worm->isCollidingWith( ownerObject->pos, m_range) )
				{
					m_event->run( ownerObject, &*worm );
				}
			}
		}*/
		

		//for ( Grid::iterator worm = game.objects.beginColLayer(Grid::WormColLayer); worm; ++worm)
		forrange_bool(worm, game.objects.beginColLayer(Grid::WormColLayer))
		{
			if(&*worm != ownerObject)
			{
				if ( m_detectOwner || worm->getOwner() != ownerObject->getOwner() )
				if ( worm->isCollidingWith( ownerObject->pos(), m_range) )
				{
					//m_event->run( ownerObject, &*worm );
					run( ownerObject, &*worm );
				}
			}
		}
	}
		
	for ( int customFilter = Grid::CustomColLayerStart, filterFlag = 2; customFilter < Grid::ColLayerCount; ++customFilter, filterFlag*=2 )
	{
		if ( m_detectFilter & filterFlag )
		{
			//for ( Grid::area_iterator object = game.objects.beginArea(x1, y1, x2, y2, customFilter); object; ++object)
			forrange_bool(object, game.objects.beginArea(x1, y1, x2, y2, customFilter))
			{
				//cerr << "Found: " << &*worm << endl;
				if(&*object != ownerObject)
				{
					//cerr << "Found: " << &*worm << endl;
					if ( !object->deleteMe && (m_detectOwner || object->getOwner() != ownerObject->getOwner() ) )
					if ( object->isCollidingWith( ownerObject->pos(), m_range) )
					{
						//m_event->run( ownerObject, &*object );
						run( ownerObject, &*object );
					}
				}
			}
		}
	}
#else
	if ( m_detectFilter & 1 ) // 1 is the worm collision layer flag
	{
		ObjectsList::ColLayerIterator worm;
		for ( worm = game.objects.colLayerBegin(GusGame::WORMS_COLLISION_LAYER); worm; ++worm)
		{
			if ( m_detectOwner || (*worm)->getOwner() != ownerObject->getOwner() )
				if ( (*worm)->isCollidingWith( ownerObject->pos, m_range) )
			{
				//m_event->run( ownerObject, (*worm) );
				run( ownerObject, (*worm) );
			}
		}
	}

	// from CUSTOM_COL_LAYER_START to COLLISION_LAYERS_AMMOUNT its the particles collision layers
	for ( int customFilter = GusGame::CUSTOM_COL_LAYER_START, filterFlag = 2; customFilter < COLLISION_LAYERS_AMMOUNT; ++customFilter, filterFlag*=2 )
	{
		if ( m_detectFilter & filterFlag )
		{
			ObjectsList::ColLayerIterator object;
			for ( object = game.objects.colLayerBegin(customFilter); object; ++object)
			{
				if ( (*object) != ownerObject )
				if ( !(*object)->deleteMe && (m_detectOwner || (*object)->getOwner() != ownerObject->getOwner() ) )
				if ( (*object)->isCollidingWith( ownerObject->pos, m_range) )
				{
					//m_event->run( ownerObject,(*object) );
					run( ownerObject,(*object) );
				}
			}
		}
	}
#endif
}
