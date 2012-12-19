#include <linux/module.h>  
#include <linux/kernel.h>  
#include <linux/init.h>  
#include <linux/sched.h> 
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/quicklist.h>
#include <asm/page.h>
#include <asm/unistd.h>
#include <asm/cacheflush.h>
#include <asm/io.h>
#include <asm/desc.h>
#include <asm/tlbflush.h>
#include <asm/cacheflush.h>
#include <asm/pgtable_types.h>
#include <asm/e820.h>

#define MYMODNAME "Capture-sys_open"
/*Environment need to be set before make*/
#define PID 
#define ADDR 0x
#define INIT_MM 0x
#define SYS_CALL_TABLE 0x


static int my_init_module(void);
static void my_cleanup_module(void);
#define PADDRPROTECT 0x8000000000000000

void **sys_call_table;
asmlinkage long (*do_open) (const char*, int flags, mode_t);
struct task_struct *p;
char *array;
pte_t *last_pte;


/*
 * Three things need to be impemented in Func()
 * 1. Call do_open() and store the return value. And return this value in the end of Func()
 *
 * 2. Copy message "Process [process name] invoke open system call" into char *array which we mapped from user space.
 *
 * 3. Send signal “SIGUNUSED” to user space process
 * */
long Func(const char __user *filename, int flags, mode_t mode){
    long ret;
    char *str;
    char *process_name;
    
    ret = do_open(filename, flags, mode);
    
    /*Copy message into global variable char *array
     * you will use current->mm->arg_start to get the address of process name
     * (current is the task_struct pointer which represent current running process)
     * */
    /*Need to implement*/
    /*...................*/
    /*...................*/
    /*End of implement*/

    send_sig( SIGUNUSED, p, 0);
    return ret; 
}


static unsigned long virtaddr_to_physaddr(struct mm_struct *mm, unsigned long vaddr, pte_t **last_pte)
{
    pgd_t *pgd;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    unsigned long paddr = 0;


    pgd = pgd_offset(mm, vaddr);
    printk("pgd_val = 0x%lx\n", pgd_val(*pgd));
    printk("pgd_index = %lu\n", pgd_index(vaddr));
    if (pgd_none(*pgd)) {
        printk("not mapped in pgd\n");
        return -1;
    }

    pud = pud_offset(pgd, vaddr);
    printk("pud_val = 0x%lx\n", pud_val(*pud));
    printk("pud_index = %lu\n", pud_index(vaddr));
    if (pud_none(*pud)) {
        printk("not mapped in pud\n");
        return -1;
    }

    pmd = pmd_offset(pud, vaddr);
    printk("pmd_val = 0x%lx\n", pmd_val(*pmd));
    printk("pmd_index = %lx\n", pmd_index(vaddr));
    if(pmd_none(*pmd)){
        printk("not mapped in pmd\n");
        return -1;
    }
    /*If pmd_large is true, represent pmd is the last level*/
    if(pmd_large(*pmd)){
        paddr = (pmd_val(*pmd) & PAGE_MASK);
        paddr = paddr | (vaddr & ~PAGE_MASK);
        *last_pte = (pte_t *)pmd;
        return paddr;
    }
    /*Walk the forth level page table
     * you may use PAGE_MASK = 0xfffffffffffff000 to help you get [0:11] bits
     * */
    else{
        /*Need to implement*/       
        /*...................*/
        /*...................*/       
        /*End of implement*/
        *last_pte = pte;
        return paddr;
    }

}

int set_page_rw( pte_t *pte){
    if(pte==NULL)
        return 1;
    pte->pte |= _PAGE_RW;
    return 0; 
}

int set_page_ro( pte_t *pte){    
    if(pte==NULL)
        return 1;
    pte->pte &= ~_PAGE_RW;
    return 0; 
}

void* paddr_to_kernel_vaddr(unsigned long paddr){
    if(paddr > PADDRPROTECT){
        paddr = paddr - PADDRPROTECT;
        return __va(paddr);
    }
    else
        return NULL;
}

static int my_init_module(void)
{ 
    unsigned long sys_addr, paddr;
    struct mm_struct *kernel_mm;
    printk("Insert %s\n", MYMODNAME);

    kernel_mm = INIT_MM;
    p = pid_task(find_vpid(PID), PIDTYPE_PID);          
    if(p==NULL){
        printk("Insert error due to wrong processID \n");
        return 1;
    }

    /*Translate user space array vaddr to paddr*/
    paddr = virtaddr_to_physaddr(p->mm, ADDR, &last_pte);
    if(paddr == -1){
        printk("Wrong paddr\n");
        return 1;
    }

    /*
     * Map user space paddr to kernel vaddr
     * after doing these you can use "array" as a char*
     * */
    array = paddr_to_kernel_vaddr(paddr);
    
    sys_call_table = (void *)SYS_CALL_TABLE;

    /*sys_addr = the address of do_open()*/
    sys_addr = (unsigned long)(sys_call_table+__NR_open);

    /*Translate sys_addr in order to find last page table entry(last_pte)*/
    paddr = virtaddr_to_physaddr( kernel_mm, sys_addr, &last_pte);
    if(paddr == -1){
        printk("Wrong paddr\n");
        return 1;
    }

    /*Set sys_call_table to read/write */
    set_page_rw(last_pte);
       
    do_open = sys_call_table[__NR_open];
    /*Redirect do_open() to Func() by change the value in sys_call_table[__NR_open]*/
    sys_call_table[ __NR_open ] = Func;    

    return 0;
}

static void my_cleanup_module(void)
{
    /*Recovery do_open() address and set sys_call_table page read_only*/ 
    sys_call_table[ __NR_open ] = do_open;
    set_page_ro(last_pte);

    printk(KERN_ALERT "Module %s unloaded.\n", MYMODNAME);
}

module_init(my_init_module);
module_exit(my_cleanup_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sense Lab");
MODULE_DESCRIPTION("OS Homework");
