#include "EncryptData.h"

#ifdef _WIN32

// 加密
QByteArray EncryptWithWindowsDPAPI(const QByteArray& data)
{
    DATA_BLOB inBlob;
    inBlob.pbData = (BYTE*)data.data();
    inBlob.cbData = data.size();

    DATA_BLOB outBlob;
    if (CryptProtectData(&inBlob, L"Password", nullptr, nullptr, nullptr, 0, &outBlob)) {
        QByteArray result((char*)outBlob.pbData, outBlob.cbData);
        LocalFree(outBlob.pbData);
        return result;
    } else {
        return {};
    }
}

// 解密
QByteArray DecryptWithWindowsDPAPI(const QByteArray& encryptedData)
{
    DATA_BLOB inBlob;
    inBlob.pbData = (BYTE*)encryptedData.data();
    inBlob.cbData = encryptedData.size();

    DATA_BLOB outBlob;
    if (CryptUnprotectData(&inBlob, nullptr, nullptr, nullptr, nullptr, 0, &outBlob)) {
        QByteArray result((char*)outBlob.pbData, outBlob.cbData);
        LocalFree(outBlob.pbData);
        return result;
    } else {
        return {};
    }
}

#else
QByteArray SimpleEncrypt(const QByteArray& data, const QByteArray& key = "SimpleKey")
{
    QByteArray ba = data;
    for (int i = 0; i < ba.size(); ++i){
        ba[i] = ba[i] ^ key[i % key.size()];
    }
    return ba;
}

QByteArray SimpleDecrypt(const QByteArray& data, const QByteArray& key = "SimpleKey")
{
    QByteArray ba = data;
    for (int i = 0; i < ba.size(); ++i){
        ba[i] = ba[i] ^ key[i % key.size()];
    }
    return ba;
}
#endif

// 包装为字符串操作
QString EncryptString(const QString& password)
{
    #ifdef _WIN32
    QByteArray encrypted = EncryptWithWindowsDPAPI(password.toUtf8());
    #else
    QByteArray encrypted = SimpleEncrypt(password.toUtf8());
    #endif
    return encrypted.toBase64();
}

QString DecryptString(const QString& base64EncodedPassword)
{
    QByteArray decoded = QByteArray::fromBase64(base64EncodedPassword.toUtf8());
    #ifdef _WIN32
    QByteArray decrypted = DecryptWithWindowsDPAPI(decoded);
    #else
    QByteArray decrypted = SimpleDecrypt(decoded);
    #endif
    return QString::fromUtf8(decrypted);
}
