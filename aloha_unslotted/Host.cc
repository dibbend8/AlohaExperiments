/*
 * host.cc
 *
 *  Created on: Jun 4, 2022
 *      Author: dibbendu
 */

#include<omnetpp.h>

using namespace omnetpp;
using namespace std;

class Host: public cSimpleModule
{
private:
    cPar *txRate,*iaTime,*pklen;//parameters from INI
    double arrrate;// arrival rate from INI
    double retransrate;//retransmission rate from INI
    cModule *server;//to get server parameters or gate if required
    cMessage *arrival,*retransmit;//self messages/events for arrivals and retransmissions
    bool isretransmit = false;//if retransmissions are going on discard new arrivals

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
public:
    ~Host()
    {
        cancelAndDelete(arrival);
        cancelAndDelete(retransmit);
    }
};
Define_Module(Host);
void Host::initialize(){

    server = getModuleByPath("server");
    txRate = &par("txrate");
    arrrate = par("arrrate").doubleValue();
//    retransrate = par("retransrate").doubleValue();
    retransrate = arrrate;
    pklen = &par("packet_size");
    arrival = new cMessage("arrival");
    retransmit = new cMessage("retransmit");
    scheduleAt(simTime()+exponential(1.0/arrrate),arrival);// schedule new arrivals
//    scheduleAfter(iaTime->doubleValue(),arrival);
}
void Host::handleMessage(cMessage *msg){
    if(msg==arrival)
    {
        /*
         * This handles new arrivals. If retransmissions are going on, skip new arrivals
         * Otherwise create a new message/packet and send to the server
         */
        if(!isretransmit)
        {
            cMessage *arr = new cMessage("arrival");//create new arrival msg
            sendDirect(arr,server->gate("in"));//send to server
            EV<<"New packet transmitting from "<<getIndex()<<" at "<<simTime()<<endl;
        }
        scheduleAt(simTime()+exponential(1.0/arrrate),arrival);//schedule new arrivals

    }
    if(msg==retransmit)
    {
        /*
         * This handles retransmissions. This is triggered if collision message
         * arrives from the server.
         */
        cMessage *arr = new cMessage("retransmit"); //create a new retransmit message
        sendDirect(arr,server->gate("in"));//send to the server
        EV<<"retransmit packet retransmitted from "<<getIndex()<<" at "<<simTime()<<endl;

    }

    if(strcmp(msg->getName(),"collision")==0)
    {
        /*
         * This handles collisions i.e the host receives feedback from the server
         * that its data has collided. In this case, a retransmission is scheduled after
         * waiting for an exponentially distributed interval
         */
        cancelEvent(retransmit);
        isretransmit  = true;// retransmissions are going on. arrivals will be discarded
        simtime_t retransmittime = simTime()+exponential(1.0/retransrate);
        scheduleAt(retransmittime,retransmit);
//        cout<<"Retransmit scheduled for "<<getIndex()<<" at "<< retransmittime<<endl;
        delete msg;
    }
    else if(strcmp(msg->getName(),"success")==0)
    {
        //new arrival transmission or retransmission was successful.
        isretransmit = false;
        delete msg;
    }

}
