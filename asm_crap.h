#define START_INTERRUPT() __asm__ __volatile__ \
	("pushfl\n	pushal\n	pushl %esi\n	pushl %ss
	pushl %ds\n	pushl %es\n	pushl %fs\n	pushl %gs");
#define END_INTERRUPT() __asm__ __volatile__ \
	("popl %gs\n	popl %fs\n	popl %es\n	popl %ds
	popl %ss\n	popl %esi\n	popal\n	popfl\n	movl %ebp,%esp
	popl %ebp\n 	sti\n	iret");

#define ACKNOWLEDGE_KEYBOARD_INTERRUPT() __asm__ __volatile__ \
	("inb $0x61, %al\n	orb $0x80, %al\n	outb %al, $0x61\n \
	andb $0x7f, %al\n	outb %al, $0x61\n	movb $0x20, %al\n \
	outb %al, $0x20");

#define LOCK_VARIABLE(x)  {  __dpmi_meminfo ___tmp_lock_info;\
	___tmp_lock_info.address = (long)&x;\
	___tmp_lock_info.size = sizeof(x);\
	___tmp_lock_info.handle = _my_ds();\
	if(__dpmi_lock_linear_region(&___tmp_lock_info))\
	  Exit(1, "Unable to lock required memory!!\n");  }


#define UNLOCK_VARIABLE(x)  {  __dpmi_meminfo ___tmp_unlock_info;\
	___tmp_unlock_info.address = (long)&x;\
	___tmp_unlock_info.size = sizeof(x);\
	___tmp_unlock_info.handle = _my_ds();\
	if(__dpmi_unlock_linear_region(&___tmp_unlock_info))\
	  Exit(1, "Unable to unlock required memory!!\n");  }

#define LOCK_ARRAY(x, s)  {  __dpmi_meminfo ___tmp_lock_info;\
	___tmp_lock_info.address = (long)x;\
	___tmp_lock_info.size = (sizeof(*x) * s);\
	___tmp_lock_info.handle = _my_ds();\
	if(__dpmi_lock_linear_region(&___tmp_lock_info))\
	  Exit(1, "Unable to lock required memory!!\n");  }

#define UNLOCK_ARRAY(x, s)  {  __dpmi_meminfo ___tmp_unlock_info;\
	___tmp_unlock_info.address = (long)x;\
	___tmp_unlock_info.size = (sizeof(*x) * s);\
	___tmp_unlock_info.handle = _my_ds();\
	if(__dpmi_unlock_linear_region(&___tmp_unlock_info))\
	  Exit(1, "Unable to unlock required memory!!\n");  }
