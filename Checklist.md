# Projeto RCI

## Checklist

### RMB
- [ ] Interface  
- [ ] Protocol UDP: GET_SERVERS ; SERVERS\n(n;i;u;t\n)  
- [ ] Protocol UDP: PUBLISH msg; GET_MESSAGES n; MESSAGES\n(message\n)  
- [ ] Fetch servers from id server. Exit if there are no answers. Refresh if no server presentin answer.  
- [ ] User: message 140 chars only, show_latest n with positive diferent of zero number  
- [ ] Servers saved in lists. (Banned and fetched)  
- [ ] Print Servers List (Only useful information)  
- [ ] Print Messages received (Minor treatment)  
- [ ] Memory Leakage  
- [ ] Gracefuly Termination: Errors in system calls, Wrongly Formated Protocol Messages(Don't exit just discard)


### MSGServ
- [ ] LC: UDP:LC<-LC+1; TCP:LC<-max(LC,incomingLC)+1  
- [ ] Interface  
- [ ] Protocol UDP: GET_SERVERS ; SERVERS\n(n;i;u;t\n)  
- [ ] Register in id server. Update the registration  
- [ ] Fetch servers from id server, connect to those, and ask for messages to one  
- [ ] If none servers are present, just proceed with 0 messages.  
- [ ] Protocol UDP: PUBLISH msg; GET_MESSAGES n; MESSAGES\n(message\n)    
- [ ] Protocol TCP: SGET_MESSAGES\n; SMESSAGES\n(clock;message\n)\n  
- [ ] Servers saved in lists. (Connected)  
- [ ] Messages saved in matrix. (Initialized space on demand)  
- [ ] Print Servers list (Only useful information)  
- [ ] Print Messages array (As is, with LC)  
- [ ] Memory Leakage  
- [ ] Gracefuly Termination: Errors in system calls, Wrongly Formated Protocol 
Messages (Don't exit, just discard), TCP session umpredicted termination.  

### Submition
- [ ] zipped, proj\<GroupNumber\>.zip ex:proj28.zip
- [ ] Done and submited by Friday, April 7, 23h59. Via e-mail to the lab teacher 
(Paulo Correia - paulo.lobato.correia@tecnico.ulisboa.pt)


