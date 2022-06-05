/*
 * AlohaNode.cc
 *
 *  Created on: Jun 4, 2022
 *      Author: dibbendu
 */

#include<omnetpp.h>
using namespace omnetpp;
using namespace std;

class AlohaNode:public cSimpleModule
{
private:
    int numsources;
    double framedur,retransmitRate;
    cMessage *endtx, *retransmit,*endbusy;
    bool busychan = false;
    int attempts=0,success = 0;
protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
    void finish() override;
public:
    ~AlohaNode()
    {
        cancelAndDelete(retransmit);
        cancelAndDelete(endtx);
        cancelAndDelete(endbusy);
    }
};
Define_Module(AlohaNode);

void AlohaNode::initialize()
{
  framedur = 1;
  retransmitRate = par("retransmitRate").doubleValue() * framedur;
  retransmit = new cMessage("retransmit");
  endtx = new cMessage("end_tx");
  endbusy = new cMessage("endbusy");
  scheduleAt(simTime()+exponential(1.0/retransmitRate),retransmit);
}
void AlohaNode::handleMessage(cMessage *msg)
{
    if(msg==retransmit)
    {
        attempts++;
        EV<<"transmit attempt " << attempts<< " at "<<simTime()<<endl;
        if(!busychan)
        {
            EV<<"channel is free"<<endl;
            scheduleAt(simTime()+framedur,endtx);
            scheduleAt(simTime()+framedur,endbusy);
            busychan = true;
        }
        else
        {
          EV<<"channel is busy.. cancelling transmission"<<endl;
          cancelEvent(endtx);
          cancelEvent(endbusy);
          scheduleAt(simTime()+framedur,endbusy);

        }
        scheduleAt(simTime()+exponential(1.0/retransmitRate),retransmit);

    }
    if(msg==endtx)
    {
        success++;
        EV<<"successfully transmitted"<<endl;
//        busychan = false;
//        EV<<"channel freed"<<endl;
    }
    if(msg==endbusy)
    {
        busychan = false;
        EV<<"channel freed"<<endl;
    }

}
void AlohaNode::finish()
{
        cout<<"success prob = "<<(1.0*success)/attempts<<endl;
        recordScalar("successprob",(1.0*success)/attempts);
}

