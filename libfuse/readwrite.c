#include <string.h>
#include <unistd.h>
#include <assert.h>
#include "../include/libtfs.h"

int read_from_block(blockno_t block, offset_t offset, char *buffer, int bytes)
{
    START("read_from_block");
    LOG("block:%d,offset:%d,bytes:%d",block,offset,bytes);
    if (block == -1)
    {
      printf("\nFile overread");
      return -1;
    }
    numassert(block >= 0 && block <= NUM_BLOCKS, block);
    numassert(offset >= 0, offset);
    assertd(buffer != NULL);
    numassert(bytes >= 0, bytes);

    //seek to the correct block
    while (offset > BLOCKSIZE)
    {
        offset -= BLOCKSIZE;
        block = get_next_block(block);
    }
    // start reading data
    FILE *handle;
    while (bytes > 0)
    {
        handle = get_data_handle(block, offset);
        LOG("read_from_block : handle : %d",handle);
        offset = 0;
        for (int i = 0; i < BLOCKSIZE && bytes > 0; i++)
        {
            *(buffer++) = fgetc(handle);
            LOG("buffer : %s",buffer);
            bytes--;
        }
        fclose(handle);
        block = get_next_block(block);
        if (block==-1)
        {
          return 0;
        }
    }
    return 0;

    END("read_from_block");
}

//auto-skips if offset and/or (offset+bytes) is greater than block size.
int write_to_block(blockno_t block, offset_t offset, char *buffer, int bytes)
{
    START("write_to_block");

    numassert(block >= 0 && block < NUM_BLOCKS, block);
    numassert(offset >= 0, offset);
    assertd(buffer != NULL);
    numassert(bytes > 0, bytes);

    //seek to the correct block
    while (offset > BLOCKSIZE)
    {
        offset -= BLOCKSIZE;
        block = get_next_block(block);
        if (-1 == block)
        {
            printf("\nFile overread");
            return -1;
        }
    }
    // start writing data
    FILE *handle;
    while (bytes > 0)
    {
        handle = get_data_handle(block, offset);
        offset = 0;
        LOG("%s",handle);
        for (int i = 0; i < BLOCKSIZE && bytes > 0; i++)
        {
            fputc(*buffer++, handle);
            bytes--;
        }
        fclose(handle);
        block = get_or_create_next_block(block);
    }
    return 0;

    END("write_to_block");
}

int read_from_path(char *path, offset_t offset, char *buffer, int bytes)
{
    assertd(path != NULL);
    numassert(offset >= 0, offset);
    assertd(buffer != NULL);
    numassert(bytes >= 0, bytes);

    START("read_from_path");
    return read_from_block(get_first_block_from_path(path), offset, buffer, bytes);

    END("read_from_path");
}

int write_to_path(char *path, offset_t offset, char *buffer, int bytes)
{
    START("write_to_path");

    assertd(path != NULL);
    numassert(offset > 0, offset);
    assertd(buffer != NULL);
    numassert(bytes >= 0, bytes);

    return write_to_block(get_first_block_from_path(path), offset, buffer, bytes);

    END("write_to_path");
}

blockno_t get_first_block_from_path(char *path)
{
    //LOG("Path : %s ; iNode : %d",path,);
    int j;
    assertd(path != NULL);
    START("get_first_block_from_path");
    LOG("%s",path);
    for (inode_t inode = 0; inode < NUM_FILES; inode++)
    {
        char fpath[10];
        fpath[0] = '/';
        strcat(fpath,files[inode].path);
        char* fpath2 = fpath;
        LOG("%s",fpath);
        if (0 == strcmp(path, fpath2))
        {
            return get_first_block_from_inode(inode);
        }
        for(j=0; j<10; j++)
        {
          fpath[j]='\0';
        }
    }
    END("get_first_block_from_path");
    return -1;
}
