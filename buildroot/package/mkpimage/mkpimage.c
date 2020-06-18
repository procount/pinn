#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <endian.h>

#define VALIDATION_WORD 0x31305341

#define BRANCH_INST 0xea /* ARM opcode for "b" (unconditional branch) */

#define MAX_V0IMAGE_SIZE (60 * 1024 - 4)
/* Max size without authentication is 224 KB, due to memory used by
 * the ROM boot code as a workspace out of the 256 KB of OCRAM */
#define MAX_V1IMAGE_SIZE (224 * 1024 - 4)

static int add_barebox_header;

struct socfpga_header {
	uint8_t validation_word[4];
	uint8_t version;
	uint8_t flags;
	union {
		struct {
			uint8_t program_length[2];
			uint8_t spare[2];
			uint8_t checksum[2];
			uint8_t start_vector[4];
		} v0;
		struct {
			uint8_t header_length[2];
			uint8_t program_length[4];
			uint8_t entry_offset[4];
			uint8_t spare[2];
			uint8_t checksum[2];
		} v1;
	};
};

static uint32_t bb_header[] = {
	0xea00007e,	/* b 0x200  */
	0xeafffffe,	/* 1: b 1b  */
	0xeafffffe,	/* 1: b 1b  */
	0xeafffffe,	/* 1: b 1b  */
	0xeafffffe,	/* 1: b 1b  */
	0xeafffffe,	/* 1: b 1b  */
	0xeafffffe,	/* 1: b 1b  */
	0xeafffffe,	/* 1: b 1b  */
	0x65726162,	/* 'bare'   */
	0x00786f62,	/* 'box\0'  */
	0x00000000,	/* padding  */
	0x00000000,	/* padding  */
	0x00000000,	/* padding  */
	0x00000000,	/* padding  */
	0x00000000,	/* padding  */
	0x00000000,	/* padding  */
	0x00000000,	/* socfpga header */
	0x00000000,	/* socfpga header */
	0x00000000,	/* socfpga header */
	0xea00006b,	/* entry. b 0x200 (offset may be adjusted) */
};

static int read_full(int fd, void *buf, size_t size)
{
	size_t insize = size;
	int now;
	int total = 0;

	while (size) {
		now = read(fd, buf, size);
		if (now == 0)
			return total;
		if (now < 0)
			return now;
		total += now;
		size -= now;
		buf += now;
	}

	return insize;
}

static int write_full(int fd, void *buf, size_t size)
{
	size_t insize = size;
	int now;

	while (size) {
		now = write(fd, buf, size);
		if (now <= 0)
			return now;
		size -= now;
		buf += now;
	}

	return insize;
}

static const uint32_t crc_table[256] = {
	0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
	0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
	0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
	0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
	0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
	0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
	0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
	0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
	0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
	0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
	0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
	0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
	0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
	0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
	0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
	0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
	0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
	0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
	0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
	0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
	0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
	0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
	0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
	0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
	0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
	0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
	0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
	0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
	0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
	0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
	0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
	0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
	0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
	0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
	0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
	0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
	0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
	0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
	0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
	0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
	0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
	0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
	0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

uint32_t crc32(uint32_t crc, void *_buf, int length)
{
	uint8_t *buf = _buf;

	while (length--)
		crc = crc << 8 ^ crc_table[(crc >> 24 ^ *(buf++)) & 0xff];

	return crc;
}

/* Create an ARM relative branch instuction 
 * branch is where the instruction will be placed and dest points to where
 * it should branch too. */
static void branch(uint8_t *branch, uint8_t *dest)
{
	int offset = dest - branch - 8; /* PC is offset +8 bytes on ARM */

	branch[0] = (offset >> 2) & 0xff; /* instruction uses offset/4 */
	branch[1] = (offset >> 10) & 0xff;
	branch[2] = (offset >> 18) & 0xff;
	branch[3] = BRANCH_INST;
}

/* start_addr is where the socfpga header's start instruction should branch to.
 * It should be relative to the start of buf */
static int add_socfpga_header(void *buf, size_t size, unsigned start_addr, unsigned version)
{
	struct socfpga_header *header = buf + 0x40;
	void *entry;
	uint8_t *bufp, *sumendp;
	uint32_t *crc;
	unsigned checksum;

	if (size & 0x3) {
		fprintf(stderr, "%s: size must be multiple of 4\n", __func__);
		return -EINVAL;
	}

	/* Absolute address of entry point in buf */
	entry = buf + start_addr;
	if (version == 0) {
		sumendp = &header->v0.checksum[0];
	} else {
		sumendp = &header->v1.checksum[0];

		/* The ROM loader can't handle a negative offset */
		if (entry < (void*)header) {
			/* add a trampoline branch inst after end of the header */
			uint8_t *trampoline = (void*)(header + 1);
			branch(trampoline, entry);

			/* and then make the trampoline the entry point */
			entry = trampoline;
		}
		/* Calculate start address as offset relative to start of header */
		start_addr = entry - (void*)header;
	}

	header->validation_word[0] = VALIDATION_WORD & 0xff;
	header->validation_word[1] = (VALIDATION_WORD >> 8) & 0xff;
	header->validation_word[2] = (VALIDATION_WORD >> 16) & 0xff;
	header->validation_word[3] = (VALIDATION_WORD >> 24) & 0xff;
	header->version = version;
	header->flags = 0;

	if (version == 0) {
		header->v0.program_length[0] = (size >>  2) & 0xff; /* length in words */
		header->v0.program_length[1] = (size >> 10) & 0xff;
		header->v0.spare[0] = 0;
		header->v0.spare[1] = 0;
		branch(header->v0.start_vector, entry);
	} else {
		header->v1.header_length[0] = (sizeof(*header) >> 0) & 0xff;
		header->v1.header_length[1] = (sizeof(*header) >> 8) & 0xff;
		header->v1.program_length[0] = (size >>  0) & 0xff;
		header->v1.program_length[1] = (size >>  8) & 0xff;
		header->v1.program_length[2] = (size >> 16) & 0xff;
		header->v1.program_length[3] = (size >> 24) & 0xff;
		header->v1.entry_offset[0] = (start_addr >>  0) & 0xff;
		header->v1.entry_offset[1] = (start_addr >>  8) & 0xff;
		header->v1.entry_offset[2] = (start_addr >> 16) & 0xff;
		header->v1.entry_offset[3] = (start_addr >> 24) & 0xff;
		header->v1.spare[0] = 0;
		header->v1.spare[1] = 0;
	}

	/* Sum from beginning of header to start of checksum field */
	checksum = 0;
	for (bufp = (uint8_t*)header; bufp < sumendp; bufp++)
		checksum += *bufp;

	if (version == 0) {
		header->v0.checksum[0] = checksum & 0xff;;
		header->v0.checksum[1] = (checksum >> 8) & 0xff;;
	} else {
		header->v1.checksum[0] = checksum & 0xff;;
		header->v1.checksum[1] = (checksum >> 8) & 0xff;;
	}

	crc = buf + size - sizeof(uint32_t);

	*crc = crc32(0xffffffff, buf, size - sizeof(uint32_t));
	*crc ^= 0xffffffff;

	return 0;
}

static void usage(const char *prgname)
{
	fprintf(stderr, "usage: %s [-hb] [-v version] <infile> -o <outfile>\n", prgname);
}

int main(int argc, char *argv[])
{
	int opt, ret;
	const char *outfile = NULL, *infile;
	struct stat s;
	void *buf;
	int fd;
	int max_image_size, min_image_size = 80;
	int addsize = 0, pad;
	unsigned int version = 0;

	while ((opt = getopt(argc, argv, "o:hbv:")) != -1) {
		switch (opt) {
		case 'v':
			version = atoi(optarg);
			if (version > 1) {
				printf("Versions supported: 0 or 1\n");
				usage(argv[0]);
				exit(1);
			}
			break;
		case 'b':
			add_barebox_header = 1;
			min_image_size = 0;
			addsize = 512;
			break;
		case 'h':
			usage(argv[0]);
			exit(0);
		case 'o':
			outfile = optarg;
			break;
		default:
			usage(argv[0]);
			exit(1);
		}
	}
	if (version == 0) {
		max_image_size = MAX_V0IMAGE_SIZE;
	} else {
		max_image_size = MAX_V1IMAGE_SIZE;
	}
	max_image_size -= addsize;

	if (optind == argc || !outfile) {
		usage(argv[0]);
		exit(1);
	}
	infile = argv[optind];

	ret = stat(infile, &s);
	if (ret) {
		perror("stat");
		exit(1);
	}

	if (s.st_size < min_image_size) {
		fprintf(stderr, "input image too small. Minimum is %d bytes\n",
			min_image_size);
		exit(1);
	}

	if (s.st_size > max_image_size) {
		fprintf(stderr, "input image too big. Maximum is %d bytes, got %ld bytes\n",
				max_image_size, s.st_size);
		exit(1);
	}

	fd = open(infile, O_RDONLY);
	if (fd == -1) {
		perror("open infile");
		exit(1);
	}

	pad = s.st_size & 0x3;
	if (pad)
		pad = 4 - pad;

	buf = calloc(s.st_size + 4 + addsize + pad, 1);
	if (!buf) {
		perror("malloc");
		exit(1);
	}

	ret = read_full(fd, buf + addsize, s.st_size);
	if (ret < 0) {
		perror("read infile");
		exit(1);
	}

	close(fd);

	if (add_barebox_header) {
		memcpy(buf, bb_header, sizeof(bb_header));
	}

	ret = add_socfpga_header(buf, s.st_size + 4 + addsize + pad, addsize,
	                         version);
	if (ret)
		exit(1);

	fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0) {
		perror("open outfile");
		exit(1);
	}

	ret = write_full(fd, buf, s.st_size + 4 + addsize + pad);
	if (ret < 0) {
		perror("write outfile");
		exit(1);
	}

	exit(0);
}
