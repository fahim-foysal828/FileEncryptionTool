#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QCryptographicHash>
#include <openssl/evp.h>
#include <openssl/crypto.h>
#include <openssl/rand.h>




QByteArray vigenereEncrypt(QByteArray data, QString password)
{
    for (int i = 0; i < data.size(); i++)
    {
        int keyIndex = i % password.length();

        int shift = password[keyIndex].unicode() % 256;

        unsigned char byte =
            static_cast<unsigned char>(data[i]);

        byte = static_cast<unsigned char>(
            (byte + shift) % 256
            );

        data[i] = static_cast<char>(byte);
    }

    return data;
}




QByteArray vigenereDecrypt(QByteArray data, QString password)
{
    for (int i = 0; i < data.size(); i++)
    {
        int keyIndex = i % password.length();

        int shift = password[keyIndex].unicode() % 256;

        unsigned char byte =
            static_cast<unsigned char>(data[i]);

        byte = static_cast<unsigned char>(
            (byte - shift + 256) % 256
            );

        data[i] = static_cast<char>(byte);
    }

    return data;
}




QByteArray deriveAESKey(QString password, QByteArray salt)
{
    QByteArray key(16, 0);

    PKCS5_PBKDF2_HMAC(
        password.toUtf8().constData(),
        password.toUtf8().size(),
        reinterpret_cast<const unsigned char*>(salt.constData()),
        salt.size(),
        100000,
        EVP_sha256(),
        16,
        reinterpret_cast<unsigned char*>(key.data())
        );

    return key;
}




QByteArray generateRandomBytes(int size)
{
    QByteArray data(size, 0);

    if (RAND_bytes(
            reinterpret_cast<unsigned char*>(data.data()),
            size) != 1)
    {
        return QByteArray();
    }

    return data;
}



QByteArray aesEncrypt(QByteArray data, QByteArray key, QByteArray iv)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    if (ctx == nullptr)
    {
        return QByteArray();
    }

    if (EVP_EncryptInit_ex(
            ctx,
            EVP_aes_128_cbc(),
            nullptr,
            reinterpret_cast<const unsigned char*>(key.constData()),
            reinterpret_cast<const unsigned char*>(iv.constData())
            ) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }

    QByteArray encrypted(data.size() + EVP_CIPHER_block_size(EVP_aes_128_cbc()), 0);

    int len = 0;
    int finalLen = 0;

    if (EVP_EncryptUpdate(
            ctx,
            reinterpret_cast<unsigned char*>(encrypted.data()),
            &len,
            reinterpret_cast<const unsigned char*>(data.constData()),
            data.size()
            ) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }

    if (EVP_EncryptFinal_ex(
            ctx,
            reinterpret_cast<unsigned char*>(encrypted.data()) + len,
            &finalLen
            ) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }

    encrypted.resize(len + finalLen);

    EVP_CIPHER_CTX_free(ctx);

    return encrypted;
}



QByteArray aesDecrypt(QByteArray encryptedData, QByteArray key, QByteArray iv)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    if (ctx == nullptr)
    {
        return QByteArray();
    }

    if (EVP_DecryptInit_ex(
            ctx,
            EVP_aes_128_cbc(),
            nullptr,
            reinterpret_cast<const unsigned char*>(key.constData()),
            reinterpret_cast<const unsigned char*>(iv.constData())
            ) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }

    QByteArray decrypted(
        encryptedData.size() + EVP_CIPHER_block_size(EVP_aes_128_cbc()),
        0
        );

    int len = 0;
    int finalLen = 0;

    if (EVP_DecryptUpdate(
            ctx,
            reinterpret_cast<unsigned char*>(decrypted.data()),
            &len,
            reinterpret_cast<const unsigned char*>(encryptedData.constData()),
            encryptedData.size()
            ) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }

    if (EVP_DecryptFinal_ex(
            ctx,
            reinterpret_cast<unsigned char*>(decrypted.data()) + len,
            &finalLen
            ) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }

    decrypted.resize(len + finalLen);

    EVP_CIPHER_CTX_free(ctx);

    return decrypted;
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);




    connect(ui->browseButton, &QPushButton::clicked, this, [this]()
            {
                QString fileName = QFileDialog::getOpenFileName(
                    this,
                    "Select File"
                    );

                if (!fileName.isEmpty())
                {
                    ui->filePathEdit->setText(fileName);
                    ui->statusLabel->setText("File selected.");
                }
            });





    connect(ui->showPasswordCheckBox, &QCheckBox::toggled,
            this, [this](bool checked)
            {
                if (checked)
                {
                    ui->passwordEdit->setEchoMode(QLineEdit::Normal);
                }
                else
                {
                    ui->passwordEdit->setEchoMode(QLineEdit::Password);
                }
            });





    connect(ui->encryptButton, &QPushButton::clicked, this, [this]()
            {
                QString fileName = ui->filePathEdit->text();
                QString password = ui->passwordEdit->text();


                if (fileName.isEmpty())
                {
                    QMessageBox::warning(
                        this,
                        "Error",
                        "Please select a file first."
                        );
                    return;
                }


                if (password.isEmpty())
                {
                    QMessageBox::warning(
                        this,
                        "Error",
                        "Please enter a password."
                        );
                    return;
                }

                ui->statusLabel->setText("Encrypting...");

                QString algorithm =
                    ui->algorithmComboBox->currentText();




                int shift = 0;

                for (int i = 0; i < password.length(); i++)
                {
                    shift += password[i].unicode();
                }

                shift %= 256;




                QByteArray passwordHash =
                    QCryptographicHash::hash(
                        password.toUtf8(),
                        QCryptographicHash::Sha256
                        );





                QFile inputFile(fileName);

                if (!inputFile.open(QIODevice::ReadOnly))
                {
                    QMessageBox::warning(
                        this,
                        "Error",
                        "Could not open the file."
                        );

                    ui->statusLabel->setText(
                        "Encryption failed."
                        );

                    return;
                }




                if (algorithm == "AES-128")
                {

                    QByteArray originalData =
                        inputFile.readAll();

                    inputFile.close();



                    QByteArray salt =
                        generateRandomBytes(16);


                    QByteArray iv =
                        generateRandomBytes(16);


                    if (salt.size() != 16 || iv.size() != 16)
                    {
                        QMessageBox::warning(
                            this,
                            "Error",
                            "Could not generate secure encryption data."
                            );

                        ui->statusLabel->setText(
                            "Encryption failed."
                            );

                        return;
                    }


                    QByteArray key =
                        deriveAESKey(
                            password,
                            salt
                            );


                    if (key.size() != 16)
                    {
                        QMessageBox::warning(
                            this,
                            "Error",
                            "Could not create AES-128 key."
                            );

                        ui->statusLabel->setText(
                            "Encryption failed."
                            );

                        return;
                    }




                    QByteArray encryptedData =
                        aesEncrypt(
                            originalData,
                            key,
                            iv
                            );


                    if (encryptedData.isEmpty() &&
                        !originalData.isEmpty())
                    {
                        QMessageBox::warning(
                            this,
                            "Error",
                            "AES-128 encryption failed."
                            );

                        ui->statusLabel->setText(
                            "Encryption failed."
                            );

                        return;
                    }





                    QFileInfo fileInfo(fileName);

                    QString outputFileName =
                        fileInfo.path() + "/" +
                        fileInfo.completeBaseName() +
                        "_encrypted." +
                        fileInfo.suffix();


                    QFile outputFile(outputFileName);

                    if (!outputFile.open(QIODevice::WriteOnly))
                    {
                        QMessageBox::warning(
                            this,
                            "Error",
                            "Could not create encrypted file."
                            );

                        ui->statusLabel->setText(
                            "Encryption failed."
                            );

                        return;
                    }




                    if (outputFile.write(passwordHash) !=
                        passwordHash.size())
                    {
                        outputFile.close();

                        QMessageBox::warning(
                            this,
                            "Error",
                            "Could not write password verification data."
                            );

                        ui->statusLabel->setText(
                            "Encryption failed."
                            );

                        return;
                    }


                    if (outputFile.write(salt) !=
                        salt.size())
                    {
                        outputFile.close();

                        QMessageBox::warning(
                            this,
                            "Error",
                            "Could not write salt."
                            );

                        ui->statusLabel->setText(
                            "Encryption failed."
                            );

                        return;
                    }


                    if (outputFile.write(iv) !=
                        iv.size())
                    {
                        outputFile.close();

                        QMessageBox::warning(
                            this,
                            "Error",
                            "Could not write IV."
                            );

                        ui->statusLabel->setText(
                            "Encryption failed."
                            );

                        return;
                    }


                    if (outputFile.write(encryptedData) !=
                        encryptedData.size())
                    {
                        outputFile.close();

                        QMessageBox::warning(
                            this,
                            "Error",
                            "Could not write encrypted file."
                            );

                        ui->statusLabel->setText(
                            "Encryption failed."
                            );

                        return;
                    }


                    outputFile.close();



                    if (!QFile::remove(fileName))
                    {
                        QMessageBox::warning(
                            this,
                            "Warning",
                            "Encryption was successful, but the original file could not be removed."
                            );

                        ui->statusLabel->setText(
                            "Encryption successful, original file not removed."
                            );

                        return;
                    }


                    ui->statusLabel->setText(
                        "Encryption successful."
                        );

                    QMessageBox::information(
                        this,
                        "Success",
                        "AES-128 encryption successful."
                        );

                    return;
                }




                QFileInfo fileInfo(fileName);

                QString outputFileName =
                    fileInfo.path() + "/" +
                    fileInfo.completeBaseName() +
                    "_encrypted." +
                    fileInfo.suffix();


                QFile outputFile(outputFileName);

                if (!outputFile.open(QIODevice::WriteOnly))
                {
                    inputFile.close();

                    QMessageBox::warning(
                        this,
                        "Error",
                        "Could not create encrypted file."
                        );

                    ui->statusLabel->setText(
                        "Encryption failed."
                        );

                    return;
                }



                if (outputFile.write(passwordHash) !=
                    passwordHash.size())
                {
                    inputFile.close();
                    outputFile.close();

                    QMessageBox::warning(
                        this,
                        "Error",
                        "Could not write encryption data."
                        );

                    ui->statusLabel->setText(
                        "Encryption failed."
                        );

                    return;
                }




                while (!inputFile.atEnd())
                {
                    QByteArray data =
                        inputFile.read(4096);


                    if (data.isEmpty() && !inputFile.atEnd())
                    {
                        inputFile.close();
                        outputFile.close();

                        QMessageBox::warning(
                            this,
                            "Error",
                            "Could not read the original file."
                            );

                        ui->statusLabel->setText(
                            "Encryption failed."
                            );

                        return;
                    }


                    if (algorithm == "Vigenere")
                    {
                        data =
                            vigenereEncrypt(
                                data,
                                password
                                );
                    }
                    else
                    {

                        for (int i = 0; i < data.size(); i++)
                        {
                            unsigned char byte =
                                static_cast<unsigned char>(
                                    data[i]
                                    );

                            byte =
                                static_cast<unsigned char>(
                                    (byte + shift) % 256
                                    );

                            data[i] =
                                static_cast<char>(byte);
                        }
                    }



                    if (outputFile.write(data) != data.size())
                    {
                        inputFile.close();
                        outputFile.close();

                        QMessageBox::warning(
                            this,
                            "Error",
                            "Could not write encrypted file."
                            );

                        ui->statusLabel->setText(
                            "Encryption failed."
                            );

                        return;
                    }
                }


                inputFile.close();
                outputFile.close();




                if (!QFile::remove(fileName))
                {
                    QMessageBox::warning(
                        this,
                        "Warning",
                        "Encryption was successful, but the original file could not be removed."
                        );

                    ui->statusLabel->setText(
                        "Encryption successful, original file not removed."
                        );

                    return;
                }


                ui->statusLabel->setText(
                    "Encryption successful."
                    );

                QMessageBox::information(
                    this,
                    "Success",
                    "File encrypted successfully."
                    );
            });





    connect(ui->decryptButton, &QPushButton::clicked, this, [this]()
            {
                QString fileName = ui->filePathEdit->text();
                QString password = ui->passwordEdit->text();

                QString algorithm =
                    ui->algorithmComboBox->currentText();


                if (fileName.isEmpty())
                {
                    QMessageBox::warning(
                        this,
                        "Error",
                        "Please select a file first."
                        );
                    return;
                }



                if (password.isEmpty())
                {
                    QMessageBox::warning(
                        this,
                        "Error",
                        "Please enter a password."
                        );
                    return;
                }


                ui->statusLabel->setText(
                    "Checking password..."
                    );


                QByteArray enteredPasswordHash =
                    QCryptographicHash::hash(
                        password.toUtf8(),
                        QCryptographicHash::Sha256
                        );


                int shift = 0;

                for (int i = 0; i < password.length(); i++)
                {
                    shift += password[i].unicode();
                }

                shift %= 256;


                QFile inputFile(fileName);

                if (!inputFile.open(QIODevice::ReadOnly))
                {
                    QMessageBox::warning(
                        this,
                        "Error",
                        "Could not open encrypted file."
                        );

                    ui->statusLabel->setText(
                        "Decryption failed."
                        );

                    return;
                }


                QByteArray storedPasswordHash =
                    inputFile.read(32);


                if (storedPasswordHash.size() != 32)
                {
                    inputFile.close();

                    QMessageBox::warning(
                        this,
                        "Error",
                        "Invalid encrypted file."
                        );

                    ui->statusLabel->setText(
                        "Decryption failed."
                        );

                    return;
                }



                if (storedPasswordHash != enteredPasswordHash)
                {
                    inputFile.close();

                    QMessageBox::warning(
                        this,
                        "Wrong Password",
                        "The password is incorrect."
                        );

                    ui->statusLabel->setText(
                        "Wrong password."
                        );

                    return;
                }


                ui->statusLabel->setText(
                    "Decrypting..."
                    );


                if (algorithm == "AES-128")
                {

                    QByteArray salt =
                        inputFile.read(16);

                    if (salt.size() != 16)
                    {
                        inputFile.close();

                        QMessageBox::warning(
                            this,
                            "Error",
                            "Invalid AES-128 encrypted file."
                            );

                        ui->statusLabel->setText(
                            "Decryption failed."
                            );

                        return;
                    }



                    QByteArray iv =
                        inputFile.read(16);

                    if (iv.size() != 16)
                    {
                        inputFile.close();

                        QMessageBox::warning(
                            this,
                            "Error",
                            "Invalid AES-128 encrypted file."
                            );

                        ui->statusLabel->setText(
                            "Decryption failed."
                            );

                        return;
                    }



                    QByteArray encryptedData =
                        inputFile.readAll();

                    inputFile.close();


                    QByteArray key =
                        deriveAESKey(
                            password,
                            salt
                            );


                    if (key.size() != 16)
                    {
                        QMessageBox::warning(
                            this,
                            "Error",
                            "Could not create AES-128 key."
                            );

                        ui->statusLabel->setText(
                            "Decryption failed."
                            );

                        return;
                    }



                    QByteArray decryptedData =
                        aesDecrypt(
                            encryptedData,
                            key,
                            iv
                            );



                    if (decryptedData.isEmpty() &&
                        !encryptedData.isEmpty())
                    {
                        QMessageBox::warning(
                            this,
                            "Error",
                            "AES-128 decryption failed. The encrypted file may be damaged."
                            );

                        ui->statusLabel->setText(
                            "Decryption failed."
                            );

                        return;
                    }



                    QFileInfo fileInfo(fileName);

                    QString baseName =
                        fileInfo.completeBaseName();


                    if (baseName.endsWith("_encrypted"))
                    {
                        baseName.chop(
                            QString("_encrypted").length()
                            );
                    }


                    QString outputFileName =
                        fileInfo.path() + "/" +
                        baseName +
                        "." +
                        fileInfo.suffix();



                    QFile outputFile(outputFileName);

                    if (!outputFile.open(QIODevice::WriteOnly))
                    {
                        QMessageBox::warning(
                            this,
                            "Error",
                            "Could not create decrypted file."
                            );

                        ui->statusLabel->setText(
                            "Decryption failed."
                            );

                        return;
                    }



                    if (outputFile.write(decryptedData) !=
                        decryptedData.size())
                    {
                        outputFile.close();

                        QMessageBox::warning(
                            this,
                            "Error",
                            "Could not write decrypted file."
                            );

                        ui->statusLabel->setText(
                            "Decryption failed."
                            );

                        return;
                    }


                    outputFile.close();



                    if (!QFile::remove(fileName))
                    {
                        QMessageBox::warning(
                            this,
                            "Warning",
                            "Decryption was successful, but the encrypted file could not be removed."
                            );

                        ui->statusLabel->setText(
                            "Decryption successful, encrypted file not removed."
                            );

                        return;
                    }


                    ui->statusLabel->setText(
                        "Decryption successful."
                        );

                    QMessageBox::information(
                        this,
                        "Success",
                        "AES-128 decryption successful."
                        );

                    return;
                }



                QFileInfo fileInfo(fileName);

                QString baseName =
                    fileInfo.completeBaseName();


                if (baseName.endsWith("_encrypted"))
                {
                    baseName.chop(
                        QString("_encrypted").length()
                        );
                }


                QString outputFileName =
                    fileInfo.path() + "/" +
                    baseName +
                    "_decrypted." +
                    fileInfo.suffix();




                QFile outputFile(outputFileName);

                if (!outputFile.open(QIODevice::WriteOnly))
                {
                    inputFile.close();

                    QMessageBox::warning(
                        this,
                        "Error",
                        "Could not create decrypted file."
                        );

                    ui->statusLabel->setText(
                        "Decryption failed."
                        );

                    return;
                }



                while (!inputFile.atEnd())
                {
                    QByteArray data =
                        inputFile.read(4096);


                    if (data.isEmpty() && !inputFile.atEnd())
                    {
                        inputFile.close();
                        outputFile.close();

                        QMessageBox::warning(
                            this,
                            "Error",
                            "Could not read encrypted file."
                            );

                        ui->statusLabel->setText(
                            "Decryption failed."
                            );

                        return;
                    }



                    if (algorithm == "Vigenere")
                    {
                        data =
                            vigenereDecrypt(
                                data,
                                password
                                );
                    }
                    else
                    {

                        for (int i = 0; i < data.size(); i++)
                        {
                            unsigned char byte =
                                static_cast<unsigned char>(
                                    data[i]
                                    );

                            byte =
                                static_cast<unsigned char>(
                                    (byte - shift + 256) % 256
                                    );

                            data[i] =
                                static_cast<char>(byte);
                        }
                    }



                    if (outputFile.write(data) !=
                        data.size())
                    {
                        inputFile.close();
                        outputFile.close();

                        QMessageBox::warning(
                            this,
                            "Error",
                            "Could not write decrypted file."
                            );

                        ui->statusLabel->setText(
                            "Decryption failed."
                            );

                        return;
                    }
                }


                inputFile.close();
                outputFile.close();



                if (!QFile::remove(fileName))
                {
                    QMessageBox::warning(
                        this,
                        "Warning",
                        "Decryption was successful, but the encrypted file could not be removed."
                        );

                    ui->statusLabel->setText(
                        "Decryption successful, encrypted file not removed."
                        );

                    return;
                }


                ui->statusLabel->setText(
                    "Decryption successful."
                    );

                QMessageBox::information(
                    this,
                    "Success",
                    "File decrypted successfully."
                    );
            });


}


MainWindow::~MainWindow()
{
    delete ui;
}