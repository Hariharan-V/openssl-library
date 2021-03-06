#ifndef CRYPTO_HH_
#define CRYPTO_HH_

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <iostream>
#include <string.h>
#include <iostream>
#include <fstream>
#include <openssl/err.h>
#include <openssl/rand.h>

#include <memory>
#include <limits>
#include <stdexcept>
#include <openssl/bio.h>
#include <string.h>
#include <openssl/rsa.h>
#include <openssl/engine.h>
#include <openssl/sha.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;
#define byte unsigned char
static const unsigned int KEY_SIZE = 32;
static const unsigned int BLOCK_SIZE = 16;
template <typename T>
struct secallocator///secure allocator that implements secure_string
{
public:
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    
    pointer address (reference v) const {return &v;}
    const_pointer address (const_reference v) const {return &v;}
    
    pointer allocate (size_type n, const void* hint = 0) {
        if (n > std::numeric_limits<size_type>::max() / sizeof(T))
            throw std::bad_alloc();
        return static_cast<pointer> (::operator new (n * sizeof (value_type)));
    }
    
    void deallocate(pointer p, size_type n) {
        OPENSSL_cleanse(p, n*sizeof(T));
        ::operator delete(p);
    }
    
    size_type max_size() const {
        return std::numeric_limits<size_type>::max() / sizeof (T);
    }
    
    template<typename U>
    struct rebind
    {
        typedef secallocator<U> other;
    };
    
    void construct (pointer ptr, const T& val) {
        new (static_cast<T*>(ptr) ) T (val);
    }
    
    void destroy(pointer ptr) {
        static_cast<T*>(ptr)->~T();
    }
    
#if __cpluplus >= 201103L
    template<typename U, typename... Args>
    void construct (U* ptr, Args&&  ... args) {
        ::new (static_cast<void*> (ptr) ) U (std::forward<Args> (args)...);
    }
    
    template<typename U>
    void destroy(U* ptr) {
        ptr->~U();
    }
#endif
};


typedef std::basic_string<char, std::char_traits<char>, secallocator<char> > secure_string;
using EVP_CIPHER_CTX_free_ptr = std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)>;


class Random{
public:
    void next(byte* buffer,size_t size);

};


class SymmetricKey{
private:
    void aes_encrypt(const byte Key[KEY_SIZE], const byte Iv[BLOCK_SIZE], const secure_string& ptext, secure_string& ctext);
    void aes_decrypt(const byte Key[KEY_SIZE], const byte Iv[BLOCK_SIZE], const secure_string& ctext, secure_string& rtext);
    void SymIVKeygen();
    byte key[KEY_SIZE];
    byte iv[BLOCK_SIZE];
    secure_string msg;
    secure_string cmsg;
public:
    SymmetricKey();
    SymmetricKey(byte*KEY, int keysize, byte*IV, int IVsize);
    void encrypt(secure_string buffer);
    unsigned char* getKey();
    unsigned  char* getIV();
    void newKey();
    void decrypt(secure_string cypher);
    int setKeyIV(byte*KEY, int keysize, byte*IV, int IVsize);
    secure_string getCypher();
    secure_string getMsg();
    ~SymmetricKey();
};
class RSASig{
private:
    unsigned char* signature;
    unsigned int siglen;
public:
    RSASig(unsigned char* sign, unsigned long len);
    unsigned char* getSig();
    unsigned int getLen();
    ~RSASig();
};

class RSApubKey{
private:
    BIGNUM *N;
    BIGNUM *E;
    
public:
    RSApubKey(RSA *keypair);
    RSApubKey(char* Hexn, char* Hexe);
    char * n();
    char *e ();
    RSA *genRSAkey();
    ~RSApubKey();
};
class RSApub{
private:
    RSA *keypair;
    bool simpleSHA256(void* input, unsigned long length, unsigned char* md);
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
public:
	  void sha256(char *string, unsigned char hash[SHA256_DIGEST_LENGTH]);
  	void sha256(char *string, unsigned char hash[SHA256_DIGEST_LENGTH],int len);
    static void printHex(ostream& s, const unsigned char buf[], int n);
    RSApub();
  	RSApub(RSA * key);
    RSApub(RSApubKey publickey);
	  char *encrypt(char* msg,int size) const;
    char* decrypt(char* encryptedmsg);
    int keysize() const;
  	RSASig signobj(char* message);
    unsigned char*  sign(char* message);
	int verifysigobj(unsigned char* messageHash, RSASig sign);	 
    int verify(unsigned char* messageHash, unsigned char* sign);
    unsigned char *gethash();
    RSApubKey getPubKey();
    ~RSApub();
};
class FileEncryption{
private:
    SymmetricKey sym;
    secure_string cipher;
    secure_string message;
    string filename;
    string encfilename;
    RSApub *client;
    RSApub *server;
public:
    FileEncryption(string FILENAME, string ENCFILENAME,SymmetricKey &SYM, RSApub *Client, RSApub *Server);
    FileEncryption(string FILENAME,SymmetricKey &SYM,RSApub *Client,RSApub *Server );
    void readinfile();
    void encryptandsendtofile();
};

class FileDecryption {
    
private:
    
    
    secure_string message; // encrypted raw text
    string filename;       // original (encrypted) file
    string decfilename;    // decrypted file
    RSApub *client;
    char * IV ;            // seed for AES-256 CBC
    char *KEY ;            // symmetric key (AES-256)
    RSApub *server;
public:
    FileDecryption(const string& FILENAME, const string& DECP, RSApub *Client,RSApub *Server);
    FileDecryption(string FILENAME,RSApub *Client,RSApub *Server);
    void readinfile();
    void  decryptandsendtofile();
};


//class BigInt {
//public:
//    BigInt(uint64_t arr[], int n){}
//	friend ostream& operator <<(ostream& s, const BigInt& b);
//	friend istream& operator >>(istream& s, BigInt& b);
//	
//};
//
//
//
//	
//class RemoteBackup {
//private:
//	string serviceName; // ie drive.google.com or dropbox.com
//	string userid; // user id to log into
//	string passwd; // password to use
//	string path; // path of filename on the service (this could be combined with serviceName)
//	uint32_t offset; // offset within file at which the data is to be stored
//	uint8_t skipBytes; // can skip bytes (do not have to use every one)
//	uint8_t startBit; // start storing data at this bit in each byte
//	uint8_t stopBit; // stop storing data at this bit in each byte
//	//	uint32_t bits[32]; // extra control of how data is stored for future expansion?
//	
//	// default is all bits used, but we can skip bytes, use only start to stop bits
//	//example: 2:2:0  (store every other byte in bits from 2 to 0 (0 is rightmost)
//public:
//	RemoteBackup(const string serviceName, const string& userid, const string& passwd,
//							 uint32_t offset, uint8_t skipBytes, uint8_t startBit, uint8_t stopBit) {}
//	void backup(const Repository& r) {}
//};
//
///**
//	 Store Error Correction data to be able to recover from accidental
//	 byte modifications
//
//	 Since the bytes in the ECC itself can go wrong, the ECC will itself
//	 contain a smaller ECC to validate and correct all bytes in the ECC
//
//	 For deliberate tampering, all we can do is identify that it has
//	 happened.
// */
//class ECC {
//private:
//	vector<uint64_t> ecc; // bits for multi-dimensional ECC
//	uint32_t ecc_crc32;
//	vector<uint64_t> ecc2; // bits for ecc of the ecc
//	uint32_t ecc2_crc32;
//public:
//	// create this ECC based on a block of 64 bit data
//	ECC(const uint64_t* data, uint32_t size);
//	
//};
//
///**
//	 Message Authentication Code, represent a secure hash used to
//	 validate whether an encrypted block has been tampered
// */
//class MAC {
//
//};
//
///**
//	 Represent information required to unlock an area of a repository
//
//	  A repository consists of an unknown number of blocks of information
//		at random offsets.  The UnlockInfo object indicates a key,
//		and a list of blocks to unlock. 
//		Each block is 64-bit aligned, and sizes are in 64-bit chunks
//
//		SymmetricKey 
//		numblocks
//		offset length
//		offset length
//		..
// */
//class UnlockInfo {
//	SymmetricKey key;
//	vector<BlockInfo> blocks;
//};
//
//class Repository {
//private:
//	vector<RemoteBackup> remoteBackups;
//	vector<UnlockInfo> unlockKeys;
//
//	vector<PublicKey> serverKeys;
//	vector<KeyPair> clientKeys; // for each client account, one keypair is generated
//	ECC ecc;
//	MAC auth; // authenticate this object to so tampering can be detected
//public:
//	Repository() {}
//
//	// add a login to this repository with standard userid and password
//	void addLogin(const string& userid, const string& passwd);
//	
//	// add a login to this repository with a password and second factor authentication
//	void addLogin(const string& password, const U2F& factor2);
//
//	// look up server name and get public key
//	PublicKey getServer(const string& serverName);
//
//	//
//	bool validateServer(const string& serverName) const;
//	Socket getConnection(const string& serverName) const;
//};

#endif
