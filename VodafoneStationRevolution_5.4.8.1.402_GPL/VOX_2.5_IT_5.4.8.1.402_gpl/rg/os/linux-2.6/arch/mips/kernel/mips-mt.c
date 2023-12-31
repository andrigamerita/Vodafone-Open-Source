/*
 * General MIPS MT support routines, usable in AP/SP, SMVP, or SMTC kernels
 * Copyright (C) 2005 Mips Technologies, Inc
 */

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/cpumask.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/security.h>

#include <asm/cpu.h>
#include <asm/processor.h>
#include <asm/atomic.h>
#include <asm/system.h>
#include <asm/hardirq.h>
#include <asm/mmu_context.h>
#include <asm/smp.h>
#include <asm/mipsmtregs.h>
#include <asm/r4kcache.h>
#include <asm/cacheflush.h>

int vpelimit;

static int __init maxvpes(char *str)
{
	get_option(&str, &vpelimit);

	return 1;
}

__setup("maxvpes=", maxvpes);

int tclimit;

static int __init maxtcs(char *str)
{
	get_option(&str, &tclimit);

	return 1;
}

__setup("maxtcs=", maxtcs);

#ifdef CONFIG_IFX_VPE_EXT
int stlb;

static int __init istlbshared(char *str)
{
       get_option(&str, &stlb);

       return 1;
}

__setup("vpe_tlb_shared=", istlbshared);

int vpe0_wired;

static int __init vpe0wired(char *str)
{
       get_option(&str, &vpe0_wired);

       return 1;
}

__setup("vpe0_wired_tlb_entries=", vpe0wired);

int vpe1_wired;

static int __init vpe1wired(char *str)
{
       get_option(&str, &vpe1_wired);

       return 1;
}

__setup("vpe1_wired_tlb_entries=", vpe1wired);

#ifdef CONFIG_MIPS_MT_SMTC
extern int nostlb;
#endif
void configure_tlb(void)
{
       	int vpeflags, tcflags, tlbsiz;
        unsigned int config1val;
	vpeflags = dvpe();
	tcflags = dmt();
	write_c0_vpeconf0((read_c0_vpeconf0() | VPECONF0_MVP));
	write_c0_mvpcontrol((read_c0_mvpcontrol() | MVPCONTROL_VPC));
	mips_ihb();
	//printk("stlb = %d, vpe0_wired = %d vpe1_wired=%d\n", stlb,vpe0_wired, vpe1_wired);
	if (stlb) {
                if (!(read_c0_mvpconf0() & MVPCONF0_TLBS)) {
			emt(tcflags);
                        evpe(vpeflags);
                        return;
                }

		write_c0_mvpcontrol(read_c0_mvpcontrol() | MVPCONTROL_STLB);
		write_c0_wired(vpe0_wired + vpe1_wired);
                if (((read_vpe_c0_config() & MIPS_CONF_MT) >> 7) == 1) {
                        config1val = read_vpe_c0_config1();
                        tlbsiz = (((config1val >> 25) & 0x3f) + 1);
                        if (tlbsiz > 64)
                                tlbsiz = 64;
                        cpu_data[0].tlbsize = current_cpu_data.tlbsize = tlbsiz;
                }

	}
	else {
		write_c0_mvpcontrol(read_c0_mvpcontrol() & ~MVPCONTROL_STLB);
		write_c0_wired(vpe0_wired);
	}

	ehb();
	write_c0_mvpcontrol((read_c0_mvpcontrol() & ~MVPCONTROL_VPC));
	ehb();
        local_flush_tlb_all(); 

	printk("Wired TLB entries for Linux read_c0_wired() = %d\n", read_c0_wired());
#ifdef CONFIG_MIPS_MT_SMTC
	nostlb = !stlb;
#endif
	emt(tcflags);
	evpe(vpeflags);
}

#endif

/*
 * CPU mask used to set process affinity for MT VPEs/TCs with FPUs
 */

cpumask_t mt_fpu_cpumask;

#ifdef CONFIG_MIPS_MT_FPAFF

#include <linux/cpu.h>
#include <linux/delay.h>
#include <asm/uaccess.h>

unsigned long mt_fpemul_threshold = 0;

/*
 * Replacement functions for the sys_sched_setaffinity() and
 * sys_sched_getaffinity() system calls, so that we can integrate
 * FPU affinity with the user's requested processor affinity.
 * This code is 98% identical with the sys_sched_setaffinity()
 * and sys_sched_getaffinity() system calls, and should be
 * updated when kernel/sched.c changes.
 */

/*
 * find_process_by_pid - find a process with a matching PID value.
 * used in sys_sched_set/getaffinity() in kernel/sched.c, so
 * cloned here.
 */
static inline struct task_struct *find_process_by_pid(pid_t pid)
{
	return pid ? find_task_by_pid(pid) : current;
}


/*
 * mipsmt_sys_sched_setaffinity - set the cpu affinity of a process
 */
asmlinkage long mipsmt_sys_sched_setaffinity(pid_t pid, unsigned int len,
				      unsigned long __user *user_mask_ptr)
{
	cpumask_t new_mask;
	cpumask_t effective_mask;
	int retval;
	struct task_struct *p;

	if (len < sizeof(new_mask))
		return -EINVAL;

	if (copy_from_user(&new_mask, user_mask_ptr, sizeof(new_mask)))
		return -EFAULT;

	lock_cpu_hotplug();
	read_lock(&tasklist_lock);

	p = find_process_by_pid(pid);
	if (!p) {
		read_unlock(&tasklist_lock);
		unlock_cpu_hotplug();
		return -ESRCH;
	}

	/*
	 * It is not safe to call set_cpus_allowed with the
	 * tasklist_lock held.  We will bump the task_struct's
	 * usage count and drop tasklist_lock before invoking
	 * set_cpus_allowed.
	 */
	get_task_struct(p);

	retval = -EPERM;
	if ((current->euid != p->euid) && (current->euid != p->uid) &&
			!capable(CAP_SYS_NICE)) {
		read_unlock(&tasklist_lock);
		goto out_unlock;
	}

	retval = security_task_setscheduler(p, 0, NULL);
	if (retval)
		goto out_unlock;

	/* Record new user-specified CPU set for future reference */
	p->thread.user_cpus_allowed = new_mask;

	/* Unlock the task list */
	read_unlock(&tasklist_lock);

	/* Compute new global allowed CPU set if necessary */
	if( (p->thread.mflags & MF_FPUBOUND)
	&& cpus_intersects(new_mask, mt_fpu_cpumask)) {
		cpus_and(effective_mask, new_mask, mt_fpu_cpumask);
		retval = set_cpus_allowed(p, effective_mask);
	} else {
		p->thread.mflags &= ~MF_FPUBOUND;
		retval = set_cpus_allowed(p, new_mask);
	}


out_unlock:
	put_task_struct(p);
	unlock_cpu_hotplug();
	return retval;
}

/*
 * mipsmt_sys_sched_getaffinity - get the cpu affinity of a process
 */
asmlinkage long mipsmt_sys_sched_getaffinity(pid_t pid, unsigned int len,
				      unsigned long __user *user_mask_ptr)
{
	unsigned int real_len;
	cpumask_t mask;
	int retval;
	struct task_struct *p;

	real_len = sizeof(mask);
	if (len < real_len)
		return -EINVAL;

	lock_cpu_hotplug();
	read_lock(&tasklist_lock);

	retval = -ESRCH;
	p = find_process_by_pid(pid);
	if (!p)
		goto out_unlock;
	retval = security_task_getscheduler(p);
	if (retval)
		goto out_unlock;

	cpus_and(mask, p->thread.user_cpus_allowed, cpu_possible_map);

out_unlock:
	read_unlock(&tasklist_lock);
	unlock_cpu_hotplug();
	if (retval)
		return retval;
	if (copy_to_user(user_mask_ptr, &mask, real_len))
		return -EFAULT;
	return real_len;
}

#endif /* CONFIG_MIPS_MT_FPAFF */

/*
 * Dump new MIPS MT state for the core. Does not leave TCs halted.
 * Takes an argument which taken to be a pre-call MVPControl value.
 */

void mips_mt_regdump(unsigned long mvpctl)
{
	unsigned long flags;
	unsigned long vpflags;
	unsigned long mvpconf0;
	int nvpe;
	int ntc;
	int i;
	int tc;
	unsigned long haltval;
	unsigned long tcstatval;
#ifdef CONFIG_MIPS_MT_SMTC
	void smtc_soft_dump(void);
#endif /* CONFIG_MIPT_MT_SMTC */

	local_irq_save(flags);
	vpflags = dvpe();
	printk("=== MIPS MT State Dump ===\n");
	printk("-- Global State --\n");
	printk("   MVPControl Passed: %08lx\n", mvpctl);
	printk("   MVPControl Read: %08lx\n", vpflags);
	printk("   MVPConf0 : %08lx\n", (mvpconf0 = read_c0_mvpconf0()));
	nvpe = ((mvpconf0 & MVPCONF0_PVPE) >> MVPCONF0_PVPE_SHIFT) + 1;
	ntc = ((mvpconf0 & MVPCONF0_PTC) >> MVPCONF0_PTC_SHIFT) + 1;
	printk("-- per-VPE State --\n");
	for(i = 0; i < nvpe; i++) {
	    for(tc = 0; tc < ntc; tc++) {
			settc(tc);
		if((read_tc_c0_tcbind() & TCBIND_CURVPE) == i) {
		    printk("  VPE %d\n", i);
		    printk("   VPEControl : %08lx\n", read_vpe_c0_vpecontrol());
		    printk("   VPEConf0 : %08lx\n", read_vpe_c0_vpeconf0());
		    printk("   VPE%d.Status : %08lx\n",
				i, read_vpe_c0_status());
		    printk("   VPE%d.EPC : %08lx\n", i, read_vpe_c0_epc());
		    printk("   VPE%d.Cause : %08lx\n", i, read_vpe_c0_cause());
		    printk("   VPE%d.Config7 : %08lx\n",
				i, read_vpe_c0_config7());
		    break; /* Next VPE */
		}
	    }
	}
	printk("-- per-TC State --\n");
	for(tc = 0; tc < ntc; tc++) {
		settc(tc);
		if(read_tc_c0_tcbind() == read_c0_tcbind()) {
			/* Are we dumping ourself?  */
			haltval = 0; /* Then we're not halted, and mustn't be */
			tcstatval = flags; /* And pre-dump TCStatus is flags */
			printk("  TC %d (current TC with VPE EPC above)\n", tc);
		} else {
			haltval = read_tc_c0_tchalt();
			write_tc_c0_tchalt(1);
			tcstatval = read_tc_c0_tcstatus();
			printk("  TC %d\n", tc);
		}
		printk("   TCStatus : %08lx\n", tcstatval);
		printk("   TCBind : %08lx\n", read_tc_c0_tcbind());
		printk("   TCRestart : %08lx\n", read_tc_c0_tcrestart());
		printk("   TCHalt : %08lx\n", haltval);
		printk("   TCContext : %08lx\n", read_tc_c0_tccontext());
		if (!haltval)
			write_tc_c0_tchalt(0);
	}
#ifdef CONFIG_MIPS_MT_SMTC
	smtc_soft_dump();
#endif /* CONFIG_MIPT_MT_SMTC */
	printk("===========================\n");
	evpe(vpflags);
	local_irq_restore(flags);
}

static int mt_opt_norps = 0;
static int mt_opt_rpsctl = -1;
static int mt_opt_nblsu = -1;
static int mt_opt_forceconfig7 = 0;
static int mt_opt_config7 = -1;

static int __init rps_disable(char *s)
{
	mt_opt_norps = 1;
	return 1;
}
__setup("norps", rps_disable);

static int __init rpsctl_set(char *str)
{
	get_option(&str, &mt_opt_rpsctl);
	return 1;
}
__setup("rpsctl=", rpsctl_set);

static int __init nblsu_set(char *str)
{
	get_option(&str, &mt_opt_nblsu);
	return 1;
}
__setup("nblsu=", nblsu_set);

static int __init config7_set(char *str)
{
	get_option(&str, &mt_opt_config7);
	mt_opt_forceconfig7 = 1;
	return 1;
}
__setup("config7=", config7_set);

/* Experimental cache flush control parameters that should go away some day */
int mt_protiflush = 0;
int mt_protdflush = 0;
int mt_n_iflushes = 1;
int mt_n_dflushes = 1;

static int __init set_protiflush(char *s)
{
	mt_protiflush = 1;
	return 1;
}
__setup("protiflush", set_protiflush);

static int __init set_protdflush(char *s)
{
	mt_protdflush = 1;
	return 1;
}
__setup("protdflush", set_protdflush);

static int __init niflush(char *s)
{
	get_option(&s, &mt_n_iflushes);
	return 1;
}
__setup("niflush=", niflush);

static int __init ndflush(char *s)
{
	get_option(&s, &mt_n_dflushes);
	return 1;
}
__setup("ndflush=", ndflush);
#ifdef CONFIG_MIPS_MT_FPAFF
static int fpaff_threshold = -1;

static int __init fpaff_thresh(char *str)
{
	get_option(&str, &fpaff_threshold);
	return 1;
}

__setup("fpaff=", fpaff_thresh);
#endif /* CONFIG_MIPS_MT_FPAFF */

static unsigned int itc_base = 0;

static int __init set_itc_base(char *str)
{
	get_option(&str, &itc_base);
	return 1;
}

__setup("itcbase=", set_itc_base);

void mips_mt_set_cpuoptions(void)
{
	unsigned int oconfig7 = read_c0_config7();
	unsigned int nconfig7 = oconfig7;

	if (mt_opt_norps) {
		printk("\"norps\" option deprectated: use \"rpsctl=\"\n");
	}
	if (mt_opt_rpsctl >= 0) {
		printk("34K return prediction stack override set to %d.\n",
			mt_opt_rpsctl);
		if (mt_opt_rpsctl)
			nconfig7 |= (1 << 2);
		else
			nconfig7 &= ~(1 << 2);
	}
	if (mt_opt_nblsu >= 0) {
		printk("34K ALU/LSU sync override set to %d.\n", mt_opt_nblsu);
		if (mt_opt_nblsu)
			nconfig7 |= (1 << 5);
		else
			nconfig7 &= ~(1 << 5);
	}
	if (mt_opt_forceconfig7) {
		printk("CP0.Config7 forced to 0x%08x.\n", mt_opt_config7);
		nconfig7 = mt_opt_config7;
	}
	if (oconfig7 != nconfig7) {
		__asm__ __volatile("sync");
		write_c0_config7(nconfig7);
		ehb ();
		printk("Config7: 0x%08x\n", read_c0_config7());
	}

	/* Report Cache management debug options */
	if (mt_protiflush)
		printk("I-cache flushes single-threaded\n");
	if (mt_protdflush)
		printk("D-cache flushes single-threaded\n");
	if (mt_n_iflushes != 1)
		printk("I-Cache Flushes Repeated %d times\n", mt_n_iflushes);
	if (mt_n_dflushes != 1)
		printk("D-Cache Flushes Repeated %d times\n", mt_n_dflushes);

#ifdef CONFIG_MIPS_MT_FPAFF
	/* FPU Use Factor empirically derived from experiments on 34K */
#define FPUSEFACTOR 333

	if (fpaff_threshold >= 0) {
		mt_fpemul_threshold = fpaff_threshold;
	} else {
		mt_fpemul_threshold =
			(FPUSEFACTOR * (loops_per_jiffy/(500000/HZ))) / HZ;
	}
	printk("FPU Affinity set after %ld emulations\n",
			mt_fpemul_threshold);
#endif /* CONFIG_MIPS_MT_FPAFF */

	if (itc_base != 0) {
		/*
		 * Configure ITC mapping.  This code is very
		 * specific to the 34K core family, which uses
		 * a special mode bit ("ITC") in the ErrCtl
		 * register to enable access to ITC control
		 * registers via cache "tag" operations.
		 */
		unsigned long ectlval;
		unsigned long itcblkgrn;

		/* ErrCtl register is known as "ecc" to Linux */
		ectlval = read_c0_ecc();
		write_c0_ecc(ectlval | (0x1 << 26));
		ehb();
#define INDEX_0 (0x80000000)
#define INDEX_8 (0x80000008)
		/* Read "cache tag" for Dcache pseudo-index 8 */
		cache_op(Index_Load_Tag_D, INDEX_8);
		ehb();
		itcblkgrn = read_c0_dtaglo();
		itcblkgrn &= 0xfffe0000;
		/* Set for 128 byte pitch of ITC cells */
		itcblkgrn |= 0x00000c00;
		/* Stage in Tag register */
		write_c0_dtaglo(itcblkgrn);
		ehb();
		/* Write out to ITU with CACHE op */
		cache_op(Index_Store_Tag_D, INDEX_8);
		/* Now set base address, and turn ITC on with 0x1 bit */
		write_c0_dtaglo((itc_base & 0xfffffc00) | 0x1 );
		ehb();
		/* Write out to ITU with CACHE op */
		cache_op(Index_Store_Tag_D, INDEX_0);
		write_c0_ecc(ectlval);
		ehb();
		printk("Mapped %ld ITC cells starting at 0x%08x\n",
			((itcblkgrn & 0x7fe00000) >> 20), itc_base);
	}
#ifdef CONFIG_IFX_VPE_EXT
	configure_tlb();
#endif
}

/*
 * Function to protect cache flushes from concurrent execution
 * depends on MP software model chosen.
 */

void mt_cflush_lockdown(void)
{
#ifdef CONFIG_MIPS_MT_SMTC
	void smtc_cflush_lockdown(void);

	smtc_cflush_lockdown();
#endif /* CONFIG_MIPS_MT_SMTC */
	/* FILL IN VSMP and AP/SP VERSIONS HERE */
}

void mt_cflush_release(void)
{
#ifdef CONFIG_MIPS_MT_SMTC
	void smtc_cflush_release(void);

	smtc_cflush_release();
#endif /* CONFIG_MIPS_MT_SMTC */
	/* FILL IN VSMP and AP/SP VERSIONS HERE */
}

struct class *mt_class;

static int __init mt_init(void)
{
	struct class *mtc;

	mtc = class_create(THIS_MODULE, "mt");
	if (IS_ERR(mtc))
		return PTR_ERR(mtc);

	mt_class = mtc;

	return 0;
}

subsys_initcall(mt_init);
