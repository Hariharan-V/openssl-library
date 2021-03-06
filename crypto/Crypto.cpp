

#include "Crypto.hh"







    
    void Random::next(byte* buffer , size_t size){
        
        
        int rc = RAND_bytes(buffer, size);//randomly generate buffer numbers and store the success/failure code in rc
        unsigned long err = ERR_get_error();
        
        while(rc != 1) {//if the assignment is a failure (or not strong enough) keep reassigning the buffer
            rc = RAND_bytes(buffer, size);
            long err = ERR_get_error();
        }
        int newseed =RAND_bytes(buffer, size);//get a new seed from the current buffer so that there isn't a cycle
        RAND_seed(buffer, newseed);//add the seed
        //        for(int x = 0; x<size*4;x++){
        //            cout<<(int)buffer[x]<<endl;
        //        }
        
        
    }
    
    



/**
 
 Represent a symmetric key used by this system.  Currently this
 should be AES-256 We use AES-256 to encrypt session traffic between
 client and server, and also to encrypt the repository on each side
 so it cannot be read by a user with physical access to the computer
 when it is not on.
 
 */




    
    void SymmetricKey::aes_encrypt(const byte Key[KEY_SIZE], const byte Iv[BLOCK_SIZE], const secure_string& ptext, secure_string& ctext)//code from their website
    {
        EVP_CIPHER_CTX_free_ptr ctx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free);
        int rc = EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_cbc(), NULL, Key, Iv);
        if (rc != 1)
            throw std::runtime_error("EVP_EncryptInit_ex failed");
        
        // Recovered text expands upto BLOCK_SIZE
        
        ctext.resize(ptext.size()+BLOCK_SIZE);
			     int out_len1 = (int)ctext.size();
        
        rc = EVP_EncryptUpdate(ctx.get(), (byte*)&ctext[0], &out_len1, (const byte*)&ptext[0], (int)ptext.size());
        if (rc != 1)
            throw std::runtime_error("EVP_EncryptUpdate failed");
        
        int out_len2 = (int)ctext.size() - out_len1;
        rc = EVP_EncryptFinal_ex(ctx.get(), (byte*)&ctext[0]+out_len1, &out_len2);
        if (rc != 1)
            throw std::runtime_error("EVP_EncryptFinal_ex failed");
        
        // Set cipher text size now that we know it
        ctext.resize(out_len1 + out_len2);
    }
    
    void SymmetricKey::aes_decrypt(const byte key[KEY_SIZE], const byte iv[BLOCK_SIZE], const secure_string& ctext, secure_string& rtext)
    {
        EVP_CIPHER_CTX_free_ptr ctx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free);
        int rc = EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_cbc(), NULL, key, iv);
        if (rc != 1)
            throw std::runtime_error("1EVP_DecryptInit_ex failed");
        
        // Recovered text contracts upto BLOCK_SIZE
        rtext.resize(ctext.size());
        int out_len1 = (int)rtext.size();
        
        rc = EVP_DecryptUpdate(ctx.get(), (byte*)&rtext[0], &out_len1, (const byte*)&ctext[0], (int)ctext.size());
        if (rc != 1)
            throw std::runtime_error("2EVP_DecryptUpdate failed");
        
        int out_len2 = (int)rtext.size() - out_len1;
        rc = EVP_DecryptFinal_ex(ctx.get(), (byte*)&rtext[0]+out_len1, &out_len2);
        if (rc != 1){
            ERR_print_errors_fp(stderr);
            throw std::runtime_error("3EVP_DecryptFinal_ex failed");
            
        }
        
        // Set recovered textr size now that we know it
        rtext.resize(out_len1 + out_len2);
    }
    void SymmetricKey::SymIVKeygen(){
        
        
        
        // plaintext, ciphertext, recovered text
        Random rand;//initialize random number generator
        
        rand.next(key,KEY_SIZE);//fill up the key and the iv with random nombars
        rand.next(iv,BLOCK_SIZE);
        
    }


    
    SymmetricKey::SymmetricKey(){
        SymIVKeygen();//initialize the local variables
        
    }
    SymmetricKey::SymmetricKey(byte*KEY, int keysize, byte*IV, int IVsize){
        setKeyIV(KEY,  keysize, IV, IVsize);
        
    }
    void SymmetricKey::encrypt(secure_string buffer){//encrypt a secure string buffer
        EVP_add_cipher(EVP_aes_256_cbc());
        msg  = buffer;
        aes_encrypt(key, iv, msg, cmsg);
        
        
    }
    
    unsigned char* SymmetricKey::getKey(){//get a copy of the key
        unsigned char* Key = (unsigned char*)malloc(sizeof(unsigned  char)*KEY_SIZE);
        
        for(int x = 0; x<KEY_SIZE; x++){
            Key[x]=key[x];
            
        }
        return Key;
    }
    unsigned  char* SymmetricKey::getIV(){//get a copy of the initialization vector
        unsigned char* IV = (unsigned char*)malloc(sizeof(unsigned char)*BLOCK_SIZE);
        
        for(int x = 0; x<BLOCK_SIZE; x++){
            IV[x]=iv[x];
            
        }
        return IV;
    }
    
    
    void SymmetricKey::newKey(){//generate new key
        OPENSSL_cleanse(key, KEY_SIZE);
        OPENSSL_cleanse(iv, BLOCK_SIZE);
        SymIVKeygen();
    }
    void SymmetricKey::decrypt(secure_string cypher){//decrypt
        EVP_add_cipher(EVP_aes_256_cbc());
        cmsg = cypher;
        aes_decrypt(key, iv, cmsg, msg);
        
        
    }
    int SymmetricKey::setKeyIV(byte*KEY, int keysize, byte*IV, int IVsize){//set your own custom key (not recommended unless it was generated by another computer)
        if(keysize!=KEY_SIZE||IVsize!=BLOCK_SIZE){
            return 1;//error
        }
        for(int x = 0; x<KEY_SIZE;x++){
            
            key[x]=KEY[x];
        }
        for(int x = 0; x<BLOCK_SIZE;x++){
            iv[x] = IV[x];
        }
        return 0;//success
    }
    secure_string SymmetricKey::getCypher(){//get the cypher test
        return cmsg;
    }
    secure_string SymmetricKey::getMsg(){//get the message
        return msg;
    }
    SymmetricKey::~SymmetricKey(){
        
    }



    RSASig:: RSASig(unsigned char* sign, unsigned long len){//stores the signature and signature length
        signature = (unsigned char*)malloc(siglen);
        memcpy((char*)signature,(char*) sign,siglen);
        siglen = len;
    }
    unsigned char* RSASig::getSig(){//returns address to a copy of a signature
        unsigned char* sign = (unsigned char*)malloc(siglen);
        memcpy((char*)sign,(char*) signature,siglen);
        return sign;
    }
    unsigned int RSASig:: getLen(){
        return siglen;
    }
    RSASig:: ~RSASig() { free(signature); }



    RSApubKey:: RSApubKey(RSA *keypair){
        const BIGNUM *n;
        RSA_get0_key(keypair, &n, NULL, NULL);
        N = BN_dup(n);
        E = BN_new();
        BN_set_word(E,3);
        
    }
    RSApubKey:: RSApubKey(char* Hexn, char* Hexe){//hex versions of n and e ONLY (hex of n and hex of e)
        BN_hex2bn(&N, Hexn);
        BN_hex2bn(&E, Hexe);
        
    }
    char *  RSApubKey:: n(){
        return BN_bn2hex(N);
    }
    char * RSApubKey:: e (){
        return BN_bn2hex(E);
    }
    RSA *  RSApubKey::genRSAkey(){
        RSA *keypair = RSA_new();
        RSA_set0_key(keypair, N, E, NULL);
        return keypair;
    }
    RSApubKey:: ~RSApubKey(){
        BN_free(N);
        BN_free(E);
    }





    bool RSApub::simpleSHA256(void* input, unsigned long length, unsigned char* md)
    {
        SHA256_CTX context;
        if(!SHA256_Init(&context))
            return false;
        
        if(!SHA256_Update(&context, (unsigned char*)input, length))
            return false;
        
        if(!SHA256_Final(md, &context))
            return false;
        
        return true;
    }
    
    
    void  RSApub:: sha256(char *string, unsigned char hash[SHA256_DIGEST_LENGTH]) {
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, string, strlen(string));
        SHA256_Final(hash, &sha256);
    }
    void   RSApub::sha256(char *string, unsigned char hash[SHA256_DIGEST_LENGTH],int len) {
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, string, len);
        SHA256_Final(hash, &sha256);
    }
    void  RSApub::printHex(ostream& s, const unsigned char buf[], int n) {
        s << hex;
        for (int i = 0; i < n; i++)
            s << (int)buf[i]<<" ";
    }
     RSApub::RSApub(){
        keypair = RSA_new();
        BIGNUM *e = BN_new();
        BN_set_word(e,3);//3 is apparently the best one
        RSA_generate_key_ex(keypair, 2048, e, NULL);
    }
    RSApub:: RSApub(RSA *key){
        keypair = key;
    }
    RSApub:: RSApub(RSApubKey publickey){
        keypair = publickey.genRSAkey();
        
    }
    //default padding has been set to RSA_PKCS1_OAEP_PADDING
    char *  RSApub::encrypt(char* msg,int size) const {//msg has to be RSA_size(keypair) - 41 bytes, encrypted msg is usually no more than RSA_size(keypair)
        char* encryptedmsg = (char*)malloc(keysize());
        int len;
        if((len=RSA_public_encrypt(size,(unsigned char *)msg, (unsigned char*)encryptedmsg,keypair,RSA_PKCS1_OAEP_PADDING))==-1 ){
            return NULL;
        }
        return encryptedmsg;
    }
    
    char*  RSApub::decrypt(char* encryptedmsg) {//decrypts encryptedmsg and stores in msg
        char* msg =(char*)malloc(keysize());
        
        int len;
        if((len=RSA_private_decrypt(keysize(), (unsigned char*)encryptedmsg,(unsigned char *)msg,keypair,RSA_PKCS1_OAEP_PADDING))==-1 ){
            ERR_print_errors_fp(stderr);
            return NULL;
        }
        
        return msg;
    }
    
    //TODO: exactly what size is returned?//RSA modulus size in bytes
    int  RSApub::keysize() const {
        return RSA_size(keypair);
    }
    
    RSASig  RSApub::signobj(char* message) {
        unsigned char *signature = (unsigned char*)malloc(keysize());
        sha256(message,hash);
        unsigned int signaturelen;
        int status =  RSA_sign(NID_sha256, hash, (unsigned int) sizeof(hash),
                               signature,  &signaturelen, keypair);
        
        
        RSASig signaturecpy(signature, signaturelen);
        
        return signaturecpy;
    }
    unsigned char*  RSApub::sign(char* message) {
        unsigned char *signature = (unsigned char*)malloc(keysize());
        sha256(message,hash);
        unsigned int signaturelen;
        int status =  RSA_sign(NID_sha256, hash, (unsigned int) sizeof(hash),
                               signature,  &signaturelen, keypair);
        
        
        return signature;
    }
    
    
    int  RSApub::verifysigobj(unsigned char* messageHash, RSASig sign) {
        
        return RSA_verify(NID_sha256, hash,SHA256_DIGEST_LENGTH, sign.getSig(), sign.getLen(), keypair);
    }
    int  RSApub::verify(unsigned char* messageHash, unsigned char* sign) {
        
        return RSA_verify(NID_sha256, hash,SHA256_DIGEST_LENGTH, sign, 256, keypair);
    }
    unsigned char * RSApub::gethash(){
        unsigned char* hashcpy = (unsigned char*)malloc(sizeof(hash));
        memcpy((char*)hashcpy, (const char *)hash,sizeof(hash));
        return hashcpy;
        
    }
    RSApubKey  RSApub::getPubKey(){
        RSApubKey newpubkey(keypair);
        return newpubkey;
    }
    RSApub:: ~RSApub(){
        RSA_free(keypair);
    }
    



    FileEncryption:: FileEncryption(string FILENAME, string ENCFILENAME,SymmetricKey &SYM, RSApub *Client, RSApub *Server){
        filename = FILENAME;
        encfilename = ENCFILENAME+".enc";
        client = Client;
        sym = SYM;
        server = Server;
        readinfile();
        encryptandsendtofile();
        
    }
    FileEncryption::FileEncryption(string FILENAME,SymmetricKey &SYM,RSApub *Client,RSApub *Server ){
        filename = FILENAME;
        encfilename = FILENAME+".enc";
        client = Client;
        server = Server;
        sym = SYM;
        readinfile();
        encryptandsendtofile();
    }
    void FileEncryption::readinfile(){
        streampos size;
        char * memblock;
        
        ifstream file (filename, ios::in|ios::binary|ios::ate);
        if (file.is_open())
        {
            size = file.tellg();
            memblock = new char [size];
            file.seekg (0, ios::beg);
            file.read (memblock, size);
            file.close();
        }else {}
        message.resize(size);
        
        memcpy(&message[0],memblock,size);
        delete [] memblock;
        
    }
    void FileEncryption::encryptandsendtofile(){
        unsigned char *x  = server->sign(&message[0]);
        message.resize(message.size()+256);
        memcpy(&message[message.size()-256],x,256);
        sym.encrypt(message);
        secure_string Cipher = sym.getCypher();
        cipher = Cipher;
        ofstream file (encfilename, ios::out|ios::binary|ios::ate);
        
        if (file.is_open())
        {
            file.write (&cipher[0], cipher.size());
            
            
            
            
            char* g = (*client).encrypt((char*)sym.getIV(),16);
            
            
            file.write(g,256);
            char* k =(*client).encrypt((char*)sym.getKey(),32);
            
            file.write(k,256);
            
            
            
        }else{}
        file.close();
        
    }
    

    FileDecryption:: FileDecryption(const string& FILENAME, const string& DECP, RSApub *Client,RSApub *Server){
        IV = new char[256];
        KEY = new char[256];
        filename = FILENAME;
        decfilename = DECP;
        client = Client;
        server = Server;
        readinfile();
        decryptandsendtofile();
    }
   FileDecryption:: FileDecryption(string FILENAME,RSApub *Client,RSApub *Server){
        IV = new char[256];
        KEY = new char[256];
        filename = FILENAME;
        client = Client;
        server = Server;
        decfilename = FILENAME.substr(0,FILENAME.size()-4);
        readinfile();
        decryptandsendtofile();
    }
    
    void FileDecryption::readinfile(){
        streampos size;
        char * memblock;
        
        int F;
        ifstream file (filename, ios::in|ios::binary|ios::ate);
        if (file.is_open())
        {
            size = file.tellg();
            memblock = new char [size];
            F = size;
            file.seekg (0, ios::beg);
            file.read (memblock, size);
            file.close();
        } else {}
        message.resize(F-512);
        memcpy(&message[0],&memblock[0],F-512);
        memcpy(IV,&memblock[F-512],256);
        
        
        memcpy(KEY,&memblock[F-256],256);
        
        
        delete [] memblock;
        
    }
    void FileDecryption:: decryptandsendtofile(){
        const char* decryptedKey =client->decrypt(KEY);
        
        const char* decryptedIV = client->decrypt(IV);
        
        SymmetricKey sym ((unsigned char*)decryptedKey,32,(unsigned char*)decryptedIV,16);
        sym.decrypt(message);
        secure_string decmessage =sym.getMsg();
        ofstream file (decfilename, ios::out|ios::binary|ios::ate);
        
        if (file.is_open())
        {
            char *signature = new char[256];
            unsigned char hash[SHA256_DIGEST_LENGTH];
            
            client->sha256(&decmessage[0],hash,decmessage.size()-256);
            memcpy(signature,&decmessage[decmessage.size()-256],256);
            if(client->verify(hash,(unsigned char*)signature)!=1){
                throw  std::runtime_error("error: not verifiyable");
            }
            file.write (&decmessage[0], decmessage.size()-256);
        }else{}
        file.close();
        
    }


int main(){
    RSApub pub;
    SymmetricKey sym;
   
    FileEncryption file2("txt.txt", sym, &pub,&pub);
		FileDecryption file1("txt.txt.enc", "txtdec.txt",&pub,&pub);

}

