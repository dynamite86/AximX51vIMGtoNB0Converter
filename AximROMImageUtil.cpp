/*************************************************************************
File encoding: UTF-8.

Модуль для дешифрования образов официальных прошивок из формата IMG в формат NB0.
Автор: dynamite, 2013...2015.

За основу взята утилита "Dell IMG to NB0" ( (C) Mark Young, 2006).
*************************************************************************/


#include "AximROMImageUtil.h"

#define DATA_BLOCK_SIZE  0x80
#define MAGIC_KEY_OFFSET 0x100

void AximROMDecryptionProcessing::m_decryptionProcessStarted()
{
    bool magicKeyWasFound      = true;
    uchar byte                 = 0;
    uint searchPos             = 0;
    uint blockStart            = 0; //начало блока данных
    uint blockPos              = 0; //позиция чтения внутри блока данных
    uint nb0Length             = 0; //длина файла NB0
    qulonglong endOfFile       = 0; //конец файла IMG
    uint firstSubblockPos      = 0; //позиция первого зашифрованного под-блока данных в половине прошивки
    uint secondSubblockPos     = 0; //позиция второго зашифрованного под-блока данных в половине прошивки
    uint dataBlocksCountInPart = 0; //количество блоков данных в половине файла прошивки

    uchar magicKey[DATA_BLOCK_SIZE];
    uchar encryptedDataBlock1[DATA_BLOCK_SIZE];
    uchar encryptedDataBlock2[DATA_BLOCK_SIZE];
    uchar decryptedDataBlock[DATA_BLOCK_SIZE];

    memset(magicKey,            0x00, DATA_BLOCK_SIZE);
    memset(encryptedDataBlock1, 0x00, DATA_BLOCK_SIZE);
    memset(encryptedDataBlock2, 0x00, DATA_BLOCK_SIZE);
    memset(decryptedDataBlock,  0x00, DATA_BLOCK_SIZE);

    m_imageFile.open(QIODevice::ReadOnly);
    m_imageFile.seek(m_imageFile.size() - MAGIC_KEY_OFFSET);
    m_imageFile.read((char*)&magicKey, DATA_BLOCK_SIZE);
    m_imageFile.seek(0);

    //подготовка к обработке
    endOfFile = m_imageFile.size() - 1;
    searchPos = 0;

    magicKey[1] = magicKey[1]^0xFD;

    logOutputSignal(false, QString("Source img file: \'%1\'").arg(QFileInfo(m_imageFile).absoluteFilePath()));
    logOutputSignal(false, QString("Output nb0 file: \'%1\'").arg(QFileInfo(m_osNb0File).absoluteFilePath()));
    logOutputSignal(false, QString("----------"));
    logOutputSignal(false, QTime::currentTime().toString("[hh:mm:ss:zzz] ") + QString("Search signature aka Magic Key of ROM is started...\n"));
    while(searchPos <= endOfFile)
    {
        m_imageFile.read((char*)&byte, sizeof(uchar));
        if(byte == magicKey[0])
        {
            blockStart       = m_imageFile.pos();
            magicKeyWasFound = true;
            blockPos         = 1;
            do
            {
                m_imageFile.read((char*)&byte, sizeof(uchar));
                if(byte != magicKey[blockPos])
                {
                    magicKeyWasFound = false;
                    break;
                }
                blockPos++;
            }
            while(blockPos <= 0x7F);

            if(magicKeyWasFound)
            {
                nb0Length = m_imageFile.size() - blockStart + 1;

                logOutputSignal(false, QString().sprintf("WOW! Magic Key was found at 0x%08X! O.o", (uint)m_imageFile.size() - nb0Length));
                logOutputSignal(false, QString().sprintf("Length of ROM is %d (0x%08X) bytes.", nb0Length, nb0Length));
                logOutputSignal(false, QTime::currentTime().toString("[hh:mm:ss:zzz] ") + QString("Search signature aka Magic Key of ROM is completed successfully!"));
                break;
            }
            m_imageFile.seek(blockStart);
        }
        searchPos++;
    }

    if((searchPos == endOfFile)&&(!magicKeyWasFound))
    {
        logOutputSignal(true, QTime::currentTime().toString("[hh:mm:ss:zzz] ") + QString("Search signature aka Magic Key of ROM is failed! Aborting!"));
        signalOsNb0FileDecryptIsComplete(false);
    }
    else
    {
        logOutputSignal(false, QTime::currentTime().toString("[hh:mm:ss:zzz] ") + QString("Decryption ROM is started..."));
        magicKey[1] = magicKey[1]^0xFD;

        m_osNb0File.open(QIODevice::WriteOnly);

        dataBlocksCountInPart = (uint)round((double)((((double)nb0Length)/256.0)-1.0));
        if(m_verboseLogging)
        {
            logOutputSignal(false, QString().sprintf("The count encrypted data blocks in two parts of ROM is %d", dataBlocksCountInPart));
        }

        firstSubblockPos = m_imageFile.size() - nb0Length;
        secondSubblockPos   = m_imageFile.size() - DATA_BLOCK_SIZE;

        if(m_verboseLogging)
        {
            logOutputSignal(false, QTime::currentTime().toString("[hh:mm:ss:zzz] ") + QString("++++ Decryption of the first half of ROM ++++"));
        }

        for(uint currentDataBlock = 0; currentDataBlock <= dataBlocksCountInPart; currentDataBlock++)
        {
            if(m_verboseLogging)
            {
                logOutputSignal(false, QString().sprintf("    * reading first sub-block data at offset 0x%08X;", firstSubblockPos));
            }
            m_imageFile.seek(firstSubblockPos);
            m_imageFile.read((char*)&encryptedDataBlock1, DATA_BLOCK_SIZE);

            if(m_verboseLogging)
            {
                logOutputSignal(false, QString().sprintf("    * reading second sub-block data at offset 0x%08X;", secondSubblockPos));
            }
            m_imageFile.seek(secondSubblockPos);
            m_imageFile.read((char*)&encryptedDataBlock2, DATA_BLOCK_SIZE);

            if(m_verboseLogging)
            {
                logOutputSignal(false, QString().sprintf("    * decrypting the data block #%d;", currentDataBlock+1));
            }

            blockPos = 0;
            do
            {
                decryptedDataBlock[blockPos] = encryptedDataBlock2[0x7F-blockPos]^magicKey[0x7F-blockPos];
                blockPos++;
                decryptedDataBlock[blockPos] = encryptedDataBlock1[blockPos]^magicKey[blockPos];
                blockPos++;
            }
            while(blockPos <= 0x7F);
            secondSubblockPos -= 0x80;
            firstSubblockPos  += 0x80;
            m_osNb0File.write((char*)decryptedDataBlock, DATA_BLOCK_SIZE);

            if(m_verboseLogging)
            {
                logOutputSignal(false, QString().sprintf("    * writing decrypted data block #%d.", currentDataBlock+1));
                logOutputSignal(false, QString("-----------------"));
            }

        }

        if(m_verboseLogging)
        {
            logOutputSignal(false, QString("---------------------------------------------------"));
            logOutputSignal(false, QTime::currentTime().toString("[hh:mm:ss:zzz] ") + QString("++++ Decryption of the second half of ROM ++++"));

        }

        for(uint currentDataBlock = 0; currentDataBlock <= dataBlocksCountInPart; currentDataBlock++)
        {
            if(m_verboseLogging)
            {
                logOutputSignal(false, QString().sprintf("    * reading first sub-block data at offset 0x%08X;", firstSubblockPos));
            }
            m_imageFile.seek(firstSubblockPos);
            m_imageFile.read((char*)&encryptedDataBlock1, DATA_BLOCK_SIZE);

            if(m_verboseLogging)
            {
                logOutputSignal(false, QString().sprintf("    * reading second sub-block data at offset 0x%08X;", secondSubblockPos));
            }
            m_imageFile.seek(secondSubblockPos);
            m_imageFile.read((char*)&encryptedDataBlock2, DATA_BLOCK_SIZE);

            if(m_verboseLogging)
            {
                logOutputSignal(false, QString().sprintf("    * decrypting the data block #%d;", currentDataBlock+1));
            }
            blockPos = 0;
            do
            {
                decryptedDataBlock[blockPos] = 0xFF - (encryptedDataBlock1[blockPos]^magicKey[blockPos]);
                blockPos++;
                decryptedDataBlock[blockPos] = 0xFF - (encryptedDataBlock2[0x7F-blockPos]^magicKey[0x7F-blockPos]);
                blockPos++;
            }
            while(blockPos <= 0x7F);
            secondSubblockPos -= 0x80;
            firstSubblockPos  += 0x80;

            m_osNb0File.write((char*)decryptedDataBlock, DATA_BLOCK_SIZE);
            if(m_verboseLogging)
            {
                logOutputSignal(false, QString().sprintf("    * writing decrypted data block #%d.", currentDataBlock+1));
                logOutputSignal(false, QString("-----------------"));
            }
        }
        if(m_verboseLogging)
        {
            logOutputSignal(false, QString("---------------------------------------------------"));
        }
        logOutputSignal(false, QString("ROM was saved in \'%1\'").arg(QFileInfo(m_osNb0File).absoluteFilePath()));
        logOutputSignal(false, QTime::currentTime().toString("[hh:mm:ss:zzz] ") + QString("Decryption completed successfully!"));

    }
    m_imageFile.close();
    m_osNb0File.close();
    m_logFile.close();
    signalOsNb0FileDecryptIsComplete(true);
}

AximROMDecryptionProcessing::AximROMDecryptionProcessing(QObject *parent)
{
    Q_UNUSED(parent);
}

AximROMDecryptionProcessing::~AximROMDecryptionProcessing(){}

bool AximROMDecryptionProcessing::initDecryption(const QString &imageFilePath, const QString &nb0FilePath, const bool verboseLogging)
{
    if(imageFilePath.isEmpty())
    {
        return false;
    }
    if(nb0FilePath.isEmpty())
    {
        return false;
    }

    m_imageFile.setFileName(imageFilePath);
    m_osNb0File.setFileName(nb0FilePath);
    m_verboseLogging = verboseLogging;
    return true;
}
/**************************************************************************************************************************************************************/
AximROMImageUtil::AximROMImageUtil(QObject *parent) : QObject(parent){}
AximROMImageUtil::~AximROMImageUtil(){}

QString AximROMImageUtil::imageFile() const
{
    return m_imageFilePath;
}

void AximROMImageUtil::setImageFile(const QString &file)
{
    const QString patternOSNb0Filename    = "DiAd_K_%1.nb0";
//    const QString patternEbootNb0Filename = "DiAd_B_%1.nb0";
    if(file.compare(m_imageFilePath, Qt::CaseInsensitive) != 0)
    {
        m_imageFilePath = file;
        m_osNb0FilePath = QDir::toNativeSeparators(QFileInfo(file).absoluteDir().absolutePath()) + QDir::separator() + patternOSNb0Filename.arg(QFileInfo(file).baseName());
    }
}

QString AximROMImageUtil::osNb0File() const
{
    return m_osNb0FilePath;
}

bool AximROMImageUtil::decryptOsNb0File(const bool verboseLogging)
{
    m_decryptor = new AximROMDecryptionProcessing(0);
    if(!m_decryptor->initDecryption(m_imageFilePath, m_osNb0FilePath, verboseLogging))
    {
        delete m_decryptor;
        return false;
    }
    QThread *threadForProcessing = new QThread;
    m_decryptor->moveToThread(threadForProcessing);

    connect(threadForProcessing, SIGNAL(started()),                              m_decryptor,         SLOT(m_decryptionProcessStarted()));
    connect(threadForProcessing, SIGNAL(finished()),                             threadForProcessing, SLOT(deleteLater()));
    connect(m_decryptor,         SIGNAL(signalOsNb0FileDecryptIsComplete(bool)), m_decryptor,         SLOT(deleteLater()));
    connect(m_decryptor,         SIGNAL(signalOsNb0FileDecryptIsComplete(bool)), threadForProcessing, SLOT(quit()));
    connect(m_decryptor,         SIGNAL(signalOsNb0FileDecryptIsComplete(bool)), this,                SIGNAL(signalOsNb0FileDecryptIsComplete(bool)));
    connect(m_decryptor,         SIGNAL(logOutputSignal(bool,QString)),          this,                SIGNAL(logOutputSignal(bool,QString)));

    threadForProcessing->start();
    return true;
}
