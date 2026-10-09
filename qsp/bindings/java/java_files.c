#include "../../declarations.h"

#ifdef _JAVA_BINDING

#include "../../errors.h"
#include "../../game.h"

QSP_BOOL qspOpenQuestFromFILE(FILE *f, const QSP_BOOL isNewGame)
{
    fseek(f, 0, SEEK_END);
    const int fileSize = ftell(f);

    char *buf = malloc(fileSize);
    if (buf == NULL) return QSP_FALSE;

    fseek(f, 0, SEEK_SET);
    fread(buf, 1, fileSize, f);

    QSP_BOOL res = qspOpenGame(buf, fileSize, isNewGame);
    free(buf);

    return res;
}

QSP_BOOL qspOpenGameStatusFromFILE(FILE *f)
{
    fseek(f, 0, SEEK_END);
    const int fileSize = ftell(f);

    char *buf = malloc(fileSize);
    if (buf == NULL) return QSP_FALSE;

    fseek(f, 0, SEEK_SET);
    fread(buf, 1, fileSize, f);

    QSP_BOOL res = qspOpenGameStatus(buf, fileSize);
    free(buf);

    return res;
}

QSP_BOOL qspSaveGameStatusToFILE(FILE *f)
{
    void *dataBuf;
    int dataBufSize = 64 * 1024;
    QSP_BOOL res = QSP_FALSE;

    dataBuf = malloc(dataBufSize);
    if (dataBuf == NULL) return QSP_FALSE;

    while (1)
    {
        if (qspSaveGameStatus(dataBuf, &dataBufSize, QSP_TRUE))
        {
            res = QSP_TRUE;
            break;
        }
        if (!dataBufSize)
        {
            free(dataBuf);
            return QSP_FALSE;
        }
        dataBufSize += QSP_SAVEDGAMEDATAEXTRASPACE;
        dataBuf = realloc(dataBuf, dataBufSize);
    }

    if (res)
    {
        fwrite(dataBuf, 1, dataBufSize, f);
    }

    free(dataBuf);
    return res;
}

#endif