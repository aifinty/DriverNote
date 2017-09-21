	设备树是对平台设备驱动的一种升级，平台设备驱动主要是为了能够实现驱动和数据的分离。因为硬件的信息是多样的，
但是对应的硬件驱动确实不变的。在早期系统中，采用设备驱动模型。其中设备用一个结构体来描述：struct platform_device,
驱动用一个结构体来描述：struct platform_driver结构体。
	platform_device 继承了struct device对象，同时自己还包含了 ：
	struct platform_device {
		const char	*name; // 平台设备的名字
		int		id;
		bool		id_auto;
		struct device	dev; //继承struct device对象
		u32		num_resources;	//资源数目
		struct resource	*resource;  //资源（一般用于描述硬件地址信息）

		const struct platform_device_id	*id_entry;
		... 
};

	在平台设备匹配函数中，会对设备的(name)名字进行匹配。即使没有使用名字匹配也需要给name进行赋值。结构体中包含了
struct device dev;则被认为是继承了该对象。

	struct device {
		struct device		*parent; //父节点
		
		struct device_private	*p;

		struct kobject kobj; //kobject 用于用户空间信息显示
		const char		*init_name; /* initial name of the device */
		const struct device_type *type;

		struct bus_type	*bus;		/* 所在总线的类型 */
		struct device_driver *driver;	/* 匹配到的驱动 */
		void		*platform_data;	/* 平台数据 */
		struct dev_pm_info	power; //电源相关
		struct dev_pm_domain	*pm_domain;

		struct device_node	*of_node; /* 设备树节点 */
		struct acpi_dev_node	acpi_node; /* ACPI节点 */


		dev_t			devt;	/* dev_t, creates the sysfs "dev" */
		u32			id;	/* device 句柄 */

		struct list_head	devres_head; // 循环双链表

		struct klist_node	knode_class;
		struct class		*class;

		void	(*release)(struct device *dev); //release函数，需要平台device实现
		struct iommu_group	*iommu_group;

		bool			offline_disabled:1;
		bool			offline:1;
	};
	
	platform_device注册过程：
	int platform_device_register(struct platform_device *pdev)// 用户调用该函数，注册设备 
		{
			device_initialize(&pdev->dev);
			arch_setup_pdev_archdata(pdev); // 空函数
			return platform_device_add(pdev);
		}
		====> // 内核默认初始化一些结构体成员
	void device_initialize(struct device *dev)
		{
			dev->kobj.kset = devices_kset;
			kobject_init(&dev->kobj, &device_ktype);
			INIT_LIST_HEAD(&dev->dma_pools);
			mutex_init(&dev->mutex);
			lockdep_set_novalidate_class(&dev->mutex);
			spin_lock_init(&dev->devres_lock);
			INIT_LIST_HEAD(&dev->devres_head);
			device_pm_init(dev);
			set_dev_node(dev, -1);
		}
	========> // 设置资源，插入资源结构体到对应的资源链表，注册struct device 结构体
	int platform_device_add(struct platform_device *pdev)
	{
		int i, ret;

		if (!pdev->dev.parent)
			pdev->dev.parent = &platform_bus;

		pdev->dev.bus = &platform_bus_type;

		switch (pdev->id) {
		default:
			dev_set_name(&pdev->dev, "%s.%d", pdev->name,  pdev->id);
			break;
		case PLATFORM_DEVID_NONE:
			dev_set_name(&pdev->dev, "%s", pdev->name);
			break;
		case PLATFORM_DEVID_AUTO:
			/*
			 * Automatically allocated device ID. We mark it as such so
			 * that we remember it must be freed, and we append a suffix
			 * to avoid namespace collision with explicit IDs.
			 */
			ret = ida_simple_get(&platform_devid_ida, 0, 0, GFP_KERNEL);
			if (ret < 0)
				goto err_out;
			pdev->id = ret;
			pdev->id_auto = true;
			dev_set_name(&pdev->dev, "%s.%d.auto", pdev->name, pdev->id);
			break;
		}

		for (i = 0; i < pdev->num_resources; i++) {
			struct resource *p, *r = &pdev->resource[i];

			if (r->name == NULL)
				r->name = dev_name(&pdev->dev); // 设置资源

			p = r->parent;
			if (!p) { // 根据资源类型，选择父亲节点
				if (resource_type(r) == IORESOURCE_MEM)
					p = &iomem_resource;
				else if (resource_type(r) == IORESOURCE_IO)
					p = &ioport_resource;
			}

			if (p && insert_resource(p, r)) { // 插入资源
				dev_err(&pdev->dev, "failed to claim resource %d\n", i);
				ret = -EBUSY;
				goto failed;
			}
		}

		pr_debug("Registering platform device '%s'. Parent at %s\n",
			 dev_name(&pdev->dev), dev_name(pdev->dev.parent));

		ret = device_add(&pdev->dev); // 插入struct device 结构体
		if (ret == 0)
			return ret;
			...
	}
	========> //注册struct device 结构体
	int device_add(struct device *dev)
	{
		struct device *parent = NULL;
		struct kobject *kobj;
		struct class_interface *class_intf;
		int error = -EINVAL;

		dev = get_device(dev);

		error = device_private_init(dev);

		dev_set_name(dev, "%s", dev->init_name);
		dev->init_name = NULL;

	dev_set_name(dev, "%s%u", dev->bus->dev_name, dev->id);

		parent = get_device(dev->parent);
		kobj = get_device_parent(dev, parent);
		dev->kobj.parent = kobj;

		set_dev_node(dev, dev_to_node(parent));

		error = kobject_add(&dev->kobj, dev->kobj.parent, NULL);
		platform_notify(dev);

		error = device_create_file(dev, &dev_attr_uevent);


	  error = device_create_file(dev, &dev_attr_dev);

		error = device_create_sys_dev_entry(dev);


		devtmpfs_create_node(dev);
		
		error = device_add_class_symlinks(dev);

		error = device_add_attrs(dev);

		error = bus_add_device(dev);

		error = dpm_sysfs_add(dev);

		device_pm_add(dev);


		blocking_notifier_call_chain(&dev->bus->p->bus_notifier,
						     BUS_NOTIFY_ADD_DEVICE, dev);

		kobject_uevent(&dev->kobj, KOBJ_ADD);
		bus_probe_device(dev);
		if (parent)
			klist_add_tail(&dev->p->knode_parent,
				       &parent->p->klist_children);

		if (dev->class) {
			/* tie the class to the device */
			klist_add_tail(&dev->knode_class,
				       &dev->class->p->klist_devices);

			/* notify any interfaces that the device is here */
			list_for_each_entry(class_intf,
					    &dev->class->p->interfaces, node)
				if (class_intf->add_dev)
					class_intf->add_dev(dev, class_intf);
		}
		...
	}

	








