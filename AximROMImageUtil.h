/*************************************************************************
File encoding: UTF-8.

Модуль для дешифрования образов официальных прошивок из формата IMG в формат NB0.
Автор: dynamite, 2013...2015.

За основу взята утилита "Dell IMG to NB0" ( (C) Mark Young, 2006).
*************************************************************************/

#ifndef _AXIM_ROM_IMAGE_UTIL_
#define _AXIM_ROM_IMAGE_UTIL_

#include <QtCore>

class AximROMDecryptionProcessing : public QObject
{
    Q_OBJECT

private:
    QFile m_imageFile;
    QFile m_osNb0File;
    QFile m_logFile;
    bool m_verboseLogging;

    /************************************************************************************************************************************************************************/
    // STRINGS TO TRANSLATE: BEGIN
    /************************************************************************************************************************************************************************/
    QString msgPatternCreationArchiveBegin;
    QString msgPatternCreationArchiveFinished;
    QString msgPatternAddingEntry;

    QString msgPatternExtractionArchiveBegin;
    QString msgPatternExtractionArchiveFinished;

    QString msgPatternExtractionFileBegin;
    QString msgPatternExtractionFileFinished;

    QString msgPatternExtractingEntry;

    QString msgPatternSearchEntriesBegin;
    QString msgPatternSearchEntriesFinished;
    QString msgPatternEntryWasFound;

    QString msgDeflate;
    QString msgStore;
    QString msgDirectory;
    QString msgFile;
    QString msgFail;
    QString msgDone;
    /************************************************************************************************************************************************************************/
    // STRINGS TO TRANSLATE: END
    /************************************************************************************************************************************************************************/

private slots:
    void m_decryptionProcessStarted();

public:
    explicit AximROMDecryptionProcessing(QObject *parent = 0);
    ~AximROMDecryptionProcessing();

    //инициализация дешифрования файла с ОС
    bool initDecryption(const QString &imageFilePath, const QString &nb0FilePath, const bool verboseLogging);

signals:
    void signalOsNb0FileDecryptIsComplete(bool success);
    void logOutputSignal(bool errorMsg, QString msg);
};

class AximROMImageUtil : public QObject
{
    Q_OBJECT
private:
    AximROMDecryptionProcessing *m_decryptor;
    QString m_imageFilePath;
    QString m_osNb0FilePath;

public:
    explicit AximROMImageUtil(QObject *parent = 0);
    ~AximROMImageUtil();

signals:
    void signalOsNb0FileDecryptIsComplete(bool success);
    void logOutputSignal(bool errorMsg, QString msg);

public slots:
    QString imageFile() const;
    void setImageFile(const QString &file);

    QString osNb0File() const;

    bool decryptOsNb0File(const bool verboseLogging);
};

#endif // _AXIM_ROM_IMAGE_UTIL_
