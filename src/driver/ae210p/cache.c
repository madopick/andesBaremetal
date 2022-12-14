/*
 * Copyright (c) 2012-2018 Andes Technology Corporation
 * All rights reserved.
 *
 */

#include "cache.h"

#define MEMSET(s, c, n) __builtin_memset ((s), (c), (n))

static inline void GIE_SAVE(unsigned long *var)
{
	*var = __nds32__mfsr(NDS32_SR_PSW);
	__nds32__gie_dis();
}

static inline void GIE_RESTORE(unsigned long var)
{
	if (var & PSW_mskGIE)
		__nds32__gie_en();
}

#define chk_range_alignment(start, end, line_size)

void nds32_dcache_invalidate(void)
{
#ifdef CFG_CACHE_ENABLE
	__nds32__cctl_l1d_invalall();

	__nds32__msync_store();
	__nds32__dsb();
#endif
}

void nds32_dcache_flush(void)
{
#ifdef CFG_CACHE_ENABLE
#ifndef CFG_CACHE_WRITETHROUGH
	unsigned long saved_gie;
#endif
	unsigned long end;
	unsigned long cache_line;

	cache_line = CACHE_LINE_SIZE(DCACHE);
	end = CACHE_WAY(DCACHE) * CACHE_SET(DCACHE) * cache_line;

#ifndef CFG_CACHE_WRITETHROUGH
	GIE_SAVE(&saved_gie);

	/*
	 * Use CCTL L1D_IX_WB/L1D_IX_INVAL subtype instead of combined
	 * L1D_IX_WBINVAL. Because only N903 supports L1D_IX_WBINVAL.
	 */
	do {
		end -= cache_line;
		__nds32__cctlidx_wbinval(NDS32_CCTL_L1D_IX_WB, end);
        __nds32__cctlidx_wbinval(NDS32_CCTL_L1D_IX_INVAL, end);
	} while (end > 0);
	GIE_RESTORE(saved_gie);
#else
	while (end > 0){
		end -= cache_line;
		__nds32__cctlidx_wbinval(NDS32_CCTL_L1D_IX_INVAL, end);
	}
#endif
	__nds32__msync_store();
	__nds32__dsb();
#endif
}

void nds32_dcache_clean_range(unsigned long start, unsigned long end){

#ifdef CFG_CACHE_ENABLE
#ifndef CFG_CACHE_WRITETHROUGH

	unsigned long line_size;

	line_size = CACHE_LINE_SIZE(DCACHE);
	chk_range_alignment(start, end, line_size);

	while (end > start){
		__nds32__cctlva_wbinval_one_lvl(NDS32_CCTL_L1D_VA_WB, (void *)start);
		start += line_size;
	}

	__nds32__msync_store();
	__nds32__dsb();
#endif
#endif
}

/*
 * nds32_dcache_invalidate_range(start, end)
 *
 * throw away all D-cached data in specified region without an obligation
 * to write them back. Note however that we must clean the D-cached entries
 * around the boundaries if the start and/or end address are not cache
 * aligned.
 */
void nds32_dcache_invalidate_range(unsigned long start, unsigned long end)
{
#ifdef CFG_CACHE_ENABLE
	unsigned long line_size;

	line_size = CACHE_LINE_SIZE(DCACHE);
	chk_range_alignment(start, end, line_size);

	while (end > start){
		__nds32__cctlva_wbinval_one_lvl(NDS32_CCTL_L1D_VA_INVAL, (void *)start);
		start += line_size;
	}
#endif
}

void nds32_dcache_flush_range(unsigned long start, unsigned long end)
{
#ifdef CFG_CACHE_ENABLE
	unsigned long line_size;

	line_size = CACHE_LINE_SIZE(DCACHE);

	while (end > start){
#ifndef CFG_CACHE_WRITETHROUGH
		__nds32__cctlva_wbinval_one_lvl(NDS32_CCTL_L1D_VA_WB, (void *)start);
#endif
		__nds32__cctlva_wbinval_one_lvl(NDS32_CCTL_L1D_VA_INVAL, (void *)start);
		start += line_size;
	}
#endif
}

void nds32_dcache_writeback_range(unsigned long start, unsigned long end)
{
#ifdef CFG_CACHE_ENABLE
#ifndef CFG_CACHE_WRITETHROUGH
	unsigned long line_size;

	line_size = CACHE_LINE_SIZE(DCACHE);

	while (end > start){
		__nds32__cctlva_wbinval_one_lvl(NDS32_CCTL_L1D_VA_WB, (void *)start);
		start += line_size;
	}
#endif
#endif
}

void nds32_icache_flush(void)
{
#ifdef CFG_CACHE_ENABLE
	unsigned long end;
	unsigned long cache_line = CACHE_LINE_SIZE(ICACHE);

	end = CACHE_WAY(ICACHE) * CACHE_SET(ICACHE) * CACHE_LINE_SIZE(ICACHE);

	do {
		end -= cache_line;
		__nds32__cctlidx_wbinval(NDS32_CCTL_L1I_IX_INVAL, end);
	} while (end > 0);

	__nds32__isb();
#endif
}

void nds32_icache_invalidate_range(unsigned long start, unsigned long end)
{
#ifdef CFG_CACHE_ENABLE

	unsigned long line_size;

	line_size = CACHE_LINE_SIZE(ICACHE);

	start &= (~(line_size-1));
	end = ( end + line_size - 1 )&(~(line_size-1));
	if (end == start)
		end += line_size;

	while (end > start){

		end -= line_size;
		__nds32__cctlva_wbinval_one_lvl(NDS32_CCTL_L1I_VA_INVAL, (void *)end);
	}
#endif
}

void unaligned_cache_line_move(unsigned char* src, unsigned char* dst, unsigned long len )
{
         int i;
         unsigned char* src_p = (unsigned char*)src;
         unsigned char* dst_p = (unsigned char*)dst;

         for( i = 0 ;i < len; ++i)
                 *(dst_p+i)=*(src_p+i);
}

void nds32_dma_clean_range(unsigned long start, unsigned long end)
{
	unsigned long line_size;
	line_size = CACHE_LINE_SIZE(DCACHE);
	start = start & (~(line_size-1));
	end = (end + line_size -1) & (~(line_size-1));
	if (start == end)
		return;

	nds32_dcache_clean_range(start, end);
}

void nds32_dma_inv_range(unsigned long start, unsigned long end)
{
	unsigned long line_size;
	unsigned long old_start=start;
	unsigned long old_end=end;
	line_size = CACHE_LINE_SIZE(DCACHE);
	unsigned char h_buf[line_size];
	unsigned char t_buf[line_size];
	MEMSET((void*)h_buf,0,line_size);
	MEMSET((void*)t_buf,0,line_size);

	start = start & (~(line_size-1));
	end = (end + line_size -1) & (~(line_size-1));
	if (start == end)
		return;
	if (start != old_start)
	{
		// nds32_dcache_flush_range(start, start + line_size);
		unaligned_cache_line_move((unsigned char*)start, h_buf, old_start - start);
	}
	if (end != old_end)
	{
		// nds32_dcache_flush_range(end - line_size ,end);
		unaligned_cache_line_move((unsigned char*)old_end, t_buf, end - old_end);

	}
	nds32_dcache_invalidate_range(start, end);

	/* handle cache line unaligned */
	if(start != old_start)
		unaligned_cache_line_move(h_buf,(unsigned char*)start, old_start - start);

	if( end != old_end )
		unaligned_cache_line_move(t_buf,(unsigned char*)old_end, end - old_end);
}

void nds32_dma_flush_range(unsigned long start, unsigned long end)
{
	unsigned long line_size;
	line_size = CACHE_LINE_SIZE(DCACHE);
	start = start & (~(line_size-1));
	end = (end + line_size -1 ) & (~(line_size-1));
	if (start == end)
		return;

	nds32_dcache_flush_range(start, end);
}
