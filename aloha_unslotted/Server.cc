/*
 * server.cc
 *
 *  Created on: Jun 4, 2022
 *      Author: dibbendu
 */
/*
 * Server.cc
 *
 *  Created on: Jun 4, 2022
 *      Author: dibbendu
 *
 *      This module maintains the channel state and effectively simulates Aloha
 *      Hosts send arrivals/retransmissions. On arrival, if the link or
 *      channel is already busy or occupied, there is a collision and the server
 *      indicates there is a collision by sending a message to both colliding hosts. If link is free,
 *      the channel is kept busy till a packet duration (txtime). If there are no
 *      arrivals/retransmits during this period, then a successful transmission
 *      occurs and transmission ends.
 *
 *      Although packets from multiple hosts can collide at a time, we keep
 *      track of only the latest ongoing transmission for implementation purposes
 *      and hence capture only pairwise collisions.
 *      We use cModule *currhost to indicate the host currently occcupying the channel.
 *      With each arrival/retransmit, this is updated.
 *      We use cModule *succhost to indicate the host that might become successful.
 */

#include<omnetpp.h>

using namespace omnetpp;
using namespace std;

class Server: public cSimpleModule
{
private:
    cMessage *endRx,*endBusy;//self messages for ending reception and freeing the channel
    bool linkbusy;//control state of channel
    int success=0,attempts=0;//for computing success probability
    cModule *currhost,*succhost;//track which host is currently occupying channel or may be successful
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
    linkbusy=false;//keep state of link or channel
//    txhost = getModuleByPath("host[0]");
    int packetsize = getModuleByPath("host[0]")->par("packet_size").intValue();
    double transrate = getModuleByPath("host[0]")->par("txrate").doubleValue();
    txtime = (1.0*packetsize)/transrate;//packet duration
}
void Server::handleMessage(cMessage *msg)
{
    if(strcmp(msg->getName(),"arrival")==0||strcmp(msg->getName(),"retransmit")==0)
    {
        /*
         * If hosts send a new transmission or retransmit
         */

        attempts++;
        if(linkbusy)
        {
            /*
             * if link is busy and we are here implies there must be a collision
             * following code is just creates a visual tip that there has been a
             * collision at the server between two hosts.
             */

            if (hasGUI()) {
                char buf[32];
                sprintf(buf, "Collision! with hosts %d and %d",currhost->getIndex(),msg->getSenderModule()->getIndex());
                bubble(buf);
            }
            /*
             * Send message to colliding hosts. There may be two cases. Ongoing
             * transmission was of a different host than currently requesting host.
             * In this case both hosts have to be informed. Otherwise, the currently
             * occupying host has retransmitted leading to a collision. In this case,
             * only the current host needs to be informed about collision.
             */
            cMessage *collision = new cMessage("collision");
            if(currhost->getIndex() != msg->getSenderModule()->getIndex())
            {
                sendDirect(collision->dup(), currhost->gate("in"));
            }
            sendDirect(collision, msg->getSenderModule()->gate("in"));
            /*
             * Since a collision has occurred, the time when channel gets to be free
             * is updated by the packet duration that came from the latest/last
             * colliding host. Further, if a successful transmission was scheduled to
             * end has to be cancelled as collision will disrupt the transmission.
             */
            cancelEvent(endBusy);//cancel the current channel freeing event as it has to be readjusted
            cancelEvent(endRx);//cancel the successful reception complete event as collision occurred
            succhost = nullptr;//successful host is unset due to collision
            //            succgate = nullptr;
            currhost = msg->getSenderModule();//current host is updated by the one that has sent the last arrival/retransmission

            scheduleAt(simTime()+txtime,endBusy);//readjusting the time for freeing the channel
        }
        else
        {
            /*
             * If link is free, we start the transmission of a packet and schedule the
             * end of transmission as well as free the channel. Both currhost and
             * succhost are updated. link is set to busy
             */
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
        /*
         * Transmission was successful without collisions
         * We send a message to the successful host
         */
//        cout<<"Successfully Transmitted"<<endl;
//        cout<<txhost->getIndex()<<endl;
        success++;
        cMessage *success = new cMessage("success");
        sendDirect(success, succhost->gate("in"));
        succhost = nullptr;
    }
    if(msg==endBusy)
    {
        /*
         * channel is cleared after either a packet has been successfully transmitted
         * or the currently collided packet duration has ended.
         */
        linkbusy = false;
        currhost = nullptr;
    }
}
void Server::finish()
{
    /*
     * Printing and recording results
     */
    double prob_suc = (1.0*success)/attempts;
    cout<<"success_prob = "<<prob_suc<<endl;
    double uti = (1.0*success*txtime)/simTime();
    cout<<"uti = "<<uti*100<<endl;
    recordScalar("prob_succ", prob_suc);
    recordScalar("uti", uti*100);
}






