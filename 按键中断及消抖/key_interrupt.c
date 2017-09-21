
	�ж�ϵͳ��ϵ�ṹ
	
	Linux�ں˷����жϴ������̣�
		1.�����жϵ�ʱ��cpu�����ָ�����쳣�������С�
		2.���쳣�������У�������жϴ��������ں��� asm_do_IRQ
		3.���ݻ���жϺŻ�ȡ�õ� irq_desc[]�����л��һ�� struct irq_desc �ṹ����
		4.�� struct irq_desc �ṹ�����ȡ handl_irq �����������жϵ�һЩ����������жϱ�־λ��
		5.ִ��ͨ�� request_irq() ����ע����жϴ�����

	�ں��й����жϵ���һ�� struct irq_desc ���͵Ľṹ�����飬����һ��ȫ�ֵ����顣�жϺŶ�Ӧ��������±ꡣ 
	struct irq_desc �ṹ������������ṹ�壨���󣩣�
	
		struct irq_desc{
				...
				struct irq_data {struct irq_chip		*chip;...} // struct irq_chip Ӳ���ײ������һ�㳧��ʵ��
				irq_flow_handler_t	handle_irq;	// �жϴ���ʱ���Ӳ��������һ�㳧��ʵ�֣��������ͨ��
																				// request_irq() ����ע����жϴ�����
				struct irqaction   // �û�ͨ�� request_irq() ����ע��Ľṹ��
				...
			}
		
		
		���Ҷ� struct irq_chip ��irq_flow_handler_t	handle_irq��ע�᣺
	/************************************************************************************************/
		// ���� struct irq_chip �ṹ��
		static struct irq_chip s3c_irq_eint0t4 = {
			.name		= "s3c-ext0",
			.irq_ack	= s3c_irq_ack,
			.irq_mask	= s3c_irq_mask,
			.irq_unmask	= s3c_irq_unmask,
			.irq_set_wake	= s3c_irq_wake,
			.irq_set_type	= s3c_irqext_type,
		};
		
		irq_set_chip_and_handler(irqno, &s3c_irqext_chip,handle_edge_irq); // ע��ṹ��ʹ�����
		==>
		irq_set_chip_and_handler_name(irq, chip, handle, NULL);
		==>
		irq_set_chip(irq, chip); // ע��chip�ṹ��
		{
			==>
			struct irq_desc *desc = irq_get_desc_lock(irq, &flags, 0);
			{
				==>
				__irq_get_desc_lock(irq, flags, false, check);
				==>
				irq_to_desc(irq);
				==>
				#define irq_to_desc(irq)	(&irq_desc[irq]) // ���жϺ�Ϊ�±�ȡ��irq_desc[] �����ж�Ӧ��
			}
			desc->irq_data.chip = chip; // ��ֵ chip 
			
		}
		__irq_set_handler(irq, handle, 0, name);// ע�ᴦ����
		{
			==>
			struct irq_desc *desc = irq_get_desc_buslock(irq, &flags, 0);
			{
				... //��������ͬ
			}
			desc->handle_irq = handle;
			
		}
		
		// ������������жϵĵײ�Ӳ��������ע��
	/************************************************************************************************/	
	
	
	�жϷ���ִ�����̣�
	
	/************************************************************************************************/
	asm_do_IRQ(unsigned int irq, struct pt_regs *regs) // �жϺ�����ൽC���Եĵ�һ������
	==>
	handle_IRQ(irq, regs);
	==>
	generic_handle_irq(irq);
	==>
	struct irq_desc *desc = irq_to_desc(irq); // �����жϺŻ�ȡdesc
	generic_handle_irq_desc(irq, desc);
	==>
	desc->handle_irq(irq, desc); // ����desc�е�handle_irq�������ú���������ʵ�֣���ע��
	==>
	void handle_edge_irq(unsigned int irq, struct irq_desc *desc)
	==>
	handle_irq_event(desc);
	==>
	ret = handle_irq_event_percpu(desc, action); 
	==>
	res = action->handler(irq, action->dev_id);// ����ͨ��request_irq()ע��ĺ���
	
 /************************************************************************************************/					
	
	������ע���ж����̣�
	
	request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags,const char *name, void *dev) 
	// ������������ request_irq ��ע���ж�
	==>
	request_threaded_irq(irq, handler, NULL, flags, name, dev);
	==>
	desc = irq_to_desc(irq); // ��ȡdesc�ṹ��
	action->handler = handler; //����action�ṹ��
	==>
	__setup_irq(irq, desc, action); // ע��action
	// �ж�ע�����
/************************************************************************************************/

	�����ж�ʵ��ʵ����
	









