
	根文件系统制作与挂载
	
	1. busybox编译，和运行需要的库拷贝
	2. 启动脚本的制作，和设备节点的创建
	
	/**********************************************************************************************/
		1.1 设置 /etc/inittab 文件，它是系统挂载文件系统执行的第一个脚本，busybox在没有这个脚本情况下执行默认的脚本程序
		
		每个条目的格式:<id>:<runlevels>:<action>:<process>
			
			<id>: 没有什么实际意义。一般用于指定特定的程序运行的终端
			<runlevels>: 完全被忽略
			<action>: 有效的取值有：sysinit, respawn, askfirst, wait, once, restart, ctrlaltdel, and shutdown.	
			<process>: 需要执行的命令
			
			inittab 具体内容：
			::sysinit:/etc/init.d/rcS  
			::respawn:-/bin/sh
			tty2::askfirst:-/bin/sh		
			::ctrlaltdel:/bin/umount -a -r	
			
			// 启动命令解释
			::sysinit:/etc/init.d/rcS	 // 执行 /etc/init.d/rcS 脚本
			tty2::askfirst:-/bin/sh 	//启动tty2作为终端， “-” 的意思使用登录的shell
			// 在重启之前该做的事
			::ctrlaltdel:/sbin/reboot
			::shutdown:/bin/umount -a -r
			::shutdown:/sbin/swapoff -a
				
	/**********************************************************************************************/			
		
		对于rcS脚本内容：
	
			#! /bin/sh
				/bin/mount -a
				
				// 实现设备的完全挂载
			  // you need to have /sys mounted before executing mdev
			 echo /sbin/mdev > /proc/sys/kernel/hotplug  // instruct the kernel to execute /sbin/mdev whenever a 
			 																						// device is added or removed
			 mdev -s  // seed /dev with all the device nodes that were created
		   // 添加部分
			 mount -t tmpfs -o size=64k,mode=0755 tmpfs /dev
			 mkdir /dev/pts
			 mount -t devpts devpts /dev/pts
		
		该脚本就只执行第一个命令 mount -a 该命令会去执行 /etc/fstab文件内容，主要是实现设备文件的挂载
		
			#device mount-point type options dump fsck order
			proc /proc proc defaults 	0 0 // 需要挂载
			tmpfs /tmp tmpfs defaults 0 0 // 需要内核对tmpfs的支持
			sysfs /sys sysfs defaults 0 0 // 必须挂载
			tmpfs /dev tmpfs defaults 0 0 // 需要内核对tmpfs的支持
		
		// 执行完毕这些文件系统挂载，会继续回到rcS脚本继续执行命令。
		
	/**********************************************************************************************/
		最后启动bash，会执行 /etc/profile 脚本主要用于设置一些环境变量。
		#!/bin/sh
			export HOSTNAME=JZ2440
			export USER=root
			export HOME=root
			export PS1="[$USER@$HOSTNAME \W]\# "
			PATH=/bin:/sbin:/usr/bin:/usr/sbin
			LD_LIBRARY_PATH=/lib:/usr/lib:$LD_LIBRARY_PATH
			export PATH LD_LIBRARY_PATH
		
	
	
	




