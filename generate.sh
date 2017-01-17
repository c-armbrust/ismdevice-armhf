# This script will issue the make command and then copy the created binary to the bbb.
# It will use the host bbb for copying, so make sure you edited the host file, e.g.:
#Host bbb
#        HostName 192.168.178.46 # Or whatever your IP is
#        User debian # Standard user on beaglebone black
#        IdentityFile /root/.ssh/id_ed25519 # Your ssh key to authenticate if you placed the public part in ~/.ssh/authorized_keys
echo "Building project..."
make

if [ $? -ne 0 ]; then
  echo -e "\nAn error occured during compilation." 1>&2
  exit 1
fi
chown 1000:1000 statetest
echo "Copying executable to BBB..."
scp statetest bbb:~
if [ $? -ne 0 ]; then
  echo -e "\nAn error occured during copying." 1>&2
  exit 1
fi
echo "Done!"
