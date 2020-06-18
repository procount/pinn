/* vi: set sw=4 ts=4: */
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <getopt.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifndef __APPLE__
#include <sys/sysmacros.h>     /* major() and minor() */
#endif
#include <ftw.h>
#ifdef EXTENDED_ATTRIBUTES
#include <sys/capability.h>
#endif /* EXTENDED_ATTRIBUTES */

const char *bb_applet_name;
uid_t recursive_uid;
gid_t recursive_gid;
unsigned int recursive_mode;
#define PASSWD_PATH "etc/passwd"  /* MUST be relative */
#define GROUP_PATH "etc/group"  /* MUST be relative */

void bb_verror_msg(const char *s, va_list p)
{
	fflush(stdout);
	fprintf(stderr, "%s: ", bb_applet_name);
	vfprintf(stderr, s, p);
}

void bb_error_msg(const char *s, ...)
{
	va_list p;

	va_start(p, s);
	bb_verror_msg(s, p);
	va_end(p);
	putc('\n', stderr);
}

void bb_error_msg_and_die(const char *s, ...)
{
	va_list p;

	va_start(p, s);
	bb_verror_msg(s, p);
	va_end(p);
	putc('\n', stderr);
	exit(1);
}

void bb_vperror_msg(const char *s, va_list p)
{
	int err=errno;
	if(s == 0) s = "";
	bb_verror_msg(s, p);
	if (*s) s = ": ";
	fprintf(stderr, "%s%s\n", s, strerror(err));
}

void bb_perror_msg(const char *s, ...)
{
	va_list p;

	va_start(p, s);
	bb_vperror_msg(s, p);
	va_end(p);
}

void bb_perror_msg_and_die(const char *s, ...)
{
	va_list p;

	va_start(p, s);
	bb_vperror_msg(s, p);
	va_end(p);
	exit(1);
}

FILE *bb_xfopen(const char *path, const char *mode)
{
	FILE *fp;
	if ((fp = fopen(path, mode)) == NULL)
		bb_perror_msg_and_die("%s", path);
	return fp;
}

enum {
	FILEUTILS_PRESERVE_STATUS = 1,
	FILEUTILS_DEREFERENCE = 2,
	FILEUTILS_RECUR = 4,
	FILEUTILS_FORCE = 8,
	FILEUTILS_INTERACTIVE = 16
};
int bb_make_directory (char *path, long mode, int flags)
{
	mode_t mask;
	const char *fail_msg;
	char *s = path;
	char c;
	struct stat st;

	mask = umask(0);
	if (mode == -1) {
		umask(mask);
		mode = (S_IXUSR | S_IXGRP | S_IXOTH |
				S_IWUSR | S_IWGRP | S_IWOTH |
				S_IRUSR | S_IRGRP | S_IROTH) & ~mask;
	} else {
		umask(mask & ~0300);
	}

	do {
		c = 0;

		if (flags & FILEUTILS_RECUR) {	/* Get the parent. */
			/* Bypass leading non-'/'s and then subsequent '/'s. */
			while (*s) {
				if (*s == '/') {
					do {
						++s;
					} while (*s == '/');
					c = *s;		/* Save the current char */
					*s = 0;		/* and replace it with nul. */
					break;
				}
				++s;
			}
		}

		if (mkdir(path, 0777) < 0) {
			/* If we failed for any other reason than the directory
			 * already exists, output a diagnostic and return -1.*/
			if ((errno != EEXIST && errno != EISDIR)
					|| !(flags & FILEUTILS_RECUR)
					|| (stat(path, &st) < 0 || !S_ISDIR(st.st_mode))) {
				fail_msg = "create";
				umask(mask);
				break;
			}
			/* Since the directory exists, don't attempt to change
			 * permissions if it was the full target.  Note that
			 * this is not an error conditon. */
			if (!c) {
				umask(mask);
				return 0;
			}
		}

		if (!c) {
			/* Done.  If necessary, updated perms on the newly
			 * created directory.  Failure to update here _is_
			 * an error.*/
			umask(mask);
			if ((mode != -1) && (chmod(path, mode) < 0)){
				fail_msg = "set permissions of";
				break;
			}
			return 0;
		}

		/* Remove any inserted nul from the path (recursive mode). */
		*s = c;

	} while (1);

	bb_perror_msg ("Cannot %s directory `%s'", fail_msg, path);
	return -1;
}

const char * const bb_msg_memory_exhausted = "memory exhausted";

void *xmalloc(size_t size)
{
	void *ptr = malloc(size);
	if (ptr == NULL && size != 0)
		bb_error_msg_and_die(bb_msg_memory_exhausted);
	return ptr;
}

void *xcalloc(size_t nmemb, size_t size)
{
	void *ptr = calloc(nmemb, size);
	if (ptr == NULL && nmemb != 0 && size != 0)
		bb_error_msg_and_die(bb_msg_memory_exhausted);
	return ptr;
}

void *xrealloc(void *ptr, size_t size)
{
	ptr = realloc(ptr, size);
	if (ptr == NULL && size != 0)
		bb_error_msg_and_die(bb_msg_memory_exhausted);
	return ptr;
}

char *private_get_line_from_file(FILE *file, int c)
{
#define GROWBY (80)		/* how large we will grow strings by */

	int ch;
	int idx = 0;
	char *linebuf = NULL;
	int linebufsz = 0;

	while ((ch = getc(file)) != EOF) {
		/* grow the line buffer as necessary */
		if (idx > linebufsz - 2) {
			linebuf = xrealloc(linebuf, linebufsz += GROWBY);
		}
		linebuf[idx++] = (char)ch;
		if (!ch) return linebuf;
		if (c<2 && ch == '\n') {
			if (c) {
				--idx;
			}
			break;
		}
	}
	if (linebuf) {
		if (ferror(file)) {
			free(linebuf);
			return NULL;
		}
		linebuf[idx] = 0;
	}
	return linebuf;
}

char *bb_get_chomped_line_from_file(FILE *file)
{
	return private_get_line_from_file(file, 1);
}

long my_getpwnam(const char *name)
{
	struct passwd *myuser;
	FILE *stream;

	stream = bb_xfopen(PASSWD_PATH, "r");
	while(1) {
		errno = 0;
		myuser = fgetpwent(stream);
		if (myuser == NULL)
			bb_error_msg_and_die("unknown user name: %s", name);
		if (errno)
			bb_perror_msg_and_die("fgetpwent");
		if (!strcmp(name, myuser->pw_name))
			break;
	}
	fclose(stream);

	return myuser->pw_uid;
}

long my_getgrnam(const char *name)
{
	struct group *mygroup;
	FILE *stream;

	stream = bb_xfopen(GROUP_PATH, "r");
	while(1) {
		errno = 0;
		mygroup = fgetgrent(stream);
		if (mygroup == NULL)
			bb_error_msg_and_die("unknown group name: %s", name);
		if (errno)
			bb_perror_msg_and_die("fgetgrent");
		if (!strcmp(name, mygroup->gr_name))
			break;
	}
	fclose(stream);

	return mygroup->gr_gid;
}

unsigned long get_ug_id(const char *s, long (*my_getxxnam)(const char *))
{
	unsigned long r;
	char *p;

	r = strtoul(s, &p, 10);
	if (*p || (s == p)) {
		r = my_getxxnam(s);
	}

	return r;
}

char * last_char_is(const char *s, int c)
{
	char *sret = (char *)s;
	if (sret) {
		sret = strrchr(sret, c);
		if(sret != NULL && *(sret+1) != 0)
			sret = NULL;
	}
	return sret;
}

void bb_xasprintf(char **string_ptr, const char *format, ...)
{
	va_list p;
	int r;

	va_start(p, format);
	r = vasprintf(string_ptr, format, p);
	va_end(p);

	if (r < 0) {
		bb_perror_msg_and_die("bb_xasprintf");
	}
}

char *concat_path_file(const char *path, const char *filename)
{
	char *outbuf;
	char *lc;

	if (!path)
		path = "";
	lc = last_char_is(path, '/');
	while (*filename == '/')
		filename++;
	bb_xasprintf(&outbuf, "%s%s%s", path, (lc==NULL ? "/" : ""), filename);

	return outbuf;
}

#ifdef EXTENDED_ATTRIBUTES
int bb_set_xattr(const char *fpath, const char *xattr)
{
	cap_t cap, cap_file, cap_new;
	char *cap_file_text, *cap_new_text;
	ssize_t length;

	cap = cap_from_text(xattr);
	if (cap == NULL)
		bb_perror_msg_and_die("cap_from_text failed for %s", xattr);

	cap_file = cap_get_file(fpath);
	if (cap_file == NULL) {
		/* if no capability was set before, we initialize cap_file */
		if (errno != ENODATA)
			bb_perror_msg_and_die("cap_get_file failed on %s", fpath);

		cap_file = cap_init();
		if (!cap_file)
			bb_perror_msg_and_die("cap_init failed");
	}

	if ((cap_file_text = cap_to_text(cap_file, &length)) == NULL)
		bb_perror_msg_and_die("cap_to_name failed on %s", fpath);

	bb_xasprintf(&cap_new_text, "%s %s", cap_file_text, xattr);

	if ((cap_new = cap_from_text(cap_new_text)) == NULL)
		bb_perror_msg_and_die("cap_from_text failed on %s", cap_new_text);

	if (cap_set_file(fpath, cap_new) == -1)
		bb_perror_msg_and_die("cap_set_file failed for %s (xattr = %s)", fpath, xattr);

	cap_free(cap);
	cap_free(cap_file);
	cap_free(cap_file_text);
	cap_free(cap_new);
	cap_free(cap_new_text);

	return 0;
}
#endif /* EXTENDED_ATTRIBUTES */

void bb_show_usage(void)
{
	fprintf(stderr, "%s: [-d device_table] rootdir\n\n", bb_applet_name);
	fprintf(stderr, "Creates a batch of special files as specified in a device table.\n");
	fprintf(stderr, "Device table entries take the form of:\n");
	fprintf(stderr, "name type mode user group major minor start increment count\n\n");
	fprintf(stderr, "Where name is the file name,  type can be one of:\n");
	fprintf(stderr, "      f       A regular file\n");
	fprintf(stderr, "      d       Directory\n");
	fprintf(stderr, "      r       Directory recursively\n");
	fprintf(stderr, "      c       Character special device file\n");
	fprintf(stderr, "      b       Block special device file\n");
	fprintf(stderr, "      p       Fifo (named pipe)\n");
	fprintf(stderr, "uid is the user id for the target file, gid is the group id for the\n");
	fprintf(stderr, "target file.  The rest of the entries (major, minor, etc) apply to\n");
	fprintf(stderr, "to device special files.  A '-' may be used for blank entries.\n\n");
	fprintf(stderr, "For example:\n");
	fprintf(stderr, "<name>    <type> <mode> <uid> <gid> <major> <minor> <start> <inc> <count>\n");
	fprintf(stderr, "/dev         d    755    0    0     -       -       -       -     -\n");
	fprintf(stderr, "/dev/console c    666    0    0     5       1       -       -     -\n");
	fprintf(stderr, "/dev/null    c    666    0    0     1       3       0       0     -\n");
	fprintf(stderr, "/dev/zero    c    666    0    0     1       5       0       0     -\n");
	fprintf(stderr, "/dev/hda     b    640    0    0     3       0       0       0     -\n");
	fprintf(stderr, "/dev/hda     b    640    0    0     3       1       1       1     15\n");
	fprintf(stderr, "/dev/rtp     b    640    0    0     250     0       0       1     5\n");
	fprintf(stderr, "/dev/gps     b    640    0    0     251     0       1       1     5\n");
	fprintf(stderr, "/dev/uio     b    640    0    0     252     0       1       2     5\n");
	fprintf(stderr, "/dev/uio     b    640    0    0     252     1       6       2     5\n\n");
	fprintf(stderr, "Will Produce:\n");
	fprintf(stderr, "/dev\n");
	fprintf(stderr, "/dev/console\n");
	fprintf(stderr, "/dev/null\n");
	fprintf(stderr, "/dev/zero\n");
	fprintf(stderr, "/dev/hda\n");
	fprintf(stderr, "/dev/hda[1-15] with minor numbers [1-15]\n");
	fprintf(stderr, "/dev/rtp[0-4]  with minor numbers [0-4]\n");
	fprintf(stderr, "/dev/gps[1-5]  with minor numbers [0-4]\n");
	fprintf(stderr, "/dev/uio[1-5]  with minor numbers 0,2,4,6,8\n");
	fprintf(stderr, "/dev/uio[6-10] with minor numbers 1,3,5,7,9\n");
	exit(1);
}

int bb_recursive(const char *fpath, const struct stat *sb,
		int tflag, struct FTW *ftwbuf){

	if (chown(fpath, recursive_uid, recursive_gid) == -1) {
		bb_perror_msg("chown failed for %s", fpath);
		return -1;
	}
	if (recursive_mode != -1) {
		if (chmod(fpath, recursive_mode) < 0) {
			bb_perror_msg("chmod failed for %s", fpath);
			return -1;
		}
	}

	return 0;
}

int main(int argc, char **argv)
{
	int opt;
	FILE *table = stdin;
	char *rootdir = NULL;
	char *full_name = NULL;
	char *line = NULL;
	int linenum = 0;
	int ret = EXIT_SUCCESS;

	bb_applet_name = basename(argv[0]);

	while ((opt = getopt(argc, argv, "d:")) != -1) {
		switch(opt) {
			case 'd':
				table = bb_xfopen((line=optarg), "r");
				break;
			default:
				bb_show_usage();
		}
	}

	if (optind >= argc || (rootdir=argv[optind])==NULL) {
		bb_error_msg_and_die("root directory not speficied");
	}

	if (chdir(rootdir) != 0) {
		bb_perror_msg_and_die("Couldnt chdir to %s", rootdir);
	}

	umask(0);

	printf("rootdir=%s\n", rootdir);
	if (line) {
		printf("table='%s'\n", line);
	} else {
		printf("table=<stdin>\n");
	}

	while ((line = bb_get_chomped_line_from_file(table))) {
		char type;
		unsigned int mode = 0755;
		unsigned int major = 0;
		unsigned int minor = 0;
		unsigned int count = 0;
		unsigned int increment = 0;
		unsigned int start = 0;
		char xattr[255];
		char name[4096];
		char user[41];
		char group[41];
		uid_t uid;
		gid_t gid;

		linenum++;

		if (1 == sscanf(line, " |xattr %254s", xattr)) {
#ifdef EXTENDED_ATTRIBUTES
			if (!full_name)
				bb_error_msg_and_die("line %d should be after a file\n", linenum);

			if (bb_set_xattr(full_name, xattr) < 0)
				bb_error_msg_and_die("can't set cap %s on file %s\n", xattr, full_name);
#else
			bb_error_msg_and_die("line %d not supported: '%s'\nDid you forget to enable "
					     "BR2_ROOTFS_DEVICE_TABLE_SUPPORTS_EXTENDED_ATTRIBUTES?\n",
					     linenum, line);
#endif /* EXTENDED_ATTRIBUTES */
			continue;
		}

		if ((2 > sscanf(line, "%4095s %c %o %40s %40s %u %u %u %u %u", name,
						&type, &mode, user, group, &major,
						&minor, &start, &increment, &count)) ||
				((major | minor | start | count | increment) > 0xfffff))
		{
			if (*line=='\0' || *line=='#' || isspace(*line))
				continue;
			bb_error_msg("line %d invalid: '%s'\n", linenum, line);
			ret = EXIT_FAILURE;
			continue;
		}
		if (name[0] == '#') {
			continue;
		}
		if (*group) {
			gid = get_ug_id(group, my_getgrnam);
		} else {
			gid = getgid();
		}
		if (*user) {
			uid = get_ug_id(user, my_getpwnam);
		} else {
			uid = getuid();
		}

		/*
		 * free previous full name
		 * we don't de-allocate full_name at the end of the parsing,
		 * because we may need it if the next line is an xattr.
		 */
		free(full_name);
		full_name = concat_path_file(rootdir, name);

		if (type == 'd') {
			bb_make_directory(full_name, mode | S_IFDIR, FILEUTILS_RECUR);
			if (chown(full_name, uid, gid) == -1) {
				bb_perror_msg("line %d: chown failed for %s", linenum, full_name);
				ret = EXIT_FAILURE;
				goto loop;
			}
			if ((mode != -1) && (chmod(full_name, mode) < 0)){
				bb_perror_msg("line %d: chmod failed for %s", linenum, full_name);
				ret = EXIT_FAILURE;
				goto loop;
			}
		} else if (type == 'f' || type == 'F') {
			struct stat st;
			if ((stat(full_name, &st) < 0 || !S_ISREG(st.st_mode))) {
				if (type == 'F') {
					continue; /*Ignore optional files*/
				}
				bb_perror_msg("line %d: regular file '%s' does not exist", linenum, full_name);
				ret = EXIT_FAILURE;
				goto loop;
			}
			if (chown(full_name, uid, gid) == -1) {
				bb_perror_msg("line %d: chown failed for %s", linenum, full_name);
				ret = EXIT_FAILURE;
				goto loop;
			}
			if ((mode != -1) && (chmod(full_name, mode) < 0)){
				bb_perror_msg("line %d: chmod failed for %s", linenum, full_name);
				ret = EXIT_FAILURE;
				goto loop;
			}
		} else if (type == 'r') {
			recursive_uid = uid;
			recursive_gid = gid;
			recursive_mode = mode;
			if (nftw(full_name, bb_recursive, 20, FTW_MOUNT | FTW_PHYS) < 0) {
				bb_perror_msg("line %d: recursive failed for %s", linenum, full_name);
				ret = EXIT_FAILURE;
				goto loop;
			}
		} else
		{
			dev_t rdev;
			unsigned i;
			char *full_name_inc;

			if (type == 'p') {
				mode |= S_IFIFO;
			}
			else if (type == 'c') {
				mode |= S_IFCHR;
			}
			else if (type == 'b') {
				mode |= S_IFBLK;
			} else {
				bb_error_msg("line %d: Unsupported file type %c", linenum, type);
				ret = EXIT_FAILURE;
				goto loop;
			}

			full_name_inc = xmalloc(strlen(full_name) + sizeof(int)*3 + 2);
			if (count)
				count--;
			for (i = start; i <= start + count; i++) {
				sprintf(full_name_inc, count ? "%s%u" : "%s", full_name, i);
				rdev = makedev(major, minor + (i - start) * increment);
				if (mknod(full_name_inc, mode, rdev) < 0) {
					bb_perror_msg("line %d: can't create node %s", linenum, full_name_inc);
					ret = EXIT_FAILURE;
				} else if (chown(full_name_inc, uid, gid) < 0) {
					bb_perror_msg("line %d: can't chown %s", linenum, full_name_inc);
					ret = EXIT_FAILURE;
				} else if (chmod(full_name_inc, mode) < 0) {
					bb_perror_msg("line %d: can't chmod %s", linenum, full_name_inc);
					ret = EXIT_FAILURE;
				}
			}
			free(full_name_inc);
		}
loop:
		free(line);
	}
	fclose(table);

	return ret;
}
