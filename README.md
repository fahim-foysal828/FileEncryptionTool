File Encryption Tool

<br>

A C++ and Qt-based desktop application for encrypting and decrypting files using Caesar Cipher, Vigenere Cipher, and AES-128.

<br>

Features :

- File selection using Browse button
- Password-protected encryption and decryption
- Caesar Cipher
- Vigenere Cipher
- AES-128 encryption and decryption
- SHA-256 password verification
- Random salt generation for AES-128
- PBKDF2-HMAC-SHA256 key derivation
- Random IV generation for AES-128
- Show Password option
- Encryption and decryption status messages
- Original file is removed after successful encryption
- Encrypted file is removed after successful decryption
- Supports text and binary files
- Processes one file at a time

  <br>

Technologies Used :

- C++17 — Main programming language
- Qt 6 — Desktop GUI framework
- Qt Widgets — GUI components
- CMake — Build configuration
- MinGW 64-bit — Compiler
- OpenSSL — Cryptographic operations
- SHA-256 — Password verification
- PBKDF2-HMAC-SHA256 — AES key derivation

<br>


Project Structure

- README.md — Project documentation
- CMakeLists.txt — Build and project configuration
- main.cpp — Application entry point
- mainwindow.h — Main window class declaration
- mainwindow.cpp — Main application and encryption/decryption logic
- mainwindow.ui — Qt GUI design

<br>


File Description :


main.cpp :-
Starts the Qt application and creates the main window.


mainwindow.h :-
Contains the declaration of the "MainWindow" class.


mainwindow.cpp :-
Contains the main application logic, including:

- File handling
- Password verification
- Caesar Cipher
- Vigenere Cipher
- AES-128 encryption and decryption
- SHA-256
- PBKDF2-HMAC-SHA256
- Random salt generation
- Random IV generation
- GUI button operations



mainwindow.ui :-
Contains the graphical user interface designed using Qt Designer.



CMakeLists.txt :-
Contains the project build configuration, including Qt and OpenSSL configuration.


README.md :-
Contains the documentation of the project.




<br>
<br>

How the Application Works :
1. Select a file using the Browse button.
2. Enter a password.
3. Select an encryption algorithm.
4. Click the Encrypt or Decrypt button.
5. The application processes the selected file.
6. The encryption or decryption operation is completed.
7. The application displays the operation status.



<br>

Algorithms :

<br>

1. Caesar Cipher



   
The Caesar Cipher processes file data byte by byte.

The shift value is calculated from the password:

Shift = Sum of password character Unicode values % 256




Encryption:
Encrypted Byte = (Original Byte + Shift) % 256




Decryption:
Original Byte = (Encrypted Byte - Shift + 256) % 256

<br>
<br>


2. Vigenere Cipher

   <br>
   
The Vigenere Cipher uses the password as a repeating key.
For each password character:
Shift = Password Character Unicode Value % 256



Encryption:
Encrypted Byte = (Original Byte + Shift) % 256


Decryption:
Original Byte = (Encrypted Byte - Shift + 256) % 256


<br>
<br>

3. AES-128

<br>

AES-128 is implemented using OpenSSL.


The password is used with a random salt to derive the AES key using PBKDF2-HMAC-SHA256.

<br>

Password

   ↓
   
Random Salt

   ↓
   
PBKDF2-HMAC-SHA256

   ↓
   
100,000 Iterations

   ↓
   
16-Byte AES Key

   ↓
   
AES-128-CBC

   ↓
   
Encrypted Data


<br>

<br>

Password Verification :

The entered password is converted into a SHA-256 hash.

Entered Password
       ↓
       
   SHA-256
       ↓
   32-Byte Hash
       ↓
Compare with Stored Hash

If the hashes do not match, the application displays:

The password is incorrect.

The password itself is not stored directly.


<br>

Salt Generation :

For AES-128, a random 16-byte salt is generated using OpenSSL "RAND_bytes()".

The salt is used by PBKDF2 during AES key derivation.

<br>


PBKDF2 :

The AES key is generated using:

Algorithm: PBKDF2-HMAC-SHA256
Iterations: 100,000
Output Key Size: 16 bytes

The 16-byte derived key is suitable for AES-128.


<br>


AES Initialization Vector :

A random 16-byte IV is generated using OpenSSL "RAND_bytes()".

The IV is used with AES-128-CBC during encryption and is stored in the encrypted file so that the data can be decrypted later.


<Br>


AES Encrypted File Structure :

The AES encrypted file contains:

32 bytes  → SHA-256 password hash
16 bytes  → Salt
16 bytes  → IV
Remaining → Ciphertext


<br>

File Processing :

For Caesar Cipher and Vigenere Cipher, the file is processed in 4096-byte chunks.

For AES-128, the file data is read into memory before encryption or decryption.

The application supports both text and binary files.


<br>


Output File Naming

Encryption :
original.txt
     ↓
original_encrypted.txt

Caesar/Vigenere Decryption

original_encrypted.txt
     ↓
original_decrypted.txt

AES-128 Decryption

original_encrypted.txt
     ↓
original.txt

File Removal Behavior

After successful encryption:

Original File → Removed
Encrypted File → Remains

After successful decryption:

Encrypted File → Removed
Decrypted File → Remains

<br>

GUI :

The application provides:

- File path display
- Browse button
- Password input
- Show Password option
- Algorithm selection
- Encrypt button
- Decrypt button
- Status messages

<br>

Build System:

The project uses:-

- C++17
- Qt 6
- Qt Widgets
- CMake
- MinGW 64-bit
- OpenSSL

OpenSSL is used through its Crypto library.

<br>



<br>

How to Build and Run :

1. Install Qt and a C++ compiler.
2. Install OpenSSL.
3. Open the project in Qt Creator.
4. Configure the CMake project.
5. Make sure Qt and OpenSSL are detected correctly.
6. Build the project.
7. Run the application.

<br>

Security Notes:

<br>

- Caesar Cipher and Vigenere Cipher are classical educational ciphers and are not considered modern secure encryption algorithms.
- AES-128 is implemented using OpenSSL.
- AES uses CBC mode.
- PBKDF2-HMAC-SHA256 is used for AES key derivation.
- A random salt is used for PBKDF2.
- A random IV is used for AES-128-CBC.
- The current implementation does not use an HMAC or authentication tag.
- Therefore, the application does not provide authenticated encryption or dedicated tamper detection.

<br>

Future Improvements :

- Add HMAC or authenticated encryption
- Improve password verification design
- Add AES streaming for very large files
- Add more encryption algorithms
- Add cross-platform packaging
- Improve error logging
- Add stronger file integrity protection

 <br>
 
Project Purpose:

The main purpose of this project is to demonstrate practical use of:

- C++
- Qt GUI development
- File handling
- Classical encryption algorithms
- AES-128
- OpenSSL
- SHA-256
- PBKDF2
- Salt and IV generation
- CMake

This project was developed for educational purposes.

<br>

License :

This project is intended for educational purposes.
