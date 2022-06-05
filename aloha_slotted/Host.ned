//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

//
// TODO auto-generated type
//
simple Host
{
    parameters:
        @display("i=device/pc");
        double arrrate;//arrival rate (to be set in the INI file)
//        double retransrate;//Retransmit rate (to be set in the INI file)
        int packet_size @unit(B);//Packet size in bytes (to be set in the INI file)
        double x @unit(m);// positionx in the graph (not required unless propagation delay is considered)
        double y  @unit(m); // positiony in the graph (not required unless propagation delay is considered)
        double txrate @unit(Bps);//Transmission rate in bytes per second (to be set in the INI file)
    gates:
       input in @directIn;//Input gate for the hosts (currently wireless)     
        
        
}
