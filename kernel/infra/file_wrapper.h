#ifndef LANGSU_FILE_WRAPPER_H
#define LANGSU_FILE_WRAPPER_H

#include <linux/file.h>
#include <linux/fs.h>

int langsu_install_file_wrapper(int fd);
void langsu_file_wrapper_init(void);

#endif // LANGSU_FILE_WRAPPER_H
