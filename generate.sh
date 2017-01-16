# This script will issue the make command and then copy the created binary to the bbb.
# It will use the host bbb for copying, so make sure you edited the host file, e.g.:
#Host bbb
#        HostName 192.168.178.46 # Or whatever your IP is
#        User debian # Standard user on beaglebone black
#        IdentityFile /root/.ssh/id_ed25519 # Your ssh key to authenticate if you placed the public part in ~/.ssh/authorized_keys
echo "Building project..."
make
echo "Copying executable to BBB..."
scp statetest bbb:~
echo "Done!"
