#include"ObjectTracker.h"
#include<opencv.hpp>
using namespace std;
INT32 imW,imH;
IMAGE_TYPE eImageType;
SINT16 x,y,Width,Height;
UBYTE8 *frame;
int main()
{
	CObjectTracker *m_pObjectTracker = new CObjectTracker(imW,imH, 
                                                      eImageType);
	m_pObjectTracker->ObjectTrackerInitObjectParameters(x,y,
                                                     Width,Height);
	m_pObjectTracker->ObjeckTrackerHandlerByUser(frame);

	delete m_pObjectTracker, m_pObjectTracker = 0;

}