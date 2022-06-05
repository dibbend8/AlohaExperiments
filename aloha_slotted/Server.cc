/*
 * Server.cc
 *
 *  Created on: Jun 4, 2022
 *      Author: dibbendu
 *      See unslotted aloha for the detailed description.
 *      Server code remains unchanged.
 */

#include<omnetpp.h>

using namespace omnetpp;
using namespace std;

class Server: public cSimpleModule
{
private:
    cMessage *endRx,*endBusy;
    bool linkbusy;
    int success=0,attempts=0;
    cModule *currhost,*succhost;
    simtime_t txtime;
//    cGate *succgate;
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    void finish() override;
public:
    ~Server()
    {
        cancelAndDelete(endRx);
        cancelAndDelete(endBusy);
    }
};
Define_Module(Server);
void Server::initialize()
{
    endRx = new cMessage("end_reception");
    endBusy = new cMessage("endbusy");
    linkbusy=false;
//    txhost = getModuleByPath("host[0]");
    int packetsize = getModuleByPath("host[0]")->par("packet_size").intValue();
    double transrate = getModuleByPath("host[0]")->par("txrate").doubleValue();
    txtime = (1.0*packetsize)/transrate;
}
void Server::handleMessage(cMessage *msg)
{
    if(strcmp(msg->getName(),"arrival")==0||strcmp(msg->getName(),"retransmit")==0)
    {
//        cout<<"transmitting host"<<txhost->getIndex()<<endl;
//        cout<<msg->getName()<< " received from "<<msg->getSenderModule()->getIndex()<<" at "<<simTime()<<endl;

        attempts++;
        if(linkbusy)
        {
//            cout<<"currently occupying  "<<currhost->getIndex()<<endl;
//            EV<<"Collision"<<endl;
//            cout<<"Collision between  "<< currhost->getIndex()<<" and "<<msg->getSenderModule()->getIndex()<<endl;
//            if(currhost->getIndex() == msg->getSenderModule()->getIndex())
//            {
//
//                endSimulation();
//            }
            if (hasGUI()) {
                char buf[32];
                sprintf(buf, "Collision! with hosts %d and %d", msg->getSenderModule()->getIndex(),currhost->getIndex());
                bubble(buf);
            }
            cMessage *collision = new cMessage("collision");
            if(currhost->getIndex() != msg->getSenderModule()->getIndex())
            {
                sendDirect(collision->dup(), currhost->gate("in"));
            }
            sendDirect(collision, msg->getSenderModule()->gate("in"));
            cancelEvent(endBusy);
            cancelEvent(endRx);
            succhost = nullptr;
            //            succgate = nullptr;
            currhost = msg->getSenderModule();

            scheduleAt(simTime()+txtime,endBusy);
        }
        else
        {
            EV<<"Link Free Start transmission"<<endl;


            scheduleAt(simTime()+txtime,endBusy);
            scheduleAt(simTime()+txtime,endRx);
            currhost = msg->getSenderModule();
            succhost = msg->getSenderModule();
//            cout<<txhost->getIndex()<<endl;
//            succgate = currhost->gate("in");

            linkbusy=true;

        }
        delete msg;
    }
    if(msg==endRx)
    {
//        cout<<"Successfully Transmitted"<<endl;
//        cout<<txhost->getIndex()<<endl;
        success++;
        cMessage *success = new cMessage("success");
        sendDirect(success, succhost->gate("in"));
        succhost = nullptr;
    }
    if(msg==endBusy)
    {
        linkbusy = false;
        currhost = nullptr;
    }
}
void Server::finish()
{
    double prob_suc = (1.0*success)/attempts;
    cout<<"success_prob = "<<prob_suc<<endl;
    double uti = (1.0*success*txtime)/simTime();
    cout<<"uti = "<<uti*100<<endl;
    recordScalar("prob_succ", prob_suc);
    recordScalar("uti", uti*100);
}






