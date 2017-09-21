	i2c设备驱动实现步骤：
	1. 实现IIC设备的设备树的编写
	2. 实现IIC设备驱动的编写
		注意：设备树中的名字需要和驱动中的名字匹配。
		
	设备树的实现：
			   
		i2c@138B0000 {　// 对IIC主机控制器的描述，这些数据被IIC总线驱动程序使用，也就是厂家的驱动程序使用
         samsung,i2c-sda-delay = <100>;
         samsung,i2c-max-bus-freq = <20000>;  // 20k
         pinctrl-0 = <&i2c5_bus>;
         pinctrl-names = "default";
         status = "okay";   // 状态开关
 
         mpu6050-3-axis@68 { // 子节点，表示挂接在主机控制器上，被i2c_core核心使用，生成struct i2c_client节点                                                                                      
             compatible ="invensense,mpu6050";
             reg = <0x68>;
             interrupt-parent = <&gpx3>;
             interrupts = <3 2>;
         };
     };
		
	在驱动函数中，当匹配成功，内核就会调用驱动的probe函数：
	int (*probe)(struct i2c_client *, const struct i2c_device_id *); // 探测函数
	内核中设备的地址，IIC总线数据传输的函数都被内核打包到struct i2c_client结构体中，我们获得该结构体后保持该结构体
	地址，我们自己生成一个字符设备并实现字符设备的操作集合，操作集合中最终调用struct i2c_client提供的地址和操作函数
	实现数据交互。为了实现数据的统一，需要把我们的数据打包到struct i2c_msg结构体进行发送。
	
	驱动中涉及的结构体有：
	/***************************************************************************************/
	/* i2c_driver 结构体 需要我们填充并注册 */
	struct i2c_driver {
			unsigned int class;
			/* Standard driver model interfaces */
			int (*probe)(struct i2c_client *, const struct i2c_device_id *); // 探测函数
			int (*remove)(struct i2c_client *);  // 移除函数

			struct device_driver driver;  // 设备驱动的基类
			const struct i2c_device_id *id_table;  // i2c匹配表

			/* Device detection callback for automatic device creation */
			int (*detect)(struct i2c_client *, struct i2c_board_info *);
			const unsigned short *address_list;
			struct list_head clients;  // 内核循环双链表
		};
	
		module_i2c_driver(xxx_drver);//注册驱动，并实现入口和出口函数
	/***************************************************************************************/
		/* i2c_client结构体，IIC设备的实体反映 */
		struct i2c_client {
		unsigned short flags;		/* div., see below		*/
		unsigned short addr;		/* 7位，从机地址，I2C设备地址 */
				
		char name[I2C_NAME_SIZE];  // 设备的名称
		struct i2c_adapter *adapter;	/* 适配器 */
		struct device dev;		/* 设备的基类	*/
		int irq;			/*中断号	*/
		struct list_head detected;  // 内核循环双链表
	};
	
	/***************************************************************************************/
	  
		/* IIC适配器,对IIC主机控制器的实体反映 */
	struct i2c_adapter {
		struct module *owner;
		unsigned int class;		  /* classes to allow probing for */
		const struct i2c_algorithm *algo; /* 访问总线的算法 */

		int timeout;			/*超时时间值 */
		int retries;          // 重传
		struct device dev;		/* the adapter device */
        ...
	};	
	
	/***************************************************************************************/	
		
		/* IIC主机控制器数据收发操作集合 */	
		struct i2c_algorithm {

		// 数据传输
		int (*master_xfer)(struct i2c_adapter *adap, struct i2c_msg *msgs,
				   int num);

		/* To determine what the adapter supports */
		u32 (*functionality) (struct i2c_adapter *);
	};	
		
	/***************************************************************************************/
		
		/* 用于数据打包的消息结构体 */	
		struct i2c_msg {
		__u16 addr;	/* 从机地址，芯片地址			*/
		__u16 flags;  // 读（1），写（0）标志位
	#define I2C_M_TEN		0x0010	/* this is a ten bit chip address */
	#define I2C_M_RD		0x0001	/* read data, from slave to master */
	#define I2C_M_STOP		0x8000	/* if I2C_FUNC_PROTOCOL_MANGLING */
	#define I2C_M_NOSTART		0x4000	/* if I2C_FUNC_NOSTART */
	#define I2C_M_REV_DIR_ADDR	0x2000	/* if I2C_FUNC_PROTOCOL_MANGLING */
	#define I2C_M_IGNORE_NAK	0x1000	/* if I2C_FUNC_PROTOCOL_MANGLING */
	#define I2C_M_NO_RD_ACK		0x0800	/* if I2C_FUNC_PROTOCOL_MANGLING */
	#define I2C_M_RECV_LEN		0x0400	/* length will be first received byte */
		__u16 len;		/* 消息的长度				*/
		__u8 *buf;		/* 消息的正文			*/
	};	
	/***************************************************************************************/

	到此我们就完成了对驱动的编写，但是总是有点感觉不安，不安来自于内核的struct client是如何生成的？
	设备树为什么要这样写？

	由于硬件信息描述在设备树中，我们可知i2c_client节点肯定实在平台设备驱动中生成的，匹配到该节点的就只有厂家的
	驱动了。我们去查看厂家驱动：
	三星的i2c驱动函数在 drivers\i2\busse\i2c-s3c240.c中：
	
	/* 匹配到设备树，调用s3c24xx_i2c_probe函数*/
	static int s3c24xx_i2c_probe(struct platform_device *pdev)
	===>
	i2c_add_numbered_adapter(&i2c->adap);// 设置好struct adapter结构体，传递下去
	===>
	__i2c_add_numbered_adapter(adap);
	===>
	i2c_register_adapter(adap);
	==>
	of_i2c_register_devices(adap);
	==>
	struct i2c_board_info info = {}; // 定义并设置struct i2c_board_info结构体
	of_modalias_node(node, info.type, sizeof(info.type); // 拷贝设备树compatible名字到i2c_board_info结构体中
	addr = of_get_property(node, "reg", &len); // 获取设备地址
	info.addr = be32_to_cpup(addr); // 转换为CPU字节序
	info.irq = irq_of_parse_and_map(node, 0); //获取中断
	info.of_node = of_node_get(node);
	info.archdata = &dev_ad;
	===>
	i2c_new_device(adap, &info); // 生成i2c_client并注册，但是不检查设备是否存在于总线上，也就是总是进行注册。
	
	我们发现，在of_i2c_register_devices(adap);函数中解析了IIC主机控制器里面的子节点，一共解析了三个数据：
		compatible：名字
		reg：				地址
		irq：				中断
	所以设备树中我们的子节点的书写则为：
	mpu6050-3-axis@68 {                                                                                     
             compatible ="invensense,mpu6050";
             reg = <0x68>;
             interrupt-parent = <&gpx3>;
             interrupts = <3 2>;
   };
   正好与之对应起来。
	/***************************************************************************************/
	
	基于IIC驱动框架实例MPU6050的驱动实现：
	
	#include <linux/init.h>
	#include <linux/module.h>
	#include <linux/kernel.h>
	#include <linux/cdev.h>
	#include <linux/fs.h>
	#include <linux/slab.h>
	#include <linux/sched.h>

	#include "mpu6050_ioctl.h"
	#include "mpu6050.h"


	#define BASEMINOR	0
	#define COUNT	1

	struct mpu6050{
		struct cdev *cdev;
		dev_t devno;
		struct class *pcls;
		struct device *dev;
		struct i2c_client *client;
	};

	static struct mpu6050 mpu6050;

	static int mpu6050_read_byte(unsigned char reg,unsigned char *dat)
	{
		int ret;
		struct i2c_client *client = mpu6050.client;

		unsigned char txbuf[1] = {reg};
		unsigned char rxbuf[1] = {0};

		if(!dat){
			printk("null pointer.\n");
			return -1;
		}
		
		struct i2c_msg msgs[]={
			{.addr = client->addr,.flags = 0,.len = 1,.buf = txbuf}, // 写入数据
			{.addr = client->addr,.flags = I2C_M_RD,.len = 1,.buf = rxbuf},
		};

		ret = i2c_transfer(client->adapter,msgs,ARRAY_SIZE(msgs));
		if(ret<0){
			printk("i2c_transfer error.\n");
			return ret;
		}
		*dat = rxbuf;
		return 0;
	}

	static int mpu6050_write_byte(char reg,char dat)
	{
		struct i2c_client *client = mpu6050.client;
	}

	static int mpu6050_open(struct inode *inode, struct file *filep)
	{
		printk(KERN_INFO "mpu6050_open.\n");
		return 0;
	}

	static int mpu6050_release(struct inode *inode, struct file *filep)
	{
		printk(KERN_INFO "mpu6050_open.\n");
		return 0;
	}

	static long  mpu6050_ioctl(struct file*filep, unsigned int cmd, unsigned long arg)
	{
		return 0;
	}


	static const struct file_operations fops={ // 设置字符设备操作集合
		.owner = THIS_MODULE,
		.open = mpu6050_open,
		.release = 	mpu6050_release,
		.unlocked_ioctl = mpu6050_ioctl,
	}; 

	static int mpu6050_probe(struct i2c_client *client, const struct i2c_device_id * id)
	{
		printk("mpu6050_probe.\n");
		/* 在i2c_bus的匹配函数，匹配成功调用该函数，该函数需要实现对
		*	对字符设备的注册，和设备的创建。
		*/
		int ret;
		ret = alloc_chrdev_region(&mpu6050.devno, BASEMINOR, COUNT, "mpu6050"); // 分配一个设备号
		if(ret){
			printk(KERN_ERR "alloc_chrdev_region error.\n");
			return ret;
		}
		mpu6050.cdev = cdev_alloc(); // 分配字符设备
		if(NULL==mpu6050.cdev){
			printk(KERN_ERR "cdev_alloc error.");
			ret = -ERESTART;
			goto err0;
		}
		cdev_init(mpu6050.cdev,&fops); // 初始化字符设备
		mpu6050.cdev->owner = THIS_MODULE;
		ret = cdev_add(mpu6050.cdev,mpu6050.devno,COUNT); // 注册字符设备
		if(ret){
			printk(KERN_ERR "cdev_add error.\n");
			goto err1;
		}
		mpu6050.pcls = class_create(THIS_MODULE,"mpu"); // 创建一个类
		if(IS_ERR(mpu6050.pcls)){
			printk(KERN_ERR "class_create error.\n");
			ret = PTR_ERR(mpu6050.pcls);
			goto err2;
		}
		mpu6050.dev = device_create(mpu6050.pcls,NULL,mpu6050.devno,NULL,"mpu6050"); // 在类下面创建一个设备
		if(IS_ERR(mpu6050.dev)){
			printk(KERN_ERR "device_create error.\n");
			ret = PTR_ERR(mpu6050.dev);
			goto err3;
		}
		// 初始化mpu6050硬件，使之处于就绪状态
		mpu6050.client = client;
		
		return 0;
	err3:
		class_destroy(mpu6050.pcls);
	err2:
		cdev_del(mpu6050.cdev);
	err1:
		kfree(mpu6050.cdev);
	err0:
		unregister_chrdev_region(mpu6050.devno,COUNT);
		return ret;
	}

	static int mp6050_remove(struct i2c_client *client)
	{
		printk("mp6050_remove.\n");
		device_destroy(mpu6050.pcls,mpu6050.devno);
		class_destroy(mpu6050.pcls);
		cdev_del(mpu6050.cdev);
		kfree(mpu6050.cdev);
		unregister_chrdev_region(mpu6050.devno,COUNT);
		return 0;
	}

	static const struct of_device_id of_mpu6050_table[]={ //设置compatible，进行设备树匹配
		{.compatible="invensense,mpu6050",}, 
		{/* Noting to be done */}, // 设置停止标志

	};

	MODULE_DEVICE_TABLE(of, of_mpu6050_table);


	static const struct i2c_device_id mpu6050_table[]={ //设置id_table表，即使不使用id_table
										//进行匹配，也需要设置，否则出错
		{"mpu6050",0,},
		{/* Nothing to be done */},
		
	};


	static const struct i2c_driver mpu6050_drver={ // 设置i2c_driver结构体

		.driver={
			.name = "mpu6050", // 驱动的名字，也可用于匹配，和避免重复发生注册
			.owner = THIS_MODULE,
			.of_match_table = of_mpu6050_table, // 赋值设备树匹配表
		},
		.probe = mpu6050_probe,	// 	匹配成功，将调用mpu6050_probe函数
		.remove = mp6050_remove, // 驱动发生卸载，将调用mp6050_remove函数
		.id_table = &mpu6050_table, // 内核的另外一种匹配方式，id_table匹配表

	};

	module_i2c_driver(mpu6050_drver);//注册驱动

	MODULE_LICENSE("GPL");

 /***************************************************************************************/
	
	设备树代码的实现：
	
			i2c@138B0000 {　// 对IIC主机控制器的描述，这些数据被IIC总线驱动程序使用，也就是厂家的驱动程序使用
         samsung,i2c-sda-delay = <100>;
         samsung,i2c-max-bus-freq = <20000>;  // 20k
         pinctrl-0 = <&i2c5_bus>;
         pinctrl-names = "default";
         status = "okay";   // 状态开关，使用时一定要打开
 
         mpu6050-3-axis@68 { // 子节点，表示挂接在主机控制器上，被i2c_core核心使用，生成struct i2c_client节点                                                                                      
             compatible ="invensense,mpu6050";
             reg = <0x68>;
             interrupt-parent = <&gpx3>;
             interrupts = <3 2>;
         };
     };
	
	/***************************************************************************************/
	
	
	
	
	
	
	



