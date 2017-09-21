	i2c�豸����ʵ�ֲ��裺
	1. ʵ��IIC�豸���豸���ı�д
	2. ʵ��IIC�豸�����ı�д
		ע�⣺�豸���е�������Ҫ�������е�����ƥ�䡣
		
	�豸����ʵ�֣�
			   
		i2c@138B0000 {��// ��IIC��������������������Щ���ݱ�IIC������������ʹ�ã�Ҳ���ǳ��ҵ���������ʹ��
         samsung,i2c-sda-delay = <100>;
         samsung,i2c-max-bus-freq = <20000>;  // 20k
         pinctrl-0 = <&i2c5_bus>;
         pinctrl-names = "default";
         status = "okay";   // ״̬����
 
         mpu6050-3-axis@68 { // �ӽڵ㣬��ʾ�ҽ��������������ϣ���i2c_core����ʹ�ã�����struct i2c_client�ڵ�                                                                                      
             compatible ="invensense,mpu6050";
             reg = <0x68>;
             interrupt-parent = <&gpx3>;
             interrupts = <3 2>;
         };
     };
		
	�����������У���ƥ��ɹ����ں˾ͻ����������probe������
	int (*probe)(struct i2c_client *, const struct i2c_device_id *); // ̽�⺯��
	�ں����豸�ĵ�ַ��IIC�������ݴ���ĺ��������ں˴����struct i2c_client�ṹ���У����ǻ�øýṹ��󱣳ָýṹ��
	��ַ�������Լ�����һ���ַ��豸��ʵ���ַ��豸�Ĳ������ϣ��������������յ���struct i2c_client�ṩ�ĵ�ַ�Ͳ�������
	ʵ�����ݽ�����Ϊ��ʵ�����ݵ�ͳһ����Ҫ�����ǵ����ݴ����struct i2c_msg�ṹ����з��͡�
	
	�������漰�Ľṹ���У�
	/***************************************************************************************/
	/* i2c_driver �ṹ�� ��Ҫ������䲢ע�� */
	struct i2c_driver {
			unsigned int class;
			/* Standard driver model interfaces */
			int (*probe)(struct i2c_client *, const struct i2c_device_id *); // ̽�⺯��
			int (*remove)(struct i2c_client *);  // �Ƴ�����

			struct device_driver driver;  // �豸�����Ļ���
			const struct i2c_device_id *id_table;  // i2cƥ���

			/* Device detection callback for automatic device creation */
			int (*detect)(struct i2c_client *, struct i2c_board_info *);
			const unsigned short *address_list;
			struct list_head clients;  // �ں�ѭ��˫����
		};
	
		module_i2c_driver(xxx_drver);//ע����������ʵ����ںͳ��ں���
	/***************************************************************************************/
		/* i2c_client�ṹ�壬IIC�豸��ʵ�巴ӳ */
		struct i2c_client {
		unsigned short flags;		/* div., see below		*/
		unsigned short addr;		/* 7λ���ӻ���ַ��I2C�豸��ַ */
				
		char name[I2C_NAME_SIZE];  // �豸������
		struct i2c_adapter *adapter;	/* ������ */
		struct device dev;		/* �豸�Ļ���	*/
		int irq;			/*�жϺ�	*/
		struct list_head detected;  // �ں�ѭ��˫����
	};
	
	/***************************************************************************************/
	  
		/* IIC������,��IIC������������ʵ�巴ӳ */
	struct i2c_adapter {
		struct module *owner;
		unsigned int class;		  /* classes to allow probing for */
		const struct i2c_algorithm *algo; /* �������ߵ��㷨 */

		int timeout;			/*��ʱʱ��ֵ */
		int retries;          // �ش�
		struct device dev;		/* the adapter device */
        ...
	};	
	
	/***************************************************************************************/	
		
		/* IIC���������������շ��������� */	
		struct i2c_algorithm {

		// ���ݴ���
		int (*master_xfer)(struct i2c_adapter *adap, struct i2c_msg *msgs,
				   int num);

		/* To determine what the adapter supports */
		u32 (*functionality) (struct i2c_adapter *);
	};	
		
	/***************************************************************************************/
		
		/* �������ݴ������Ϣ�ṹ�� */	
		struct i2c_msg {
		__u16 addr;	/* �ӻ���ַ��оƬ��ַ			*/
		__u16 flags;  // ����1����д��0����־λ
	#define I2C_M_TEN		0x0010	/* this is a ten bit chip address */
	#define I2C_M_RD		0x0001	/* read data, from slave to master */
	#define I2C_M_STOP		0x8000	/* if I2C_FUNC_PROTOCOL_MANGLING */
	#define I2C_M_NOSTART		0x4000	/* if I2C_FUNC_NOSTART */
	#define I2C_M_REV_DIR_ADDR	0x2000	/* if I2C_FUNC_PROTOCOL_MANGLING */
	#define I2C_M_IGNORE_NAK	0x1000	/* if I2C_FUNC_PROTOCOL_MANGLING */
	#define I2C_M_NO_RD_ACK		0x0800	/* if I2C_FUNC_PROTOCOL_MANGLING */
	#define I2C_M_RECV_LEN		0x0400	/* length will be first received byte */
		__u16 len;		/* ��Ϣ�ĳ���				*/
		__u8 *buf;		/* ��Ϣ������			*/
	};	
	/***************************************************************************************/

	�������Ǿ�����˶������ı�д�����������е�о������������������ں˵�struct client��������ɵģ�
	�豸��ΪʲôҪ����д��

	����Ӳ����Ϣ�������豸���У����ǿ�֪i2c_client�ڵ�϶�ʵ��ƽ̨�豸���������ɵģ�ƥ�䵽�ýڵ�ľ�ֻ�г��ҵ�
	�����ˡ�����ȥ�鿴����������
	���ǵ�i2c���������� drivers\i2\busse\i2c-s3c240.c�У�
	
	/* ƥ�䵽�豸��������s3c24xx_i2c_probe����*/
	static int s3c24xx_i2c_probe(struct platform_device *pdev)
	===>
	i2c_add_numbered_adapter(&i2c->adap);// ���ú�struct adapter�ṹ�壬������ȥ
	===>
	__i2c_add_numbered_adapter(adap);
	===>
	i2c_register_adapter(adap);
	==>
	of_i2c_register_devices(adap);
	==>
	struct i2c_board_info info = {}; // ���岢����struct i2c_board_info�ṹ��
	of_modalias_node(node, info.type, sizeof(info.type); // �����豸��compatible���ֵ�i2c_board_info�ṹ����
	addr = of_get_property(node, "reg", &len); // ��ȡ�豸��ַ
	info.addr = be32_to_cpup(addr); // ת��ΪCPU�ֽ���
	info.irq = irq_of_parse_and_map(node, 0); //��ȡ�ж�
	info.of_node = of_node_get(node);
	info.archdata = &dev_ad;
	===>
	i2c_new_device(adap, &info); // ����i2c_client��ע�ᣬ���ǲ�����豸�Ƿ�����������ϣ�Ҳ�������ǽ���ע�ᡣ
	
	���Ƿ��֣���of_i2c_register_devices(adap);�����н�����IIC����������������ӽڵ㣬һ���������������ݣ�
		compatible������
		reg��				��ַ
		irq��				�ж�
	�����豸�������ǵ��ӽڵ����д��Ϊ��
	mpu6050-3-axis@68 {                                                                                     
             compatible ="invensense,mpu6050";
             reg = <0x68>;
             interrupt-parent = <&gpx3>;
             interrupts = <3 2>;
   };
   ������֮��Ӧ������
	/***************************************************************************************/
	
	����IIC�������ʵ��MPU6050������ʵ�֣�
	
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
			{.addr = client->addr,.flags = 0,.len = 1,.buf = txbuf}, // д������
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


	static const struct file_operations fops={ // �����ַ��豸��������
		.owner = THIS_MODULE,
		.open = mpu6050_open,
		.release = 	mpu6050_release,
		.unlocked_ioctl = mpu6050_ioctl,
	}; 

	static int mpu6050_probe(struct i2c_client *client, const struct i2c_device_id * id)
	{
		printk("mpu6050_probe.\n");
		/* ��i2c_bus��ƥ�亯����ƥ��ɹ����øú������ú�����Ҫʵ�ֶ�
		*	���ַ��豸��ע�ᣬ���豸�Ĵ�����
		*/
		int ret;
		ret = alloc_chrdev_region(&mpu6050.devno, BASEMINOR, COUNT, "mpu6050"); // ����һ���豸��
		if(ret){
			printk(KERN_ERR "alloc_chrdev_region error.\n");
			return ret;
		}
		mpu6050.cdev = cdev_alloc(); // �����ַ��豸
		if(NULL==mpu6050.cdev){
			printk(KERN_ERR "cdev_alloc error.");
			ret = -ERESTART;
			goto err0;
		}
		cdev_init(mpu6050.cdev,&fops); // ��ʼ���ַ��豸
		mpu6050.cdev->owner = THIS_MODULE;
		ret = cdev_add(mpu6050.cdev,mpu6050.devno,COUNT); // ע���ַ��豸
		if(ret){
			printk(KERN_ERR "cdev_add error.\n");
			goto err1;
		}
		mpu6050.pcls = class_create(THIS_MODULE,"mpu"); // ����һ����
		if(IS_ERR(mpu6050.pcls)){
			printk(KERN_ERR "class_create error.\n");
			ret = PTR_ERR(mpu6050.pcls);
			goto err2;
		}
		mpu6050.dev = device_create(mpu6050.pcls,NULL,mpu6050.devno,NULL,"mpu6050"); // �������洴��һ���豸
		if(IS_ERR(mpu6050.dev)){
			printk(KERN_ERR "device_create error.\n");
			ret = PTR_ERR(mpu6050.dev);
			goto err3;
		}
		// ��ʼ��mpu6050Ӳ����ʹ֮���ھ���״̬
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

	static const struct of_device_id of_mpu6050_table[]={ //����compatible�������豸��ƥ��
		{.compatible="invensense,mpu6050",}, 
		{/* Noting to be done */}, // ����ֹͣ��־

	};

	MODULE_DEVICE_TABLE(of, of_mpu6050_table);


	static const struct i2c_device_id mpu6050_table[]={ //����id_table����ʹ��ʹ��id_table
										//����ƥ�䣬Ҳ��Ҫ���ã��������
		{"mpu6050",0,},
		{/* Nothing to be done */},
		
	};


	static const struct i2c_driver mpu6050_drver={ // ����i2c_driver�ṹ��

		.driver={
			.name = "mpu6050", // ���������֣�Ҳ������ƥ�䣬�ͱ����ظ�����ע��
			.owner = THIS_MODULE,
			.of_match_table = of_mpu6050_table, // ��ֵ�豸��ƥ���
		},
		.probe = mpu6050_probe,	// 	ƥ��ɹ���������mpu6050_probe����
		.remove = mp6050_remove, // ��������ж�أ�������mp6050_remove����
		.id_table = &mpu6050_table, // �ں˵�����һ��ƥ�䷽ʽ��id_tableƥ���

	};

	module_i2c_driver(mpu6050_drver);//ע������

	MODULE_LICENSE("GPL");

 /***************************************************************************************/
	
	�豸�������ʵ�֣�
	
			i2c@138B0000 {��// ��IIC��������������������Щ���ݱ�IIC������������ʹ�ã�Ҳ���ǳ��ҵ���������ʹ��
         samsung,i2c-sda-delay = <100>;
         samsung,i2c-max-bus-freq = <20000>;  // 20k
         pinctrl-0 = <&i2c5_bus>;
         pinctrl-names = "default";
         status = "okay";   // ״̬���أ�ʹ��ʱһ��Ҫ��
 
         mpu6050-3-axis@68 { // �ӽڵ㣬��ʾ�ҽ��������������ϣ���i2c_core����ʹ�ã�����struct i2c_client�ڵ�                                                                                      
             compatible ="invensense,mpu6050";
             reg = <0x68>;
             interrupt-parent = <&gpx3>;
             interrupts = <3 2>;
         };
     };
	
	/***************************************************************************************/
	
	
	
	
	
	
	



