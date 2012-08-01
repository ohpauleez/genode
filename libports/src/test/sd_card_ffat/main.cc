/*
 * \brief  Test for writing via libc_fs plugin to ffat_fs
 * \author Norman Feske
 * \date   2012-08-01
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>


bool attempt_write(int fd, size_t num_bytes)
{
	/* allocate buffer on stack */
	char buf[num_bytes];

	/* fill buffer with consecutive numbers */
	for (unsigned i = 0; i < num_bytes; i++)
		buf[i] = i;

	int ret = write(fd, buf, sizeof(buf));

	if (ret < 0) {
		printf("error: write returned %d, errno=%d\n", ret, errno);
		return false;
	}

	printf("write(%zd) returned %d\n", sizeof(buf), ret);
	return true;
}


int main(int argc, char **argv)
{
	printf("-- test-sd_card_ffat started ---\n");

	char const *filename = "test.dat";

	int fd = open(filename, O_RDWR | O_CREAT, 0666);

	if (attempt_write(fd, 4480)
	 && attempt_write(fd, 384))
		printf("test succeeded\n");

	close(fd);

	printf("-- test-sd_card_ffat finished ---\n");
	return 0;
}
