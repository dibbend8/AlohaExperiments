/*
 * host.cc
 *
 *  Created on: Jun 4, 2022
 *      Author: dibbendu
 *      This code simulates Hosts for Slotted Aloha. Each host transmits
 *      only at the slot boundary whether it be new arrivals or retransmissions.
 */

#include<omnetpp.h>

using namespace omnetpp;
using namespace std;

class Host: public cSimpleModule
{
private:
    cPar *txRate,*iaTime,*pklen;//parameters taken from INI
    double arrrate;//arrival rate also to be taken from the iNI
    double retransrate;//retransmission rate from INI
    simtime_t slottime,nextslot;//slottime is one slot duration and nextslot keeps track of slot
    cModule *server;//needed to access gates of server module
    cMessage *arrival,*retransmit,*slot;//self messages or self events for arrival, retransmit and generating slots
    bool isretransmit = false;
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
public:
    ~Host()
    {
        cancelAndDelete(arrival);
        cancelAndDelete(retransmit);
        cancelAndDelete(slot);
    }
};
Define_Module(Host);
void Host::initialize(){

    server = getModuleByPath("server");//get the server module for accessing sever parameters
    txRate = &par("txrate");//tranmission rate required to compute one slot duration
    arrrate = par("arrrate").doubleValue();//arrival rate from INI
    retransrate = arrrate;
//    retransrate = par("retransrate").doubleValue();//retransmission rate from INI
    pklen = &par("packet_size");//packet length of one packet in bytes from INI
    arrival = new cMessage("arrival");//arrival self message created to generate arrivals
    retransmit = new cMessage("retransmit");//retransmit self message created to retransmit
    slot = new cMessage("slot");// slot self message to generate time slots
    slottime = pklen->intValue()/txRate->doubleValue();//compute slot time = packet length/transmission rate
    nextslot = simTime()+slottime;//next slot time
    scheduleAfter(slottime,slot);//next slot scheduled after slottime
    scheduleAfter(exponential(1.0/arrrate),arrival);//next arrival scheduled with exponentially distributed interval
    //    scheduleAfter(iaTime->doubleValue(),arrival);
}
void Host::handleMessage(cMessage *msg){
    if(msg==slot)
    {

        nextslot = simTime()+slottime;//track next slot time
        scheduleAt(nextslot, slot);
        //        cout<<"slot is updating "<< "current time is "<<simTime()<<" next slot is at "<<nextslot<<endl;
    }
    if(msg==arrival)
    {
        /*
         * New arrivals are handled here. Arrivals arrive at exponential interarrivals.
         * If the arrival instant is at the start of a slot then immediate transmission
         * is attempted. Else transmission is attempted at the immediate next slot.
         */
        if(!isretransmit)
        {
            if(simTime()+slottime == nextslot)
            {
                //current time is the start of a slot then create a message and transmit
                cMessage *arr = new cMessage("arrival");
                sendDirect(arr,server->gate("in"));
                EV<<"New packet transmit from "<<getIndex()<<" at "<<simTime()<<endl;
            }
            else
            {
                //current time is not the start of a slot and hence schedule transmission to next slot
                scheduleAt(nextslot, arrival);
            }
        }
        if(!arrival->isScheduled())
        {
            scheduleAfter(exponential(1.0/arrrate),arrival);
                //        EV<<"New packet transmit from "<<getIndex()<<" at "<<simTime()<<endl;
        }
    }
    if(msg==retransmit)
    {
        // comes here if there was a collision and tries to retransmit after an
        //exponential duration
        if(simTime()+slottime==nextslot)
        {
            //check if the current time is start of a slot, then transmit in the current slot
            cMessage *arr = new cMessage("retransmit");
            sendDirect(arr,server->gate("in"));
            EV<<"packet retransmitted from "<<getIndex()<<" at "<<simTime()<<endl;
        }
        else
        {
            //wait/schedule for the next slot to retransmit
            scheduleAt(nextslot, retransmit);
        }


    }

    if(strcmp(msg->getName(),"collision")==0)
    {
        //if server informs that there is a collision, retransmit after an exponential duration
        cancelEvent(retransmit);
        isretransmit = true;
        simtime_t retransmittime = simTime()+exponential(1.0/retransrate);
        scheduleAt(retransmittime,retransmit);
        //        cout<<"Retransmit scheduled for "<<getIndex()<<" at "<< retransmittime<<endl;
        delete msg;
    }
    else if(strcmp(msg->getName(),"success")==0)
    {
        isretransmit = false;
        //if server informs that it is a success, generate new arrivals after an exponential
        //        scheduleAfter(exponential(1.0/arrrate),arrival);
        delete msg;
    }

}
