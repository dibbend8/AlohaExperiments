# AlohaExperiments
These are some simulation scripts on aloha and slotted aloha created over OMNet++. These can serve as started scripts for 
further developments.

Unslotted Aloha simulation:
  
  We create a simulation setup for hosts transmitting to a server following exponential interarrivals.
  Whenever there is a new arrival, a host tries to transmit immediately. If there is a collision,
  the server informs the concerned hosts about the collisions and the hosts retransmit after exponentially 
  distributed random intervals. During retransmissions, all new arrivals are discarded at a host.
  
  Slotted Aloha Simulation:
    
  We create a simulation setup for hosts transmitting to a server following exponential interarrivals.
  Whenever there is a new arrival, a host tries to transmit only at the beginning of a slot. A slot is 
  equal to the packet transmission time. Retransmissions are also attempted only at begging of slots.
  Server operation is unchanged.
    
  
    
