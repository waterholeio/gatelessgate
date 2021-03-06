// Gateless Gate, a Zcash miner
// Copyright 2016 zawawa @ bitcointalk.org
//
// The initial version of this software was based on:
// SILENTARMY v5
// The MIT License (MIT) Copyright (c) 2016 Marc Bevand, Genoil
//
// This program is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.If not, see <http://www.gnu.org/licenses/>.

#define PARAM_H

// When you tweak parameters in this file, make sure to uncomment the next line,
// rebuild sa-solver, and run it to see how many slots get dropped
// at each round. Note that performance suffers if too many slots get dropped.

//define ENABLE_DEBUG

//
// Parameters for Hash Tables
//

// There are PARAM_K - 1 hash tables, and each hash table has NR_ROWS rows.
// Each row contains NR_SLOTS slots.

#define NR_ROWS_LOG            15  // 12, 13, 14, or 15. 12 and 13 are not practically usable.
#define NR_SLOTS               127 // A prime number is preferable, but NEXT_PRIME_NO() cannot be used here.
#define LOCAL_WORK_SIZE        64  
#define THREADS_PER_ROW        64
#define LOCAL_WORK_SIZE_SOLS   128
#define THREADS_PER_ROW_SOLS   128
#define GLOBAL_WORK_SIZE_RATIO 512 // global_work_size = GLOBAL_WORK_SIZE_RATIO * nr_compute_units * LOCAL_WORK_SIZE
#define THREADS_PER_WRITE      1   // 1, 2, 4, or 8
#define SLOT_CACHE_SIZE        NEXT_PRIME_NO(NR_SLOTS * (LOCAL_WORK_SIZE / THREADS_PER_ROW) * 75 / 100)
#define LDS_COLL_SIZE          NEXT_PRIME_NO(NR_SLOTS * (LOCAL_WORK_SIZE / THREADS_PER_ROW) * 120 / 100)
#define BIN_SIZE               NEXT_PRIME_NO(NR_SLOTS * 25 / 100)

#if NR_SLOTS < 255
#define SLOT_CACHE_INDEX_TYPE uchar
#else
#define SLOT_CACHE_INDEX_TYPE ushort
#endif

#define PARAM_N				   200
#define PARAM_K			       9
#define PREFIX                 (PARAM_N / (PARAM_K + 1))
#define NR_INPUTS              (1 << PREFIX)
// Approximate log base 2 of number of elements in hash tables
#define APX_NR_ELMS_LOG        (PREFIX + 1)

// Setting this to 1 might make Gateless Gate faster, see TROUBLESHOOTING.md
#define OPTIM_SIMPLIFY_ROUND   1

// Ratio of time of sleeping before rechecking if task is done (0-1)
#define SLEEP_RECHECK_RATIO 0.60
// Ratio of time to busy wait for the solution (0-1)
// The higher value the higher CPU usage with Nvidia
#define SLEEP_SKIP_RATIO 0.005

// Make hash tables OVERHEAD times larger than necessary to store the average
// number of elements per row. The ideal value is as small as possible to
// reduce memory usage, but not too small or else elements are dropped from the
// hash tables.
//
// The actual number of elements per row is closer to the theoretical average
// (less variance) when NR_ROWS_LOG is small. So accordingly OVERHEAD can be
// smaller.
//
// Even (as opposed to odd) values of OVERHEAD sometimes significantly decrease
// performance as they cause VRAM channel conflicts.
#if NR_ROWS_LOG <= 16
#define OVERHEAD                        2
#elif NR_ROWS_LOG == 17
#define OVERHEAD                        3
#elif NR_ROWS_LOG == 18
#define OVERHEAD                        3
#elif NR_ROWS_LOG == 19
#define OVERHEAD                        5
#elif NR_ROWS_LOG == 20 && OPTIM_SIMPLIFY_ROUND
#define OVERHEAD                        6
#elif NR_ROWS_LOG == 20
#define OVERHEAD                        9
#endif

#define NR_ROWS                         (1 << NR_ROWS_LOG)
#ifndef NR_SLOTS
#define NR_SLOTS                        (((1 << (APX_NR_ELMS_LOG - NR_ROWS_LOG)) * OVERHEAD))
#endif
// Length of 1 element (slot) in byte
#define SLOT_LEN                        32
// Total size of hash table
#define HT_SIZE				(NR_ROWS * NR_SLOTS * SLOT_LEN)
// Length of Zcash block header, nonce (part of header)
#define ZCASH_BLOCK_HEADER_LEN		140
// Offset of nTime in header
#define ZCASH_BLOCK_OFFSET_NTIME        (4 + 3 * 32)
// Length of nonce
#define ZCASH_NONCE_LEN			32
// Length of encoded representation of solution size
#define ZCASH_SOLSIZE_LEN		3
// Solution size (1344 = 0x540) represented as a compact integer, in hex
#define ZCASH_SOLSIZE_HEX               "fd4005"
// Length of encoded solution (512 * 21 bits / 8 = 1344 bytes)
#define ZCASH_SOL_LEN                   ((1 << PARAM_K) * (PREFIX + 1) / 8)
// Last N_ZERO_BYTES of nonce must be zero due to my BLAKE2B optimization
#define N_ZERO_BYTES			12
// Number of bytes Zcash needs out of Blake
#define ZCASH_HASH_LEN                  50
// Number of wavefronts per SIMD for the Blake kernel.
// Blake is ALU-bound (beside the atomic counter being incremented) so we need
// at least 2 wavefronts per SIMD to hide the 2-clock latency of integer
// instructions. 10 is the max supported by the hw.
#define BLAKE_WPS               	10
// Maximum number of solutions reported by kernel to host
#define MAX_SOLS			11
// Length of SHA256 target
#define SHA256_TARGET_LEN               (256 / 8)

#if (NR_SLOTS < 3)
#define BITS_PER_ROW 2
#define ROWS_PER_UINT 16
#define ROW_MASK 0x03
#elif (NR_SLOTS < 7)
#define BITS_PER_ROW 3
#define ROWS_PER_UINT 10
#define ROW_MASK 0x07
#elif (NR_SLOTS < 15)
#define BITS_PER_ROW 4
#define ROWS_PER_UINT 8
#define ROW_MASK 0x0F
#elif (NR_SLOTS < 31)
#define BITS_PER_ROW 5
#define ROWS_PER_UINT 6
#define ROW_MASK 0x1F
#elif (NR_SLOTS < 63)
#define BITS_PER_ROW 6
#define ROWS_PER_UINT 5
#define ROW_MASK 0x3F
#elif (NR_SLOTS < 255)
#define BITS_PER_ROW 8
#define ROWS_PER_UINT 4
#define ROW_MASK 0xFF
#else
#define BITS_PER_ROW 16
#define ROWS_PER_UINT 2
#define ROW_MASK 0xFFFF
#endif
#define RC_SIZE (NR_ROWS * 4 / ROWS_PER_UINT)

/*
** Return the offset of Xi in bytes from the beginning of the slot.
*/
#define xi_offset_for_round(round)	4

// An (uncompressed) solution stores (1 << PARAM_K) 32-bit values
#define SOL_SIZE			((1 << PARAM_K) * 4)
typedef struct	sols_s
{
	uint	nr;
	uint	likely_invalids;
	uchar	valid[MAX_SOLS];
	uint	values[MAX_SOLS][(1 << PARAM_K)];
}		sols_t;

#if NR_ROWS_LOG <= 12 && NR_SLOTS <= (1 << 10)

#define ENCODE_INPUTS(row, slot0, slot1) \
    ((row << 20) | ((slot1 & 0x3ff) << 10) | (slot0 & 0x3ff))
#define DECODE_ROW(REF)   (REF >> 20)
#define DECODE_SLOT1(REF) ((REF >> 10) & 0x3ff)
#define DECODE_SLOT0(REF) (REF & 0x3ff)

#elif NR_ROWS_LOG <= 14 && NR_SLOTS <= (1 << 9)

#define ENCODE_INPUTS(row, slot0, slot1) \
    ((row << 18) | ((slot1 & 0x1ff) << 9) | (slot0 & 0x1ff))
#define DECODE_ROW(REF)   (REF >> 18)
#define DECODE_SLOT1(REF) ((REF >> 9) & 0x1ff)
#define DECODE_SLOT0(REF) (REF & 0x1ff)

#elif NR_ROWS_LOG <= 16 && NR_SLOTS <= (1 << 8)

#define ENCODE_INPUTS(row, slot0, slot1) \
    ((row << 16) | ((slot1 & 0xff) << 8) | (slot0 & 0xff))
#define DECODE_ROW(REF)   (REF >> 16)
#define DECODE_SLOT1(REF) ((REF >> 8) & 0xff)
#define DECODE_SLOT0(REF) (REF & 0xff)

#elif NR_ROWS_LOG <= 18 && NR_SLOTS <= (1 << 7)

#define ENCODE_INPUTS(row, slot0, slot1) \
    ((row << 14) | ((slot1 & 0x7f) << 7) | (slot0 & 0x7f))
#define DECODE_ROW(REF)   (REF >> 14)
#define DECODE_SLOT1(REF) ((REF >> 7) & 0x7f)
#define DECODE_SLOT0(REF) (REF & 0x7f)

#elif NR_ROWS_LOG == 19 && NR_SLOTS <= (1 << 6)

#define ENCODE_INPUTS(row, slot0, slot1) \
    ((row << 13) | ((slot1 & 0x3f) << 6) | (slot0 & 0x3f)) /* 1 spare bit */
#define DECODE_ROW(REF)   (REF >> 13)
#define DECODE_SLOT1(REF) ((REF >> 6) & 0x3f)
#define DECODE_SLOT0(REF) (REF & 0x3f)

#elif NR_ROWS_LOG == 20 && NR_SLOTS <= (1 << 6)

#define ENCODE_INPUTS(row, slot0, slot1) \
    ((row << 12) | ((slot1 & 0x3f) << 6) | (slot0 & 0x3f))
#define DECODE_ROW(REF)   (REF >> 12)
#define DECODE_SLOT1(REF) ((REF >> 6) & 0x3f)
#define DECODE_SLOT0(REF) (REF & 0x3f)

#else
#error "unsupported NR_ROWS_LOG"
#endif

// Windows only for now
#define DEFAULT_NUM_MINING_MODE_THREADS 1
#define MAX_NUM_MINING_MODE_THREADS 16

#if THREADS_PER_WRITE == 1
#define ADJUSTED_SLOT_LEN(round) (((round) <= 5) ? SLOT_LEN : SLOT_LEN - 16)
#else
#define ADJUSTED_SLOT_LEN(round) SLOT_LEN
#endif

#define OPENCL_BUILD_OPTIONS_AMD "-I.. -I. -O1"
#define OPENCL_BUILD_OPTIONS     "-I.. -I."

#define NEXT_PRIME_NO(n) \
	(((n) <= 2) ? 2 : \
     ((n) <= 3) ? 3 : \
     ((n) <= 5) ? 5 : \
     ((n) <= 7) ? 7 : \
     ((n) <= 11) ? 11 : \
     ((n) <= 13) ? 13 : \
     ((n) <= 17) ? 17 : \
     ((n) <= 19) ? 19 : \
     ((n) <= 23) ? 23 : \
     ((n) <= 29) ? 29 : \
     ((n) <= 31) ? 31 : \
     ((n) <= 37) ? 37 : \
     ((n) <= 41) ? 41 : \
     ((n) <= 43) ? 43 : \
     ((n) <= 47) ? 47 : \
     ((n) <= 53) ? 53 : \
     ((n) <= 59) ? 59 : \
     ((n) <= 61) ? 61 : \
     ((n) <= 67) ? 67 : \
     ((n) <= 71) ? 71 : \
     ((n) <= 73) ? 73 : \
     ((n) <= 79) ? 79 : \
     ((n) <= 83) ? 83 : \
     ((n) <= 89) ? 89 : \
     ((n) <= 97) ? 97 : \
     ((n) <= 101) ? 101 : \
     ((n) <= 103) ? 103 : \
     ((n) <= 107) ? 107 : \
     ((n) <= 109) ? 109 : \
     ((n) <= 113) ? 113 : \
     ((n) <= 127) ? 127 : \
     ((n) <= 131) ? 131 : \
     ((n) <= 137) ? 137 : \
     ((n) <= 139) ? 139 : \
     ((n) <= 149) ? 149 : \
     ((n) <= 151) ? 151 : \
     ((n) <= 157) ? 157 : \
     ((n) <= 163) ? 163 : \
     ((n) <= 167) ? 167 : \
     ((n) <= 173) ? 173 : \
     ((n) <= 179) ? 179 : \
     ((n) <= 181) ? 181 : \
     ((n) <= 191) ? 191 : \
     ((n) <= 193) ? 193 : \
     ((n) <= 197) ? 197 : \
     ((n) <= 199) ? 199 : \
     ((n) <= 211) ? 211 : \
     ((n) <= 223) ? 223 : \
     ((n) <= 227) ? 227 : \
     ((n) <= 229) ? 229 : \
     ((n) <= 233) ? 233 : \
     ((n) <= 239) ? 239 : \
     ((n) <= 241) ? 241 : \
     ((n) <= 251) ? 251 : \
     ((n) <= 257) ? 257 : \
     ((n) <= 263) ? 263 : \
     ((n) <= 269) ? 269 : \
     ((n) <= 271) ? 271 : \
     ((n) <= 277) ? 277 : \
     ((n) <= 281) ? 281 : \
     ((n) <= 283) ? 283 : \
     ((n) <= 293) ? 293 : \
     ((n) <= 307) ? 307 : \
     ((n) <= 311) ? 311 : \
     ((n) <= 313) ? 313 : \
     ((n) <= 317) ? 317 : \
     ((n) <= 331) ? 331 : \
     ((n) <= 337) ? 337 : \
     ((n) <= 347) ? 347 : \
     ((n) <= 349) ? 349 : \
     ((n) <= 353) ? 353 : \
     ((n) <= 359) ? 359 : \
     ((n) <= 367) ? 367 : \
     ((n) <= 373) ? 373 : \
     ((n) <= 379) ? 379 : \
     ((n) <= 383) ? 383 : \
     ((n) <= 389) ? 389 : \
     ((n) <= 397) ? 397 : \
     ((n) <= 401) ? 401 : \
     ((n) <= 409) ? 409 : \
     ((n) <= 419) ? 419 : \
     ((n) <= 421) ? 421 : \
     ((n) <= 431) ? 431 : \
     ((n) <= 433) ? 433 : \
     ((n) <= 439) ? 439 : \
     ((n) <= 443) ? 443 : \
     ((n) <= 449) ? 449 : \
     ((n) <= 457) ? 457 : \
     ((n) <= 461) ? 461 : \
     ((n) <= 463) ? 463 : \
     ((n) <= 467) ? 467 : \
     ((n) <= 479) ? 479 : \
     ((n) <= 487) ? 487 : \
     ((n) <= 491) ? 491 : \
     ((n) <= 499) ? 499 : \
     ((n) <= 503) ? 503 : \
     ((n) <= 509) ? 509 : \
     ((n) <= 521) ? 521 : \
     ((n) <= 523) ? 523 : \
     ((n) <= 541) ? 541 : \
     ((n) <= 547) ? 547 : \
     ((n) <= 557) ? 557 : \
     ((n) <= 563) ? 563 : \
     ((n) <= 569) ? 569 : \
     ((n) <= 571) ? 571 : \
     ((n) <= 577) ? 577 : \
     ((n) <= 587) ? 587 : \
     ((n) <= 593) ? 593 : \
     ((n) <= 599) ? 599 : \
     ((n) <= 601) ? 601 : \
     ((n) <= 607) ? 607 : \
     ((n) <= 613) ? 613 : \
     ((n) <= 617) ? 617 : \
     ((n) <= 619) ? 619 : \
     ((n) <= 631) ? 631 : \
     ((n) <= 641) ? 641 : \
     ((n) <= 643) ? 643 : \
     ((n) <= 647) ? 647 : \
     ((n) <= 653) ? 653 : \
     ((n) <= 659) ? 659 : \
     ((n) <= 661) ? 661 : \
     ((n) <= 673) ? 673 : \
     ((n) <= 677) ? 677 : \
     ((n) <= 683) ? 683 : \
     ((n) <= 691) ? 691 : \
     ((n) <= 701) ? 701 : \
     ((n) <= 709) ? 709 : \
     ((n) <= 719) ? 719 : \
     ((n) <= 727) ? 727 : \
     ((n) <= 733) ? 733 : \
     ((n) <= 739) ? 739 : \
     ((n) <= 743) ? 743 : \
     ((n) <= 751) ? 751 : \
     ((n) <= 757) ? 757 : \
     ((n) <= 761) ? 761 : \
     ((n) <= 769) ? 769 : \
     ((n) <= 773) ? 773 : \
     ((n) <= 787) ? 787 : \
     ((n) <= 797) ? 797 : \
     ((n) <= 809) ? 809 : \
     ((n) <= 811) ? 811 : \
     ((n) <= 821) ? 821 : \
     ((n) <= 823) ? 823 : \
     ((n) <= 827) ? 827 : \
     ((n) <= 829) ? 829 : \
     ((n) <= 839) ? 839 : \
     ((n) <= 853) ? 853 : \
     ((n) <= 857) ? 857 : \
     ((n) <= 859) ? 859 : \
     ((n) <= 863) ? 863 : \
     ((n) <= 877) ? 877 : \
     ((n) <= 881) ? 881 : \
     ((n) <= 883) ? 883 : \
     ((n) <= 887) ? 887 : \
     ((n) <= 907) ? 907 : \
     ((n) <= 911) ? 911 : \
     ((n) <= 919) ? 919 : \
     ((n) <= 929) ? 929 : \
     ((n) <= 937) ? 937 : \
     ((n) <= 941) ? 941 : \
     ((n) <= 947) ? 947 : \
     ((n) <= 953) ? 953 : \
     ((n) <= 967) ? 967 : \
     ((n) <= 971) ? 971 : \
     ((n) <= 977) ? 977 : \
     ((n) <= 983) ? 983 : \
     ((n) <= 991) ? 991 : \
     ((n) <= 997) ? 997 : \
     ((n) <= 1009) ? 1009 : \
		             (n))
