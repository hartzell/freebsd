/*-
 * Copyright (c) 2019 George Hartzell <hartzell@freebsd.org>
 * TODO:
 *   - License
 *   - I might have an @freebsd.org email address, but...
 *
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "nvmecontrol.h"

void aws(int argc, char *argv[]);
static void print_dev(struct nvme_controller_data *cdata);
static void print_vol(struct nvme_controller_data *cdata);

#define AWS_NVME_VID 0x1D0F
#define AWS_NVME_EBS_MN "Amazon Elastic Block Store"
#define AWS_VS_DEV_LENGTH 32   /* from ebsnvme-id */
#define AWS_BUFFER_LENGTH 128

void print_dev(struct nvme_controller_data *cdata)
{
	uint8_t str[AWS_BUFFER_LENGTH];

	if (cdata->vid == AWS_NVME_VID &&
	    strncmp(cdata->mn, AWS_NVME_EBS_MN, strlen(AWS_NVME_EBS_MN)) == 0) {
		nvme_strvis(str, cdata->vs, AWS_BUFFER_LENGTH, AWS_VS_DEV_LENGTH);
		/* follow the lead of AMZN's ebsnvme-id tool */
		if (strncmp(str, "/dev/", 5) == 0	) {
			printf("%s\n", str+5);
		} else {
		printf("%s\n", str);
		}
	}
}

void print_vol(struct nvme_controller_data *cdata)
{
	uint8_t str[AWS_BUFFER_LENGTH];

	if (cdata->vid == AWS_NVME_VID &&
	    strncmp(cdata->mn, AWS_NVME_EBS_MN, strlen(AWS_NVME_EBS_MN)) == 0) {
		nvme_strvis(str, cdata->sn, AWS_BUFFER_LENGTH, NVME_SERIAL_NUMBER_LENGTH);
		/* follow the lead of AMZN's ebsnvme-id tool */
		if (strncmp(str, "vol", 3) == 0	&& strncmp(str, "vol-", 4) != 0) {
			printf("Volume ID: vol-%s\n", str+3);
		} else {
			printf("Volume ID: %s\n", str);
		}
	}
}

static void
aws_usage(void)
{
	fprintf(stderr, "usage:\n");
	fprintf(stderr, AWS_USAGE);
	exit(1);
}

void
aws(int argc, char *argv[])
{
	struct nvme_controller_data cdata;
	int do_dev = 0;
	int do_vol = 0;
	int do_all = 0;
	int	ch, fd;

	while ((ch = getopt(argc, argv, "dv")) != -1) {
		switch ((char)ch) {
		case 'd':
			do_dev = 1;
			break;
		case 'v':
			do_vol = 1;
			break;
		default:
			aws_usage();
		}
	}
	do_all = !(do_dev || do_vol);
	/* Check that a controller was specified. */
	if (optind >= argc)
		aws_usage();

	open_dev(argv[optind], &fd, 1, 1);
	read_controller_data(fd, &cdata);
	close(fd);

	if (do_dev || do_all)
		print_dev(&cdata);
	if (do_vol || do_all)
		print_vol(&cdata);

	exit(0);
}
