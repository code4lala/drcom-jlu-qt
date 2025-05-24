#ifndef ENCRYPTDATA_H
#define ENCRYPTDATA_H

#include <QString>
#include <QByteArray>

#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
#endif

#ifdef _WIN32
QByteArray EncryptWithWindowsDPAPI(const QByteArray& data);
QByteArray DecryptWithWindowsDPAPI(const QByteArray& encryptedData);
#else
QByteArray SimpleEncrypt(const QByteArray& data, const QByteArray& key = "SimpleKey");
QByteArray SimpleDecrypt(const QByteArray& data, const QByteArray& key = "SimpleKey");
#endif

QString EncryptString(const QString& password);
QString DecryptString(const QString& base64EncodedPassword);

#endif // ENCRYPTDATA_H
