# Projeto RCI

## Checklist

### RMB
- [x] Interface  
- [x] Protocol UDP: GET_SERVERS ; SERVERS\n(n;i;u;t\n)  
- [x] Protocol UDP: PUBLISH msg; GET_MESSAGES n; MESSAGES\n(message\n)  
- [x] Fetch servers from id server. Exit if there are no answers. Refresh if no server presentin answer.  
- [x] User: message 140 chars only, show_latest n with positive diferent of zero number  
- [x] Servers saved in lists. (Banned and fetched)  
- [x] Print Servers List (Only useful information)  
- [x] Print Messages received (Minor treatment)  
- [ ] Memory Leakage  
- [x] Gracefuly Termination: Errors in system calls, Wrongly Formated Protocol Messages(Don't exit just discard)


### MSGServ
- [x] LC: UDP:LC<-LC+1; TCP:LC<-max(LC,incomingLC)+1  
- [x] Interface  
- [x] Protocol UDP: GET_SERVERS ; SERVERS\n(n;i;u;t\n) ; REG n;i;u;t
- [x] Register in id server. Update the registration  
- [x] Fetch servers from id server, connect to those, and ask for messages to one  
- [x] If none servers are present, just proceed with 0 messages.  
- [x] Protocol UDP: PUBLISH msg; GET_MESSAGES n; MESSAGES\n(message\n)    
- [x] Protocol TCP: SGET_MESSAGES\n; SMESSAGES\n(clock;message\n)\n  
- [x] Servers saved in lists. (Connected)  
- [ ] Messages saved in matrix. (Initialized space on demand)  
- [x] Print Servers list (Only useful information)  
- [x] Print Messages array (As is, with LC)  
- [x] Memory Leakage  
- [x] Gracefuly Termination: Errors in system calls, Wrongly Formated Protocol 
Messages (Don't exit, just discard), TCP session umpredicted termination.  

### Submission
- [ ] zipped, proj\<GroupNumber\>.zip ex:proj28.zip
- [ ] Done and submited by Friday, April 7, 23h59. Via e-mail to the lab teacher 
(Paulo Correia - paulo.lobato.correia@tecnico.ulisboa.pt)


