/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1999, 2000, 06 Ralf Baechle (ralf@linux-mips.org)
 * Copyright (C) 1999, 2000 Silicon Graphics, Inc.
 */
#ifndef _ASM_SPINLOCK_H
#define _ASM_SPINLOCK_H

#include <asm/barrier.h>
#include <asm/war.h>

/*
 * Your basic SMP spinlocks, allowing only a single CPU anywhere
 */

#ifdef CONFIG_CPU_CAVIUM_OCTEON
#define __raw_spin_is_locked(x) ((x)->now_serving != (x)->ticket)
#define __raw_spin_lock_flags(lock, flags) __raw_spin_lock(lock)
#define __raw_spin_unlock_wait(x) \
	do { cpu_relax(); } while (__raw_spin_is_locked(x))
#else
#define __raw_spin_is_locked(x)       ((x)->lock != 0)
#define __raw_spin_lock_flags(lock, flags) __raw_spin_lock(lock)
#define __raw_spin_unlock_wait(x) \
	do { cpu_relax(); } while ((x)->lock)
#endif

/*
 * Simple spin lock operations.  There are two variants, one clears IRQ's
 * on the local processor, one does not.
 *
 * We make no fairness assumptions.  They have a cost.
 */

static inline void __raw_spin_lock(raw_spinlock_t *lock)
{
#ifdef CONFIG_CPU_CAVIUM_OCTEON
	int tmp;
	int my_ticket;
	__asm__ __volatile__ (
	"	.set push					\n"
	"	.set noreorder					\n"
	"1:							\n"
	"	ll	%[my_ticket], %[ticket_ptr]		\n"
	"	li	%[ticket], 1				\n"
	"	addu	%[ticket], %[my_ticket]			\n"
	"	sc	%[ticket], %[ticket_ptr]		\n"
	"	beqz	%[ticket], 1b				\n"
	"	 syncw						\n"
	"	syncw						\n"
	"	lw	%[ticket], %[now_serving]		\n"
	"	bne	%[ticket], %[my_ticket], 2f		\n"
	"	 subu	%[ticket], %[my_ticket], %[ticket]	\n"
	"4:							\n"
	"	.subsection 2					\n"
	"2:							\n"
	"	subu	%[ticket], 1				\n"
	"	sll	%[ticket], 5				\n"
	"3:							\n"
	"	bnez	%[ticket], 3b				\n"
	"	 subu	%[ticket], 1				\n"
	"	lw	%[ticket], %[now_serving]		\n"
	"	beq	%[ticket], %[my_ticket], 4b		\n"
	"	 subu	%[ticket], %[my_ticket], %[ticket]	\n"
	"	subu	%[ticket], 1				\n"
	"	b	3b					\n"
	"	 sll	%[ticket], 5				\n"
	"	.previous					\n"
	"	.set pop					\n"
	: [ticket_ptr] "=m" (lock->ticket),
	[now_serving] "=m" (lock->now_serving),
	[ticket] "=r" (tmp),
	[my_ticket] "=r" (my_ticket));
#else
	unsigned int tmp;

	if (R10000_LLSC_WAR) {
		__asm__ __volatile__(
		"	.set	noreorder	# __raw_spin_lock	\n"
		"1:	ll	%1, %2					\n"
		"	bnez	%1, 1b					\n"
		"	 li	%1, 1					\n"
		"	sc	%1, %0					\n"
		"	beqzl	%1, 1b					\n"
		"	 nop						\n"
		"	.set	reorder					\n"
		: "=m" (lock->lock), "=&r" (tmp)
		: "m" (lock->lock)
		: "memory");
	} else {
		__asm__ __volatile__(
		"	.set	noreorder	# __raw_spin_lock	\n"
		"1:	ll	%1, %2					\n"
		"	bnez	%1, 2f					\n"
		"	 li	%1, 1					\n"
		"	sc	%1, %0					\n"
		"	beqz	%1, 2f					\n"
		"	 nop						\n"
		"	.subsection 2					\n"
		"2:	ll	%1, %2					\n"
		"	bnez	%1, 2b					\n"
		"	 li	%1, 1					\n"
		"	b	1b					\n"
		"	 nop						\n"
		"	.previous					\n"
		"	.set	reorder					\n"
		: "=m" (lock->lock), "=&r" (tmp)
		: "m" (lock->lock)
		: "memory");
	}

	smp_mb();
#endif
}

static inline void __raw_spin_unlock(raw_spinlock_t *lock)
{
#ifdef CONFIG_CPU_CAVIUM_OCTEON
	int now_serving = lock->now_serving;
	wmb();
	if (likely(now_serving != lock->ticket))
		lock->now_serving = now_serving+1;
	wmb();
#else
	smp_mb();

	__asm__ __volatile__(
	"	.set	noreorder	# __raw_spin_unlock	\n"
	"	sw	$0, %0					\n"
	"	.set\treorder					\n"
	: "=m" (lock->lock)
	: "m" (lock->lock)
	: "memory");
#endif
}

static inline unsigned int __raw_spin_trylock(raw_spinlock_t *lock)
{
#ifdef CONFIG_CPU_CAVIUM_OCTEON
	if (__raw_spin_is_locked(lock))
		return 0;
	__raw_spin_lock(lock);
	return 1;
#else
	unsigned int temp, res;

	if (R10000_LLSC_WAR) {
		__asm__ __volatile__(
		"	.set	noreorder	# __raw_spin_trylock	\n"
		"1:	ll	%0, %3					\n"
		"	ori	%2, %0, 1				\n"
		"	sc	%2, %1					\n"
		"	beqzl	%2, 1b					\n"
		"	 nop						\n"
		"	andi	%2, %0, 1				\n"
		"	.set	reorder"
		: "=&r" (temp), "=m" (lock->lock), "=&r" (res)
		: "m" (lock->lock)
		: "memory");
	} else {
		__asm__ __volatile__(
		"	.set	noreorder	# __raw_spin_trylock	\n"
		"1:	ll	%0, %3					\n"
		"	ori	%2, %0, 1				\n"
		"	sc	%2, %1					\n"
		"	beqz	%2, 2f					\n"
		"	 andi	%2, %0, 1				\n"
		"	.subsection 2					\n"
		"2:	b	1b					\n"
		"	 nop						\n"
		"	.previous					\n"
		"	.set	reorder"
		: "=&r" (temp), "=m" (lock->lock), "=&r" (res)
		: "m" (lock->lock)
		: "memory");
	}

	smp_mb();

	return res == 0;
#endif
}

/*
 * Read-write spinlocks, allowing multiple readers but only one writer.
 *
 * NOTE! it is quite common to have readers in interrupts but no interrupt
 * writers. For those circumstances we can "mix" irq-safe locks - any writer
 * needs to get a irq-safe write-lock, but readers can get non-irqsafe
 * read-locks.
 */

/*
 * read_can_lock - would read_trylock() succeed?
 * @lock: the rwlock in question.
 */
#define __raw_read_can_lock(rw)	((rw)->lock >= 0)

/*
 * write_can_lock - would write_trylock() succeed?
 * @lock: the rwlock in question.
 */
#define __raw_write_can_lock(rw)	(!(rw)->lock)

static inline void __raw_read_lock(raw_rwlock_t *rw)
{
	unsigned int tmp;

	if (R10000_LLSC_WAR) {
		__asm__ __volatile__(
		"	.set	noreorder	# __raw_read_lock	\n"
		"1:	ll	%1, %2					\n"
		"	bltz	%1, 1b					\n"
		"	 addu	%1, 1					\n"
		"	sc	%1, %0					\n"
		"	beqzl	%1, 1b					\n"
		"	 nop						\n"
		"	.set	reorder					\n"
		: "=m" (rw->lock), "=&r" (tmp)
		: "m" (rw->lock)
		: "memory");
	} else {
#ifdef CONFIG_CPU_CAVIUM_OCTEON
		__asm__ __volatile__(
		"	.set	noreorder	# _raw_read_lock	\n"
		OCTEON_SYNCW_STR
		"1:	ll	%1, %2					\n"
		"	bltz	%1, 1b					\n"
		"	 addu	%1, 1					\n"
		"	sc	%1, %0					\n"
		"	beqz	%1, 1b					\n"
		"	 nop						\n"
		"	.set	reorder					\n"
		: "=m" (rw->lock), "=&r" (tmp)
		: "m" (rw->lock)
		: "memory");
#else
		__asm__ __volatile__(
		"	.set	noreorder	# __raw_read_lock	\n"
		"1:	ll	%1, %2					\n"
		"	bltz	%1, 2f					\n"
		"	 addu	%1, 1					\n"
		"	sc	%1, %0					\n"
		"	beqz	%1, 1b					\n"
		"	 nop						\n"
		"	.subsection 2					\n"
		"2:	ll	%1, %2					\n"
		"	bltz	%1, 2b					\n"
		"	 addu	%1, 1					\n"
		"	b	1b					\n"
		"	 nop						\n"
		"	.previous					\n"
		"	.set	reorder					\n"
		: "=m" (rw->lock), "=&r" (tmp)
		: "m" (rw->lock)
		: "memory");
#endif
	}

	smp_mb();
}

/* Note the use of sub, not subu which will make the kernel die with an
   overflow exception if we ever try to unlock an rwlock that is already
   unlocked or is being held by a writer.  */
static inline void __raw_read_unlock(raw_rwlock_t *rw)
{
	unsigned int tmp;

	smp_mb();

	if (R10000_LLSC_WAR) {
		__asm__ __volatile__(
		"1:	ll	%1, %2		# __raw_read_unlock	\n"
		"	sub	%1, 1					\n"
		"	sc	%1, %0					\n"
		"	beqzl	%1, 1b					\n"
		: "=m" (rw->lock), "=&r" (tmp)
		: "m" (rw->lock)
		: "memory");
	} else {
#ifdef CONFIG_CPU_CAVIUM_OCTEON
		__asm__ __volatile__(
		"	.set	noreorder	# _raw_read_unlock	\n"
		OCTEON_SYNCW_STR
		"1:	ll	%1, %2					\n"
		"	sub	%1, 1					\n"
		"	sc	%1, %0					\n"
		"	beqz	%1, 1b					\n"
		"	 nop						\n"
		"	.set	reorder					\n"
		: "=m" (rw->lock), "=&r" (tmp)
		: "m" (rw->lock)
		: "memory");
#else
		__asm__ __volatile__(
		"	.set	noreorder	# __raw_read_unlock	\n"
		"1:	ll	%1, %2					\n"
		"	sub	%1, 1					\n"
		"	sc	%1, %0					\n"
		"	beqz	%1, 2f					\n"
		"	 nop						\n"
		"	.subsection 2					\n"
		"2:	b	1b					\n"
		"	 nop						\n"
		"	.previous					\n"
		"	.set	reorder					\n"
		: "=m" (rw->lock), "=&r" (tmp)
		: "m" (rw->lock)
		: "memory");
#endif
	}
}

static inline void __raw_write_lock(raw_rwlock_t *rw)
{
	unsigned int tmp;

	if (R10000_LLSC_WAR) {
		__asm__ __volatile__(
		"	.set	noreorder	# __raw_write_lock	\n"
		"1:	ll	%1, %2					\n"
		"	bnez	%1, 1b					\n"
		"	 lui	%1, 0x8000				\n"
		"	sc	%1, %0					\n"
		"	beqzl	%1, 1b					\n"
		"	 nop						\n"
		"	.set	reorder					\n"
		: "=m" (rw->lock), "=&r" (tmp)
		: "m" (rw->lock)
		: "memory");
	} else {
#ifdef CONFIG_CPU_CAVIUM_OCTEON
		__asm__ __volatile__(
		"	.set	noreorder	# _raw_write_lock	\n"
		OCTEON_SYNCW_STR
		"1:	ll	%1, %2					\n"
		"	bnez	%1, 1b					\n"
		"	 lui	%1, 0x8000				\n"
		"	sc	%1, %0					\n"
		"	beqz	%1, 1b					\n"
		"	 nop						\n"
		"	.set	reorder					\n"
		: "=m" (rw->lock), "=&r" (tmp)
		: "m" (rw->lock)
		: "memory");
#else
		__asm__ __volatile__(
		"	.set	noreorder	# __raw_write_lock	\n"
		"1:	ll	%1, %2					\n"
		"	bnez	%1, 2f					\n"
		"	 lui	%1, 0x8000				\n"
		"	sc	%1, %0					\n"
		"	beqz	%1, 2f					\n"
		"	 nop						\n"
		"	.subsection 2					\n"
		"2:	ll	%1, %2					\n"
		"	bnez	%1, 2b					\n"
		"	 lui	%1, 0x8000				\n"
		"	b	1b					\n"
		"	 nop						\n"
		"	.previous					\n"
		"	.set	reorder					\n"
		: "=m" (rw->lock), "=&r" (tmp)
		: "m" (rw->lock)
		: "memory");
#endif
	}

	smp_mb();
}

static inline void __raw_write_unlock(raw_rwlock_t *rw)
{
	smp_mb();

	__asm__ __volatile__(
	"				# __raw_write_unlock	\n"
	"	sw	$0, %0					\n"
#ifdef CONFIG_CPU_CAVIUM_OCTEON
	OCTEON_SYNCW_STR
#endif
	: "=m" (rw->lock)
	: "m" (rw->lock)
	: "memory");
}

static inline int __raw_read_trylock(raw_rwlock_t *rw)
{
	unsigned int tmp;
	int ret;

	if (R10000_LLSC_WAR) {
		__asm__ __volatile__(
		"	.set	noreorder	# __raw_read_trylock	\n"
		"	li	%2, 0					\n"
		"1:	ll	%1, %3					\n"
		"	bltz	%1, 2f					\n"
		"	 addu	%1, 1					\n"
		"	sc	%1, %0					\n"
		"	.set	reorder					\n"
		"	beqzl	%1, 1b					\n"
		"	 nop						\n"
		__WEAK_ORDERING_MB
		"	li	%2, 1					\n"
		"2:							\n"
		: "=m" (rw->lock), "=&r" (tmp), "=&r" (ret)
		: "m" (rw->lock)
		: "memory");
	} else {
		__asm__ __volatile__(
		"	.set	noreorder	# __raw_read_trylock	\n"
		"	li	%2, 0					\n"
		"1:	ll	%1, %3					\n"
		"	bltz	%1, 2f					\n"
		"	 addu	%1, 1					\n"
		"	sc	%1, %0					\n"
		"	beqz	%1, 1b					\n"
		"	 nop						\n"
		"	.set	reorder					\n"
		__WEAK_ORDERING_MB
		"	li	%2, 1					\n"
		"2:							\n"
		: "=m" (rw->lock), "=&r" (tmp), "=&r" (ret)
		: "m" (rw->lock)
		: "memory");
	}

	return ret;
}

static inline int __raw_write_trylock(raw_rwlock_t *rw)
{
	unsigned int tmp;
	int ret;

	if (R10000_LLSC_WAR) {
		__asm__ __volatile__(
		"	.set	noreorder	# __raw_write_trylock	\n"
		"	li	%2, 0					\n"
		"1:	ll	%1, %3					\n"
		"	bnez	%1, 2f					\n"
		"	 lui	%1, 0x8000				\n"
		"	sc	%1, %0					\n"
		"	beqzl	%1, 1b					\n"
		"	 nop						\n"
		__WEAK_ORDERING_MB
		"	li	%2, 1					\n"
		"	.set	reorder					\n"
		"2:							\n"
		: "=m" (rw->lock), "=&r" (tmp), "=&r" (ret)
		: "m" (rw->lock)
		: "memory");
	} else {
#ifdef CONFIG_CPU_CAVIUM_OCTEON
		__asm__ __volatile__(
		"	.set	noreorder	# _raw_write_trylock	\n"
		OCTEON_SYNCW_STR
		"	li	%2, 0					\n"
		"1:	ll	%1, %3					\n"
		"	bnez	%1, 2f					\n"
		"	lui	%1, 0x8000				\n"
		"	sc	%1, %0					\n"
		"	beqz	%1, 1b					\n"
		"	 nop						\n"
		"	li	%2, 1					\n"
		"	.set	reorder					\n"
		"2:							\n"
		: "=m" (rw->lock), "=&r" (tmp), "=&r" (ret)
		: "m" (rw->lock)
		: "memory");
#else
		__asm__ __volatile__(
		"	.set	noreorder	# __raw_write_trylock	\n"
		"	li	%2, 0					\n"
		"1:	ll	%1, %3					\n"
		"	bnez	%1, 2f					\n"
		"	lui	%1, 0x8000				\n"
		"	sc	%1, %0					\n"
		"	beqz	%1, 3f					\n"
		"	 li	%2, 1					\n"
		"2:							\n"
		__WEAK_ORDERING_MB
		"	.subsection 2					\n"
		"3:	b	1b					\n"
		"	 li	%2, 0					\n"
		"	.previous					\n"
		"	.set	reorder					\n"
		: "=m" (rw->lock), "=&r" (tmp), "=&r" (ret)
		: "m" (rw->lock)
		: "memory");
#endif
	}

	return ret;
}


#define _raw_spin_relax(lock)	cpu_relax()
#define _raw_read_relax(lock)	cpu_relax()
#define _raw_write_relax(lock)	cpu_relax()

#endif /* _ASM_SPINLOCK_H */
