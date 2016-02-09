# HPRS
Hadoop Parallel rendering System


#If SSH Tunnel Required  use the following syntax
- `ssh -f user@personal-server.com -L 2000:personal-server.com:25 -N`
1. `-f` ssh in background
2. `-L 2000:personal-server.com:25` if of the form *-L local-port:host:remote-port*
This essentially forwards the local port 2000 to port 25 on personal-server.com over, with nice benefit of being encrypted. I
