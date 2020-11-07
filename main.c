#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(push, 1)

typedef struct _ARCHIVE_HEADER {
    uint16_t magic;
    uint16_t version;
} ARCHIVE_HEADER, *PARCHIVE_HEADER;

typedef struct _FILE_DESC {
    char name[256];
    uint32_t size;
    uint32_t dataOffset;
} FILE_DESC, *PFILE_DESC;

#pragma pack(pop)

typedef struct _ARCHIVE {
    ARCHIVE_HEADER header;
    FILE *fp;
} ARCHIVE, *PARCHIVE;

#define ARCHIVE_NAME "archive.bin"

uint32_t getFileSize(FILE *fp) 
{
    uint32_t size;
    uint32_t currPos = ftell(fp);

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);

    fseek(fp, currPos, SEEK_SET);

    return size;
}

int append(PARCHIVE archive, char *filename)
{
    int ret = 0;

    FILE *fp = fopen(filename, "rb");

    if(fp == NULL)
    {
        printf("%s 파일이 없습니다.\n", filename);
        return -1;
    }

    uint8_t *buffer;
    uint32_t size;

    size = getFileSize(fp);
    buffer = malloc(size);

    if(fread(buffer, size, 1, fp) < 1)
    {
        printf("%s 파일 읽기 실패\n", filename);
        ret = -1;
        goto Error1;
    }

    PFILE_DESC desc = malloc(sizeof(FILE_DESC));
    memset(desc, 0, sizeof(FILE_DESC));
    strcpy(desc->name, filename);
    desc->size = size;

    fseek(archive->fp, sizeof(ARCHIVE_HEADER), SEEK_SET);

    desc->dataOffset = ftell(archive->fp) + sizeof(FILE_DESC);

    if(fwrite(desc, sizeof(FILE_DESC), 1, archive->fp) < 1)
    {
        printf("파일 정보 쓰기 실패\n");
        ret = -1;
        goto Error2;
    }

    if(fwrite(buffer, size, 1, archive->fp) < 1)
    {
        printf("파일 데이터 쓰기 실패\n");
        ret = -1;
        goto Error2;
    }

    printf("%s 파일 추가 성공\n크기 : %d\n", filename, size);

Error2:
    free(desc);

Error1:
    free(buffer);
    fclose(fp);

    return ret;
}

int main()
{
    PARCHIVE archive = malloc(sizeof(ARCHIVE));
    memset(archive, 0, sizeof(ARCHIVE));

    FILE *fp = fopen(ARCHIVE_NAME, "r+b");
    if(fp == NULL)
    {
        fp = fopen(ARCHIVE_NAME, "w+b");

        if(fp == NULL)
            return -1;
        
        archive->header.magic = 'AF';
        archive->header.version = 1;

        if(fwrite(&archive->header, sizeof(ARCHIVE_HEADER), 1, fp) < 1)
        {
            printf("아카이브 헤더 쓰기 실패\n");
            fclose(fp);
            return -1;
        }

        archive->fp = fp;
        append(archive, "hello.txt");
    }

    fclose(fp);
    free(archive);

    return 0;
}