#include <iostream>
#include <fstream>
#include <string>
#include <crypto++/files.h>
#include <crypto++/osrng.h>
#include <crypto++/hex.h>
#include <crypto++/default.h> 
#include <crypto++/rsa.h>
#include <cassert>  

using namespace std;
using namespace CryptoPP;

// Utility Functions
void handleError(const string& message) {
    cerr << "Error: " << message << endl;
    exit(1);
}

fstream openFile(const string& fileName, ios::openmode mode) {
    fstream file(fileName, mode);
    if (!file) {
        handleError("Unable to open file: " + fileName);
    }
    return file;
}

void processFile(const string& sourceFile, int offset, bool encrypt = true) {
    fstream src = openFile(sourceFile, fstream::in);
    fstream tgt = openFile("temp.txt", fstream::out);

    char ch;
    while (src >> noskipws >> ch) {
        ch = encrypt ? ch + offset : ch - offset;
        tgt << ch;
    }

    src.close();
    tgt.close();

    tgt = openFile(sourceFile, ios::out);
    src = openFile("temp.txt", ios::in);

    while (src >> noskipws >> ch) {
        tgt << ch;
    }

    tgt.close();
    src.close();

    remove("temp.txt");

}

// Simple crypting and decrypting funcs "adds/substracts 100 to the characters"
void basicEncryp() {
    
    string fileName;
    cout << "Enter the file name to encrypt: ";
    cin >> fileName;

    processFile(fileName, 100, true);
    //processFile(tempFile, fileName, 100, false);

    cout << "File '" << fileName << "' encrypted successfully!" << endl;
}

void basicDecryp(){
    string fileName;
    cout << "Enter the file name to decrypt: ";
    cin >> fileName;

    processFile(fileName, 100, false);
    //processFile(tempFile, fileName, 100, true);

    cout << "File '" << fileName << "' decrypted successfully!" << endl;
}

// AES crypting and decrypting funcs
void aesEncrypt(){
    string fileName, password, fileContent, encrypted;
    cout << "Enter the filename to encrypt: ";
    cin >> fileName;

    cout << "Enter password: ";
    cin >> password;

    ifstream inputFile(fileName);
    if (!inputFile) {
        cerr << "Error opening file for reading." << endl;
        return;
    }

    fileContent.assign((istreambuf_iterator<char>(inputFile)), istreambuf_iterator<char>());
    inputFile.close();

    // Encrypt file content
    try {
        StringSource ss(fileContent, true,
        new DefaultEncryptorWithMAC(
            (CryptoPP::byte*)&password[0], password.size(),
            new StringSink(encrypted)
            )
        );
    } catch (const Exception& e) {
        cerr << "Encryption error: " << e.what() << endl;
        return;
    }

    // Write encrypted content back to the same file
    ofstream outputFile(fileName, ios::out | ios::trunc);
    if (!outputFile) {
        cerr << "Error opening file for writing." << endl;
        return;
    }
    
    outputFile << encrypted;
    outputFile.close();

    cout << "File '" << fileName << "' encrypted successfully!" << endl;
}

void aesDecrypt(){
    string fileName, password, fileContent, decrypted;
    cout << "Enter the filename to decrypt: ";
    cin >> fileName;

    cout << "Enter password: ";
    cin >> password;

    // Read encrypted file content into memory
    ifstream inputFile(fileName);
    if (!inputFile) {
        cerr << "Error opening file for reading." << endl;
        return;
    }

    fileContent.assign((istreambuf_iterator<char>(inputFile)), istreambuf_iterator<char>());
    inputFile.close();

    try {
        StringSource ss(fileContent, true,
        new DefaultDecryptorWithMAC(
            (CryptoPP::byte*)&password[0], password.size(),
            new StringSink(decrypted)
            )
        );
    } catch (const Exception& e) {
        cerr << "Encryption error: " << e.what() << endl;
        return;
    }

    // Write encrypted content back to the same file
    ofstream outputFile(fileName, ios::out | ios::trunc);
    if (!outputFile) {
        cerr << "Error opening file for writing." << endl;
        return;
    }
    
    outputFile << decrypted;
    outputFile.close();

    cout << "File '" << fileName << "' decrypted successfully!" << endl;
}

// RSA funcs
// Saving private key to a file
void saveKey(const string& fileName, const RSA::PrivateKey& key) {
    FileSink file(fileName.c_str());
    key.DEREncode(file); // Encode key in DER format
}

// Saving public key to a file
void saveKey(const string& fileName, const RSA::PublicKey& key) {
    FileSink file(fileName.c_str());
    key.DEREncode(file); // Encode key in DER format
}

// Load private key from a file
void loadKey(const string& fileName, RSA::PrivateKey& key) {
    FileSource file(fileName.c_str(), true);
    key.BERDecode(file); // Decode the key from DER format
}

// Load public key from a file
void loadKey(const string& fileName, RSA::PublicKey& key) {
    FileSource file(fileName.c_str(), true);
    key.BERDecode(file); // Decode the key from DER format
}

// Generating and saving keys
void GenerateAndSaveRSAKeys() {
    AutoSeededRandomPool rng;

    // Generate private and public keys
    RSA::PrivateKey privateKey;
    RSA::PublicKey publicKey;

    privateKey.GenerateRandomWithKeySize(rng, 2048);
    publicKey = RSA::PublicKey(privateKey);

    // Save keys to files
    saveKey("private_key.pem", privateKey);
    saveKey("public_key.pem", publicKey);

    cout << "RSA keys generated and saved to 'private_key.pem' and 'public_key.pem'.\n";
}

void rsaEncrypt(){
    AutoSeededRandomPool rng;
    RSA::PublicKey publicKey;

    FileSource publicFile("public_key.pem", true);
    publicKey.BERDecode(publicFile);

    string cipher, fileContent, fileName;
    cout << "Enter the File name to encrypt: ";
    cin >> fileName;

    ifstream inputFile(fileName);
    if (!inputFile) {
        cerr << "Error opening file for reading." << endl;
        return;
    }

    fileContent.assign((istreambuf_iterator<char>(inputFile)), istreambuf_iterator<char>());
    inputFile.close();

    try {
        RSAES_OAEP_SHA_Encryptor encryptor(publicKey);
        StringSource ss(fileContent, true,
        new PK_EncryptorFilter(rng, encryptor,
            new StringSink(cipher)
            )
        );
    } catch (const Exception& e) {
        cerr << "Encryption error: " << e.what() << endl;
        return;
    }

    // Write encrypted content back to the same file
    ofstream outputFile(fileName, ios::out | ios::trunc);
    if (!outputFile) {
        cerr << "Error opening file for writing." << endl;
        return;
    }
    
    outputFile << cipher;
    outputFile.close();

    cout << "File '" << fileName << "' encrypted successfully!" << endl;
}

void rsaDecrypt(){
    AutoSeededRandomPool rng;
    RSA::PrivateKey privateKey;

    FileSource privateFile("private_key.pem", true);
    privateKey.BERDecode(privateFile);

    string recovered, fileContent, fileName;
    cout << "Enter the File name to encrypt: ";
    cin >> fileName;

    ifstream inputFile(fileName);
    if (!inputFile) {
        cerr << "Error opening file for reading." << endl;
        return;
    }

    fileContent.assign((istreambuf_iterator<char>(inputFile)), istreambuf_iterator<char>());
    inputFile.close();

    try {
        RSAES_OAEP_SHA_Decryptor decryptor(privateKey);
        StringSource ss(fileContent, true,
        new PK_DecryptorFilter(rng, decryptor,
            new StringSink(recovered)
            )
        );
    } catch (const Exception& e) {
        cerr << "Encryption error: " << e.what() << endl;
        return;
    }

    // Write encrypted content back to the same file
    ofstream outputFile(fileName, ios::out | ios::trunc);
    if (!outputFile) {
        cerr << "Error opening file for writing." << endl;
        return;
    }
    
    outputFile << recovered;
    outputFile.close();

    cout << "File '" << fileName << "' decrypted successfully!" << endl;
}

int main() {
    int ch;
    while (1)
    {
        cout << "\n\n===FILE ENCRYPTION SYSTEM==\n" << endl;
        cout << "\n[1] Basic Encryption\n[2] AES encryption\n[3] RSA encryption\n[4] Basic decryption\n[5] AES decryption\n[6] RSA decryption\n[7] Generate RSA Keys\n[8] Exit\n" << endl;
        cout << "Enter choice:\t";
        cin >> ch;

        switch(ch) {
            case 1: basicEncryp(); break;
            case 2: aesEncrypt(); break;
            case 3: rsaEncrypt(); break;
            case 4: basicDecryp(); break;
            case 5: aesDecrypt(); break;
            case 6: rsaDecrypt(); break;
            case 7: GenerateAndSaveRSAKeys(); break;
            case 8: return 0;
            default: "Invalid Choice. Please try again.\n";
        }
    }
    
    return 0;
}