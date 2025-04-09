# smb2tcp
smb2tcp is a Windows tool for TCP port forwarding over SMB named pipes. It allows both local and remote port forwarding, and can run without admin privileges on the client and server machines.
## Installation
To install smb2tcp, clone the repository and build the projects using Visual C++.
## Usage
On the server machine, run with any user:
```
smb2tcp-server.exe
```
On the client machine, for local port forwarding:
```
smb2tcp-client.exe -L <listen_address>:<listen_port>:<connect_address>:<connect_port> <remote_machine>
```
Examples:
```
smb2tcp-client.exe -L 127.0.0.1:8080:127.0.0.1:8080 3.3.3.3
smb2tcp-client.exe -R 127.0.0.1:8080:127.0.0.1:8080 3.3.3.3
smb2tcp-client.exe -L 127.0.0.1:3390:4.4.4.4:3389 3.3.3.3
```
1. Create an SMB tunnel to 3.3.3.3, listen on port 8080 on the client machine, and forward the traffic to port 8080 on the remote machine.
2. Create the listener on port 8080 on the remote machine and forward to port 8080 on the client machine.
3. Create an SMB tunnel to 3.3.3.3, listen on port 3390 and forward the traffic to the RDP port (3389) of 4.4.4.4.

Please note that the user running the client needs to have permissions for network logon to the remote machine (they don't need to be an admin). If you are using a local user, you can use runas /netonly to authenticate remotely:
```
runas /user:remote_machine_user /netonly cmd
cmd> smb2tcp-client.exe -L <listen_address>:<listen_port>:<connect_address>:<connect_port> <remote_machine>
```
## Current Limitations
- No application level encryption: The tunnel will be encrypted by SMB only if both sides will agree on encryption, the data in the tunnel will be encrypted only if you are using an encrypted protocol (HTTPS, SSH, RDP over TLS), otherwise there will be no encryption. I'm working on application level encryption using BCrypt.
- Default security descriptors: The tool consists of an RPC server over a named pipe as the control plane, and the named pipes tunnels. For both, I use the default security descriptors, which might be too permissive.
- Performance: Currently the performance is not optimal due to the use of synchronous IO.
