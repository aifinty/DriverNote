	�豸���Ƕ�ƽ̨�豸������һ��������ƽ̨�豸������Ҫ��Ϊ���ܹ�ʵ�����������ݵķ��롣��ΪӲ������Ϣ�Ƕ����ģ�
���Ƕ�Ӧ��Ӳ������ȷʵ����ġ�������ϵͳ�У������豸����ģ�͡������豸��һ���ṹ����������struct platform_device,
������һ���ṹ����������struct platform_driver�ṹ�塣
	platform_device �̳���struct device����ͬʱ�Լ��������� ��
	struct platform_device {
		const char	*name; // ƽ̨�豸������
		int		id;
		bool		id_auto;
		struct device	dev; //�̳�struct device����
		u32		num_resources;	//��Դ��Ŀ
		struct resource	*resource;  //��Դ��һ����������Ӳ����ַ��Ϣ��

		const struct platform_device_id	*id_entry;
		... 
};

	��ƽ̨�豸ƥ�亯���У�����豸��(name)���ֽ���ƥ�䡣��ʹû��ʹ������ƥ��Ҳ��Ҫ��name���и�ֵ���ṹ���а�����
struct device dev;����Ϊ�Ǽ̳��˸ö���

	struct device {
		struct device		*parent; //���ڵ�
		
		struct device_private	*p;

		struct kobject kobj; //kobject �����û��ռ���Ϣ��ʾ
		const char		*init_name; /* initial name of the device */
		const struct device_type *type;

		struct bus_type	*bus;		/* �������ߵ����� */
		struct device_driver *driver;	/* ƥ�䵽������ */
		void		*platform_data;	/* ƽ̨���� */
		struct dev_pm_info	power; //��Դ���
		struct dev_pm_domain	*pm_domain;

		struct device_node	*of_node; /* �豸���ڵ� */
		struct acpi_dev_node	acpi_node; /* ACPI�ڵ� */


		dev_t			devt;	/* dev_t, creates the sysfs "dev" */
		u32			id;	/* device ��� */

		struct list_head	devres_head; // ѭ��˫����

		struct klist_node	knode_class;
		struct class		*class;

		void	(*release)(struct device *dev); //release��������Ҫƽ̨deviceʵ��
		struct iommu_group	*iommu_group;

		bool			offline_disabled:1;
		bool			offline:1;
	};
	
	platform_deviceע����̣�
	int platform_device_register(struct platform_device *pdev)// �û����øú�����ע���豸 
		{
			device_initialize(&pdev->dev);
			arch_setup_pdev_archdata(pdev); // �պ���
			return platform_device_add(pdev);
		}
		====> // �ں�Ĭ�ϳ�ʼ��һЩ�ṹ���Ա
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
	========> // ������Դ��������Դ�ṹ�嵽��Ӧ����Դ����ע��struct device �ṹ��
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
				r->name = dev_name(&pdev->dev); // ������Դ

			p = r->parent;
			if (!p) { // ������Դ���ͣ�ѡ���׽ڵ�
				if (resource_type(r) == IORESOURCE_MEM)
					p = &iomem_resource;
				else if (resource_type(r) == IORESOURCE_IO)
					p = &ioport_resource;
			}

			if (p && insert_resource(p, r)) { // ������Դ
				dev_err(&pdev->dev, "failed to claim resource %d\n", i);
				ret = -EBUSY;
				goto failed;
			}
		}

		pr_debug("Registering platform device '%s'. Parent at %s\n",
			 dev_name(&pdev->dev), dev_name(pdev->dev.parent));

		ret = device_add(&pdev->dev); // ����struct device �ṹ��
		if (ret == 0)
			return ret;
			...
	}
	========> //ע��struct device �ṹ��
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

	








