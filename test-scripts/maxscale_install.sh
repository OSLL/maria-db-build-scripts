#!/bin/bash

# $1 - image
# $2 - IP
# $3 - target

set -x

image=$1
IP=$2
target=$3

image_type=`cat /home/ec2-user/kvm/images/image_type | grep "$image".img | sed "s/$image.img//" | sed "s/ //g"`
echo "image type is $image_type"

if [ "$image_type" != "RPM" ] && [ "$image_type" != "DEB" ] ; then
        echo "unknown image type: should be RPM or DEB"
        exit 1
else
	if [ "$image_type" == "RPM" ] ; then
		cat /home/ec2-user/test-scripts/maxscale.repo.template | sed "s/###target###/$target/" | sed "s/###image###/$image/" >  /home/ec2-user/test-scripts/maxscale.repo
		scp -i /home/ec2-user/KEYS/$image -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null /home/ec2-user/test-scripts/maxscale.repo root@$IP:/etc/yum.repos.d/
		scp -i /home/ec2-user/KEYS/$image -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no /home/ec2-user/yum_files/$image/* root@$IP:/etc/yum.repos.d/

		ssh -i /home/ec2-user/KEYS/$image -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null root@$IP "yum clean all; yum -y install maxscale"
	else
		cat /home/ec2-user/test-scripts/apt_maxscale/$image/maxscale.list | sed "s/###target###/$target/" >  /home/ec2-user/test-scripts/maxscale.list
                scp -i /home/ec2-user/KEYS/$image -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null /home/ec2-user/test-scripts/maxscale.list root@$IP:/etc/apt/sources.list.d/
                scp -i /home/ec2-user/KEYS/$image -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no /home/ec2-user/apt_files/$image/* root@$IP:/etc/apt/sources.list.d/
		ssh -i /home/ec2-user/KEYS/$image -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null root@$IP "apt-get update; apt-get install -y --force-yes maxscale"
	fi
fi