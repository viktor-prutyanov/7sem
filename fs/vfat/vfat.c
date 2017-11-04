#define _GNU_SOURCE

#include <stdio.h>
#include <linux/msdos_fs.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <wchar.h>
#include <stdlib.h>

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

#ifdef DEBUG
#define pr_debug(...) eprintf(__VA_ARGS__)
#else
#define pr_debug(...) do {} while (0)
#endif

#define LAST_SLOT_ID_MASK 0x40

struct vfat_info
{
    off_t root_start;
};

static void *map;

uint8_t id_if_dir_slot(void *start)
{
    struct msdos_dir_slot *ds = (struct msdos_dir_slot *)start;
    uint8_t id = ds->id;
    if (id & LAST_SLOT_ID_MASK)
        id = id & ~LAST_SLOT_ID_MASK;
    return ((ds->attr == 0x0f) && !ds->start) ? id : 0;
}

void utf16_to_wchar(uint16_t *utf16_str, wchar_t *wstr)
{
    do {
        *wstr++ = *utf16_str;
    } while (*utf16_str++);
}

void *process_next_dir_slot(void *start, void *utf16_name, uint8_t *id)
{
    struct msdos_dir_slot *ds = (struct msdos_dir_slot *)start;
    *id = ds->id;
    
    utf16_name = mempcpy(utf16_name, ds->name0_4, sizeof(ds->name0_4));
    utf16_name = mempcpy(utf16_name, ds->name5_10, sizeof(ds->name5_10));
    mempcpy(utf16_name, ds->name11_12, sizeof(ds->name11_12));
    
    return (start + sizeof(*ds));
}

void *process_next_dir_entry(void *start, uint8_t *attr, char *raw_sfn)
{
    struct msdos_dir_entry *de = (struct msdos_dir_entry *)start;
    
    memcpy(raw_sfn, de->name, MSDOS_NAME);

    *attr = de->attr;

    return (start + sizeof(*de));
}

void *get_next_name(void *start, wchar_t *lfn, char *sfn, uint8_t *attr, int *lname)
{
    uint16_t utf16_name[MSDOS_LONGNAME] = {0};
    char raw_sfn[MSDOS_NAME] = {0};
    uint8_t id = id_if_dir_slot(start);

    pr_debug("offset = 0x%lx\n", start - map);
    if (id) {
        ++id;
        *lname = 1;
        
        do {
            start = process_next_dir_slot(start, utf16_name + 13 * (id - 2), &id);
            if (id & LAST_SLOT_ID_MASK) {
                id = id & ~LAST_SLOT_ID_MASK;
                pr_debug("last ");
            }
            pr_debug("dir_slot with id = %u\n", id);
        } while (id != 1);

        utf16_to_wchar(utf16_name, lfn);
    } else {
        *lname = 0;
    }
   
    start = process_next_dir_entry(start, attr, raw_sfn);

    if (*attr & ATTR_DIR) {
        sfn = memccpy(sfn, raw_sfn, 0x20, MSDOS_NAME); 
        if (sfn)
            *(sfn - 1) = '\0';
    } else {
        memcpy(sfn, raw_sfn, 8); 
        sfn[8] = '.';
        memcpy(sfn + 9, raw_sfn + 8, 3);
        sfn[MSDOS_NAME + 1] = '\0';
    }

    return start;
}

void get_vfat_info(void *map, struct vfat_info *vi)
{
    struct fat_boot_sector *bs = (struct fat_boot_sector *)map;
    uint16_t sector_size = *(uint16_t *)bs->sector_size;
    vi->root_start = (bs->reserved + bs->fats * bs->fat_length) * sector_size;
    pr_debug("Root starts at 0x%zx\n", vi->root_start);
}

size_t get_file_size(int fd)
{
    struct stat st;
    
    errno = 0;
        
    if (fstat(fd, &st))
        return 0;
    else
        return (size_t)st.st_size;
}

int main(int argc, char *argv[])
{
    int err = 0;

    if (argc != 2) {
        eprintf("Usage: %s map\n", argv[0]);
        return -1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        eprintf("open: %s\n", strerror(errno));
        return -1;
    }
   
    size_t size = get_file_size(fd);
    if (!size) {
        if (!errno) {
            eprintf("Empty map\n");
        } else {
            eprintf("stat: %s\n", strerror(errno));
            err = -1;
        }
        goto out_close;
    }
    
    map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (!map) {
        eprintf("mmap: %s\n", strerror(errno));
        err = -1;
        goto out_close;
    }

    pr_debug("File '%s' (%zuB) has been opened and mmaped at %p\n", argv[1], size, map);

    struct vfat_info vi;
    get_vfat_info(map, &vi);

    void *start = map + vi.root_start;

    wchar_t lfn[MSDOS_LONGNAME] = {0};
    char sfn[MSDOS_NAME + 2] = {0}; //one byte for '.' and one for '\0'
    uint8_t attr;
    int lname;
    while (*(uint8_t *)start) {
        start = get_next_name(start, lfn, sfn, &attr, &lname);
        
        if (lname)
            wprintf(L"%S", lfn);
        else
            wprintf(L"%s", sfn);
        
        if (attr & ATTR_DIR)
            wprintf(L"\\");

        wprintf(L"\n");
    }

out_munmap:
    munmap(map, size);
out_close:
    close(fd);
    return 0;
}
