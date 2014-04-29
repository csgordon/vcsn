#! /bin/bash
# vcsn-bootstrap.sh --- Setup VCSN

export DEBIAN_FRONTEND=noninteractive
install="apt-get -y install"

echo ";;;; Configuring VCSN instance..."

echo ";; Updating system installation..."
apt-get update
apt-get upgrade

echo ";; Installing build requirements..."
$install git
$install make
$install autoconf
$install automake
$install libtool
$install flex
$install bison xsltproc
$install libboost-all-dev
$install ipython ipython-notebook python-matplotlib
$install graphviz

echo ";;; Building VCSN..."
(
    vcsndir=/opt/vaucanson
    if [ ! -d $vcsndir ]; then
	git clone https://gitlab.lrde.epita.fr/vcsn/vaucanson.git $vcsndir
    else
	cd $vcsndir
	git pull
    fi

    cd $vcsndir
    ./bootstrap
    ./configure
    make all
    # make check		      # Takes to long for my patience --- ams
)

cat <<EOF>/etc/rc.local
#!/bin/sh
if [ -e /opt/vaucanson/tests/bin/vcsn ]; then
    cd /vagrant
    echo ";;;; Starting VCSN interactive notebook..."
    /opt/vaucanson/tests/bin/vcsn notebook --no-browser --ip='*' &
fi
EOF

# Start VCSN Notebook; across reboots /etc/rc.local will handle it but
# while provisioning we have to do it explicitly.
/etc/rc.local

echo ";;;; Done configuring VCSN instance."

# vcsn-bootstrap.sh ends here.
