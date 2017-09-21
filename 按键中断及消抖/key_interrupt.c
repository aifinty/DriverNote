
	中断系统体系结构
	
	Linux内核发生中断处理流程：
		1.发生中断的时候，cpu会调到指定的异常向量表中。
		2.从异常向量表中，会调用中断处理的总入口函数 asm_do_IRQ
		3.根据获得中断号获取得到 irq_desc[]数组中获得一个 struct irq_desc 结构体项
		4.从 struct irq_desc 结构体项获取 handl_irq 函数，进行中断的一些操作，清除中断标志位等
		5.执行通过 request_irq() 函数注册的中断处理函数

	内核中管理中断的是一个 struct irq_desc 类型的结构体数组，它是一个全局的数组。中断号对应了数组的下标。 
	struct irq_desc 结构体包含了两个结构体（对象）：
	
		struct irq_desc{
				...
				struct irq_data {struct irq_chip		*chip;...} // struct irq_chip 硬件底层操作，一般厂家实现
				irq_flow_handler_t	handle_irq;	// 中断处理时候的硬件操作，一般厂家实现，并会调用通过
																				// request_irq() 函数注册的中断处理函数
				struct irqaction   // 用户通过 request_irq() 进行注册的结构体
				...
			}
		
		
		厂家对 struct irq_chip 和irq_flow_handler_t	handle_irq的注册：
	/************************************************************************************************/
		// 设置 struct irq_chip 结构体
		static struct irq_chip s3c_irq_eint0t4 = {
			.name		= "s3c-ext0",
			.irq_ack	= s3c_irq_ack,
			.irq_mask	= s3c_irq_mask,
			.irq_unmask	= s3c_irq_unmask,
			.irq_set_wake	= s3c_irq_wake,
			.irq_set_type	= s3c_irqext_type,
		};
		
		irq_set_chip_and_handler(irqno, &s3c_irqext_chip,handle_edge_irq); // 注册结构体和处理函数
		==>
		irq_set_chip_and_handler_name(irq, chip, handle, NULL);
		==>
		irq_set_chip(irq, chip); // 注册chip结构体
		{
			==>
			struct irq_desc *desc = irq_get_desc_lock(irq, &flags, 0);
			{
				==>
				__irq_get_desc_lock(irq, flags, false, check);
				==>
				irq_to_desc(irq);
				==>
				#define irq_to_desc(irq)	(&irq_desc[irq]) // 以中断号为下标取得irq_desc[] 数组中对应项
			}
			desc->irq_data.chip = chip; // 赋值 chip 
			
		}
		__irq_set_handler(irq, handle, 0, name);// 注册处理函数
		{
			==>
			struct irq_desc *desc = irq_get_desc_buslock(irq, &flags, 0);
			{
				... //与上面相同
			}
			desc->handle_irq = handle;
			
		}
		
		// 这样就完成了中断的底层硬件操作的注册
	/************************************************************************************************/	
	
	
	中断发生执行流程：
	
	/************************************************************************************************/
	asm_do_IRQ(unsigned int irq, struct pt_regs *regs) // 中断函数汇编到C语言的第一个函数
	==>
	handle_IRQ(irq, regs);
	==>
	generic_handle_irq(irq);
	==>
	struct irq_desc *desc = irq_to_desc(irq); // 根据中断号获取desc
	generic_handle_irq_desc(irq, desc);
	==>
	desc->handle_irq(irq, desc); // 调用desc中的handle_irq函数，该函数被厂家实现，并注册
	==>
	void handle_edge_irq(unsigned int irq, struct irq_desc *desc)
	==>
	handle_irq_event(desc);
	==>
	ret = handle_irq_event_percpu(desc, action); 
	==>
	res = action->handler(irq, action->dev_id);// 调用通过request_irq()注册的函数
	
 /************************************************************************************************/					
	
	驱动中注册中断流程：
	
	request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags,const char *name, void *dev) 
	// 驱动函数调用 request_irq 来注册中断
	==>
	request_threaded_irq(irq, handler, NULL, flags, name, dev);
	==>
	desc = irq_to_desc(irq); // 获取desc结构体
	action->handler = handler; //设置action结构体
	==>
	__setup_irq(irq, desc, action); // 注册action
	// 中断注册完毕
/************************************************************************************************/

	按键中断实现实例：
	









