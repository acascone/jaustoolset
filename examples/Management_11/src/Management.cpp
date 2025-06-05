

#include "Management.h"
#include "JausUtils.h"
#include "urn_jaus_jss_core_Management_1_1/Messages/MessageSet.h"

using namespace JTS;
using namespace urn_jaus_jss_core_Transport_1_1;
using namespace urn_jaus_jss_core_Events_1_1;
using namespace urn_jaus_jss_core_AccessControl_1_1;
using namespace urn_jaus_jss_core_Management_1_1;


Management::Management(unsigned int subsystem, unsigned short node, unsigned short component)
{
	jausRouter = new JausRouter(JausAddress(subsystem, node, component), ieHandler);
	
	/// Instantiate services
	TransportService* pTransportService = new TransportService(jausRouter);
	EventsService* pEventsService = new EventsService(jausRouter, pTransportService);
	AccessControlService* pAccessControlService = new AccessControlService(jausRouter, pTransportService, pEventsService);
	ManagementService* pManagementService = new ManagementService(jausRouter, pTransportService, pEventsService, pAccessControlService);
	
	
	/// Add all the Services for the Component
	serviceList.push_back(pTransportService);
	serviceList.push_back(pEventsService);
	serviceList.push_back(pAccessControlService);
	serviceList.push_back(pManagementService);
	
}

Management::~Management()
{
	Service* service;
	
	while (!serviceList.empty())
	{
		service = serviceList.back();
		serviceList.pop_back();
		
		delete service;
	}
	
	delete jausRouter;
}


void Management::startComponent()
{
	Service* service;
	
	jausRouter->start();
	this->start();
	
	for (unsigned int i = 0; i < serviceList.size(); i++)
	{
		 service = serviceList.at(i);
		 service->start();
	}
}


void Management::shutdownComponent()
{
	Service* service;
	
	for (unsigned int i = 0; i < serviceList.size(); i++)
	{
		 service = serviceList.at(i);
		 service->stop();
	}
	
	this->stop();
	jausRouter->stop();
}

void Management::processInternalEvent(InternalEvent *ie)
{
    bool done = false;

	if (ie->getName().compare("Receive") == 0)
	{
		Receive* casted_ie = (Receive*) ie;
		unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
		if ( id == urn_jaus_jss_core_Management_1_1::ClearEmergency::ID) printf("Received ClearEmergency\n");
		else if ( id == urn_jaus_jss_core_Management_1_1::SetEmergency::ID) printf("Received SetEmergency\n");
		else if ( id == urn_jaus_jss_core_Management_1_1::Reset::ID) printf("Received Reset\n");
		else if ( id == urn_jaus_jss_core_Management_1_1::Resume::ID) printf("Received Resume\n");
		else if ( id == urn_jaus_jss_core_Management_1_1::Standby::ID) printf("Received Standby\n");
		else if ( id == urn_jaus_jss_core_Management_1_1::QueryStatus::ID) printf("Received QueryStatus\n");
		else if ( id == urn_jaus_jss_core_Management_1_1::RequestControl::ID) printf("Received RequestControl\n");
		else if ( id == urn_jaus_jss_core_Management_1_1::ReleaseControl::ID) printf("Received ReleaseControl\n");
		else if ( id == urn_jaus_jss_core_Management_1_1::QueryControl::ID) printf("Received QueryControl\n");
		else if ( id == urn_jaus_jss_core_Management_1_1::SetAuthority::ID) printf("Received SetAuthority\n");
		else printf("Received message: 0x%x\n", id);
	}
	
	//
	// When a component receives an internal event, it passes it
	// to the services to handling, children services first.  If the
	// event is not processed by normal transitions, it's passed
	// again to the services (children first) for default transitions.
	// A given event may only be processed by at most one service.
	//
	for (unsigned int i = serviceList.size(); i>0; i--)
	{
		if (!done) done = serviceList.at(i-1)->processTransitions(ie);
	}
	for (unsigned int i = serviceList.size(); i>0; i--)
	{
		if (!done) done = serviceList.at(i-1)->defaultTransitions(ie);
	}
}



