#include "security.h"

Security::Security()
{
}

//字符串MD5哈希值，登录验证
QString Security::HashMD5(QString password)
{
    // 创建加密对象
    QCryptographicHash hash(QCryptographicHash::Md5);
    // 添加明文数据
    hash.addData(password.toUtf8());
    // 获取加密后的数据
    // 16个字节的数据
    QByteArray new_password = hash.result();
    // 转16进制
    //qDebug()<<new_password.toHex();
    return new_password.toHex();
}

QString Security::TextToBase64(QString str)
{
    // base64进行加密
    QByteArray text = str.toLocal8Bit();
    QByteArray by = text.toBase64();
    return QString(by);
}

QString Security::Base64ToText(QString str)
{
    // base64进行解密
    QByteArray text = str.toLocal8Bit();
    QByteArray by = text.fromBase64(text);
    QString result = QString::fromLocal8Bit(by);
    return result;
}

//文件的sha256哈希值
QString Security::sha_f(QString filepath){
    SHA256_CTX c;
    unsigned char* digest=new unsigned char[32];
    QFile file;
    file.setFileName(filepath);
    file.open(QIODevice::ReadOnly);
    SHA256_Init(&c);
    QByteArray array = file.readAll();
    qDebug()<<array.size();
    SHA256_Update(&c,array,array.size());
    SHA256_Final(digest,&c);
    file.close();
    QString result=QByteArray((char*)digest,32).toHex();
    return result;
}

//计算文件sm3哈希值
QString Security::sm3_f(QString filepath){
    unsigned char* digest=new unsigned char[32];
    unsigned int hash_len;
    QFile file;
    file.setFileName(filepath);
    file.open(QIODevice::ReadOnly);
    QByteArray array = file.readAll();
    qDebug()<<array.size();
    EVP_MD_CTX *md_ctx;
    const EVP_MD *md;
    md = EVP_sm3();
    md_ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(md_ctx, md, NULL);
    EVP_DigestUpdate(md_ctx, array, array.size());
    EVP_DigestFinal_ex(md_ctx, digest, &hash_len);
    EVP_MD_CTX_free(md_ctx);
    file.close();
    QString result=QByteArray((char*)digest,32).toHex();
    return result;
}

QByteArray Security::HexStrToByte(QString str)
{
    QByteArray byte_arr;
    bool ok;
    if (str.size() % 2 != 0) {
        return QByteArray::fromHex("");
    }
    int len = str.size();
    for (int i = 0; i < len; i += 2) {
        byte_arr.append(char(str.mid(i, 2).toUShort(&ok, 16)));
    }
    return byte_arr;
}

//计算文件sm3短哈希值
QString Security::genSh(QString filehash){
    QByteArray qb=HexStrToByte(filehash.mid(0,4));
    QByteArray resultb;
    resultb.append(qb.at(0)&0xff);
    resultb.append(qb.at(1)&0xf8);
    QString result=resultb.toHex();
    return result;
}

//生成随机密钥
QString Security::get_rand_key(unsigned char * key,int l){
    RAND_bytes(key, l);
    QString result=QByteArray((char*)key,l).toHex();
    return result;
}

//文件sm4加密
int Security::Encrypt_File(unsigned char* key,QString filename){
    unsigned char iv[EVP_MAX_KEY_LENGTH] = "EVP_SM4_CTR"; //保存初始化向量的数组
    EVP_CIPHER_CTX* ctx;    //EVP加密上下文环境
    ctx = EVP_CIPHER_CTX_new();
    unsigned char out[1024];  //保存密文的缓冲区
    int outl;
    unsigned char in[1024];   //保存原文的缓冲区
    int inl;
    int rv;
    FILE *fpIn;
    FILE *fpOut;
    //打开待加密文件
    fpIn = fopen(filename.toStdString().c_str(), "rb");
    if(fpIn == NULL){
       return -1;
    }
    //打开保存密文的文件
    char encryptedfname[100];
    strcpy(encryptedfname, filename.toStdString().c_str());
    strcat(encryptedfname, ".encrypted");
    fpOut = fopen(encryptedfname, "wb");
    if(fpOut == NULL){
       fclose(fpIn);
       return -1;
    }
    //初始化ctx
    EVP_CIPHER_CTX_init(ctx);
    //设置密码算法、key和iv
    rv = EVP_EncryptInit_ex(ctx, EVP_sm4_ecb(), NULL, key, iv);
    if(rv != 1){
       printf("Err\n");
       return -1;
    }
    //循环读取原文，加密后后保存到密文文件。
    for(;;){
       inl = fread(in,1,1024,fpIn);
       if(inl <= 0)//读取原文结束
          break;
       rv = EVP_EncryptUpdate(ctx, out, &outl, in, inl);//加密
       if(rv != 1){
          fclose(fpIn);
          fclose(fpOut);
          EVP_CIPHER_CTX_cleanup(ctx);
          return -1;
       }
       fwrite(out, 1, outl, fpOut);//保存密文到文件
    }
    //加密结束
    rv = EVP_EncryptFinal_ex(ctx, out, &outl);
    if(rv != 1){
       fclose(fpIn);
       fclose(fpOut);
       EVP_CIPHER_CTX_cleanup(ctx);
       return -1;
    }
    fwrite(out,1,outl,fpOut);  //保密密文到文件
    fclose(fpIn);
    fclose(fpOut);
    EVP_CIPHER_CTX_cleanup(ctx); //清除EVP加密上下文环境
    printf("加密已完成\n");
    return 1;
}

//文件sm4解密
int Security::Decrypt_File(unsigned char* key,QString filename){
    unsigned char iv[EVP_MAX_KEY_LENGTH] = "EVP_SM4_CTR";  //保存初始化向量的数组
    EVP_CIPHER_CTX* ctx;    //EVP加密上下文环境
    ctx=EVP_CIPHER_CTX_new();
    unsigned char out[1024+EVP_MAX_KEY_LENGTH]; //保存解密后明文的缓冲区数组
    int outl;
    unsigned char in[1024];    //保存密文数据的数组
    int inl;
    int rv;
    FILE *fpIn;
    FILE *fpOut;
  //打开待解密的密文文件
    fpIn = fopen(filename.toStdString().c_str(), "rb");
    if(fpIn == NULL)
    {
       return -1;
    }
    char decryptedfname[100];
    strcpy(decryptedfname, filename.toStdString().c_str());
    strcat(decryptedfname, ".decrypted");
    //打开保存明文的文件
    fpOut = fopen(decryptedfname, "wb");
    if(fpOut == NULL)
    {
       fclose(fpIn);
       return -1;
    }
    //初始化ctx
    EVP_CIPHER_CTX_init(ctx);
    //设置解密的算法、key和iv
    rv = EVP_DecryptInit_ex(ctx, EVP_sm4_ecb(), NULL, key, iv);
    if(rv != 1)
    {
       EVP_CIPHER_CTX_cleanup(ctx);
       return -1;
    }
    //循环读取原文，解密后后保存到明文文件。
    for(;;)
    {
       inl = fread(in, 1, 1024, fpIn);
       if(inl <= 0)
          break;
       rv = EVP_DecryptUpdate(ctx, out, &outl, in, inl);//解密
       if(rv != 1)
       {
          fclose(fpIn);
          fclose(fpOut);
          EVP_CIPHER_CTX_cleanup(ctx);
          return -1;
       }
       fwrite(out, 1, outl, fpOut);//保存明文到文件
    }
    //解密结束
    rv = EVP_DecryptFinal_ex(ctx, out, &outl);
    if(rv != 1)
    {
       fclose(fpIn);
       fclose(fpOut);
       EVP_CIPHER_CTX_cleanup(ctx);
       return -1;
    }
    fwrite(out,1,outl,fpOut);//保存明文到文件
    fclose(fpIn);
    fclose(fpOut);
    EVP_CIPHER_CTX_cleanup(ctx);//清除EVP加密上下文环境
    printf("解密已完成\n");
    return 1;
}

//hmac
QString Security::hmac(QByteArray key,QString filePath){
    QMessageAuthenticationCode code(QCryptographicHash::Sha256);
    code.setKey(key);
    QFile file;
    file.setFileName(filePath);
    file.open(QIODevice::ReadOnly);
    QByteArray message=file.readAll();
    file.close();
    code.addData(message);
    return code.result().toHex();
}

QString Security::hmacS(QByteArray key,QByteArray message){
    QMessageAuthenticationCode code(QCryptographicHash::Sha256);
    code.setKey(key);
    code.addData(message);
    return code.result().toHex();
}

//字符串sm3-hmac
QString Security::HMAC_SM3(QString data, QString key){
    HMAC_CTX* ctx=HMAC_CTX_new();
    unsigned int len;
    unsigned char out[EVP_MAX_MD_SIZE];
    HMAC_Init_ex(ctx, key.toStdString().c_str(), key.length(), EVP_sm3(),NULL);
    HMAC_Update(ctx, (unsigned char*)data.toStdString().c_str(), data.length());
    HMAC_Final(ctx, out, &len);
    HMAC_CTX_free(ctx);
    QString result=QByteArray((char*)out,32).toHex();
    return result;
}

//两个32字节十六进制字符串异或
QString Security::hashXor(QString hexStr1,QString hexStr2){
    QString result;
    for(int i=0;i<32;i++){//TODO 只计算了r的前3个字节
        QString temp1=hexStr1.mid(2*i,2);
        QString temp2=hexStr2.mid(2*i,2);
        uint hex = temp1.toUInt(NULL, 16);     // hex == 255, ok == true
        uint hex2 = temp2.toUInt(NULL, 16);
        uint res=hex^hex2;
        QString temp=QString::number(res,16);
        if(temp.size()==1)temp.insert(0,"0");
        result+=temp;
    }
    return result;
}

QString Security::block(QString filePath,int index){
    //fileblock
    QFile file1,file2,file3;
    file1.setFileName(filePath);
    file2.setFileName("/home/weakdog/clientfiles/block");
    file1.open(QIODevice::ReadOnly);
    file2.open(QIODevice::WriteOnly);
    file1.seek(1024*index);
    char buf[1024]={0};
    //往文件中读数据
    int len=file1.read(buf,sizeof(buf));
    file2.write(buf,len);
    file1.close();
    file2.close();
    //计算哈希值
    QString blockHash=sm3_f("/home/weakdog/clientfiles/block");
    file3.setFileName("/home/weakdog/clientfiles/block");
    file3.remove();
    return blockHash;
}
