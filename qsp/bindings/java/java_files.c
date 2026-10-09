#include "../../declarations.h"

#ifdef _JAVA_BINDING

#include "../../errors.h"
#include "../../game.h"

QSP_BOOL qspOpenQuestFromFILE(FILE *f, QSP_BOOL isNewGame)
{
    QSP_BOOL res;
    void *buf;
    int fileSize;
    fseek(f, 0, SEEK_END);
    fileSize = ftell(f);
    buf = malloc(fileSize);
    if (!buf) return QSP_FALSE;
    fseek(f, 0, SEEK_SET);
    fread(buf, 1, fileSize, f);
    res = qspOpenGame(buf, fileSize, isNewGame);
    free(buf);
    return res;
}

QSP_BOOL qspOpenGameStatusFromFILE(FILE *f)
{
    QSP_BOOL res;
    void *buf;
    int fileSize;
    fseek(f, 0, SEEK_END);
    fileSize = ftell(f);
    buf = malloc(fileSize);
    if (!buf) return QSP_FALSE;
    fseek(f, 0, SEEK_SET);
    fread(buf, 1, fileSize, f);
    res = qspOpenGameStatus(buf, fileSize);
    free(buf);
    return res;
}

QSP_BOOL qspSaveGameStatusToFILE(FILE *f)
{
    int dataBufSize = 64 * 1024;
    void *newBuf, *dataBuf = malloc(dataBufSize);
    if (!dataBuf) return QSP_FALSE;
    while (1)
    {
        if (qspSaveGameStatus(dataBuf, &dataBufSize, QSP_TRUE))
            break;
        if (!dataBufSize)
        {
            free(dataBuf);
            return QSP_FALSE;
        }
        dataBufSize += QSP_SAVEDGAMEDATAEXTRASPACE;
        if (!(newBuf = realloc(dataBuf, dataBufSize)))
        {
            free(dataBuf);
            return QSP_FALSE;
        }
        dataBuf = newBuf;
    }
    fwrite(dataBuf, 1, dataBufSize, f);
    free(dataBuf);
    return QSP_TRUE;
}

#endif
