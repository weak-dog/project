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
    EVP_CIPHER_CTX* ctx; //EVP加密上下文环境
    ctx = EVP_CIPHER_CTX_new();
    unsigned char* out=new unsigned char[1024];  //保存密文的缓冲区
    unsigned char* in=new unsigned char[1024];   //保存原文的缓冲区
    int inl,outl;
    int rv;
    QFile fileIn,fileOut;
    fileIn.setFileName(filename);
    fileOut.setFileName(filename+".encrypted");
    fileIn.open(QIODevice::ReadOnly);
    fileOut.open(QIODevice::WriteOnly);
    //设置密码算法、key和iv
    rv = EVP_EncryptInit_ex(ctx, EVP_sm4_ecb(), NULL, key, iv);
    if(rv != 1){
       printf("Err\n");
       return -1;
    }
    //循环读取原文，加密后后保存到密文文件。
    while(!fileIn.atEnd()){
        QByteArray qbi=fileIn.read(1024);
        in=(unsigned char*)qbi.data();
        rv = EVP_EncryptUpdate(ctx, out, &outl, in, qbi.size());//加密
        if(rv != 1){
            fileIn.close();
            fileOut.close();
            EVP_CIPHER_CTX_cleanup(ctx);
            return -1;
        }
        QByteArray qbo=QByteArray((char*)out,outl);
        fileOut.write(qbo);
    }
    //加密结束
    rv = EVP_EncryptFinal_ex(ctx, out, &outl);
    if(rv != 1){
        fileIn.close();
        fileOut.close();
       EVP_CIPHER_CTX_cleanup(ctx);
       return -1;
    }
    QByteArray qbo=QByteArray((char*)out,outl);
    fileOut.write(qbo);
    fileIn.close();
    fileOut.close();
    EVP_CIPHER_CTX_cleanup(ctx); //清除EVP加密上下文环境
    qDebug()<<"加密已完成";
    return 1;
}

//文件sm4解密
int Security::Decrypt_File(unsigned char* key,QString filename){
    unsigned char iv[EVP_MAX_KEY_LENGTH] = "EVP_SM4_CTR";  //保存初始化向量的数组
    EVP_CIPHER_CTX* ctx;    //EVP加密上下文环境
    ctx=EVP_CIPHER_CTX_new();
    unsigned char* in=new unsigned char[1024];
    unsigned char* out=new unsigned char[1024+EVP_MAX_KEY_LENGTH];
    int inl,outl;
    int rv;
    QFile fileIn,fileOut;
    fileIn.setFileName(filename);
    fileOut.setFileName(filename+".decrypted");
    fileIn.open(QIODevice::ReadOnly);
    fileOut.open(QIODevice::WriteOnly);
    //初始化ctx
    EVP_CIPHER_CTX_init(ctx);
    //设置解密的算法、key和iv
    rv = EVP_DecryptInit_ex(ctx, EVP_sm4_ecb(), NULL, key, iv);
    if(rv != 1)
    {
       EVP_CIPHER_CTX_cleanup(ctx);
       return -1;
    }
    fileIn.seek(0);
    //循环读取原文，解密后后保存到明文文件。
    while(!fileIn.atEnd()){
        QByteArray qbi=fileIn.read(1024);
        in=(unsigned char*)qbi.data();
        rv = EVP_DecryptUpdate(ctx, out, &outl, in, qbi.size());//加密
        if(rv != 1){
            fileIn.close();
            fileOut.close();
            EVP_CIPHER_CTX_cleanup(ctx);
            return -1;
        }
        QByteArray qbo=QByteArray((char*)out,outl);
        fileOut.write(qbo);
    }
    //解密结束
    rv = EVP_DecryptFinal_ex(ctx, out, &outl);
    if(rv != 1)
    {
       fileIn.close();
       fileOut.close();
       EVP_CIPHER_CTX_cleanup(ctx);
       return -1;
    }
    QByteArray qbo=QByteArray((char*)out,outl);
    fileOut.write(qbo);
    fileIn.close();
    fileOut.close();
    EVP_CIPHER_CTX_cleanup(ctx);//清除EVP加密上下文环境
    qDebug()<<"解密结束";
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

//QString block2(QString filePath,int index){
//    //先取文件块存入新文件
//    //fileblock
//    QFile file1,file2,file3;
//    file1.setFileName(filePath);
//    file2.setFileName("/home/weakdog/clientfiles/block");
//    file1.open(QIODevice::ReadOnly);
//    file2.open(QIODevice::WriteOnly);
//    file1.seek(1024*index);
//    char buf[1024]={0};
//    //往文件中读数据
//    int len=file1.read(buf,sizeof(buf));
//    file2.write(buf,len);
//    file1.close();
//    file2.close();
//    //计算哈希值
//    QString blockHash=sm3_f("/home/weakdog/clientfiles/block");
//    file3.setFileName("/home/weakdog/clientfiles/block");
//    file3.remove();
//    return blockHash;
//}

QString Security::block(unsigned char* key,QString filePath,int index){
    //先取文件块存入新文件
    QFile file1,file2;
    QString filePath2=filePath+".block";
    QString filePath3=filePath+".block.encrypted";
    QString filePath4=filePath+".block.encrypted2";
    file1.setFileName(filePath);
    file2.setFileName(filePath2);
    file1.open(QIODevice::ReadOnly);
    file2.open(QIODevice::WriteOnly);
    file1.seek(1024*index);
    char buf[1024]={0};
    int len=file1.read(buf,sizeof(buf));
    file2.write(buf,len);
    file1.close();
    file2.close();
    //对新文件进行加密
    Encrypt_File(key,filePath2);
    //截取密文的前1024字节，去掉填充
    file1.setFileName(filePath3);
    file2.setFileName(filePath4);
    file1.open(QIODevice::ReadOnly);
    file2.open(QIODevice::WriteOnly);
    file1.read(buf,1024);
    file2.write(buf,1024);
    file1.close();
    file2.close();
    QString blockHash=sm3_f(filePath4);
    qDebug()<<blockHash;
    //删除文件
    file1.setFileName(filePath2);
    file1.remove();
    file1.setFileName(filePath3);
    file1.remove();
    file1.setFileName(filePath4);
    file1.remove();
    return blockHash;
}
