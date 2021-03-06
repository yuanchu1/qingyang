#include "s3conf.cpp"
#include "gtest/gtest.h"

TEST(Config, NonExistFile) {
    EXPECT_THROW(InitConfig("s3://abc/a config=notexist/path/s3test.conf"), S3RuntimeError);
}

TEST(Config, Basic) {
    S3Params params = InitConfig("s3://abc/a config=data/s3test.conf section=default");

    EXPECT_EQ("secret_test", params.getCred().secret);
    EXPECT_EQ("accessid_test", params.getCred().accessID);
    EXPECT_EQ("ABCDEFGabcdefg", params.getCred().token);

#ifdef S3_STANDALONE
    EXPECT_EQ(0, s3ext_segid);
    EXPECT_EQ(1, s3ext_segnum);
#endif

    EXPECT_EQ((uint64_t)6, params.getNumOfChunks());
    EXPECT_EQ((uint64_t)(64 * 1024 * 1024 + 1), params.getChunkSize());

    EXPECT_EQ(EXT_INFO, s3ext_loglevel);
    EXPECT_EQ(STDERR_LOG, s3ext_logtype);

    EXPECT_EQ(1111, s3ext_logserverport);
    EXPECT_EQ("127.0.0.1", s3ext_logserverhost);

    EXPECT_EQ((uint64_t)1024, params.getLowSpeedLimit());
    EXPECT_EQ((uint64_t)600, params.getLowSpeedTime());

    EXPECT_FALSE(params.isDebugCurl());

    EXPECT_EQ("", params.getProxy());

    EXPECT_TRUE(params.isAutoCompress());
    EXPECT_TRUE(params.isVerifyCert());

    EXPECT_EQ(SSE_S3, params.getSSEType());

    EXPECT_EQ("\n", params.getGpcheckcloud_newline());
}

TEST(Config, SpecialSectionValues) {
    S3Params params = InitConfig("s3://abc/a config=data/s3test.conf section=special_over");

    EXPECT_EQ((uint64_t)8, params.getNumOfChunks());
    EXPECT_EQ((uint64_t)(128 * 1024 * 1024), params.getChunkSize());

    EXPECT_EQ((uint64_t)10240, params.getLowSpeedLimit());
    EXPECT_EQ((uint64_t)60, params.getLowSpeedTime());

    EXPECT_FALSE(params.isDebugCurl());
    EXPECT_EQ(SSE_NONE, params.getSSEType());
}

TEST(Config, SpecialSectionLowValues) {
    S3Params params = InitConfig("s3://abc/a config=data/s3test.conf section=special_low");

    EXPECT_EQ((uint64_t)1, params.getNumOfChunks());
    EXPECT_EQ((uint64_t)(8 * 1024 * 1024), params.getChunkSize());
}

TEST(Config, SpecialSectionWrongKeyName) {
    S3Params params = InitConfig("s3://abc/a config=data/s3test.conf section=special_wrongkeyname");

    EXPECT_EQ((uint64_t)4, params.getNumOfChunks());
    EXPECT_EQ((uint64_t)(64 * 1024 * 1024), params.getChunkSize());
}

TEST(Config, SpecialSwitches) {
    S3Params params = InitConfig("s3://abc/a config=data/s3test.conf section=special_switches");

    EXPECT_TRUE(params.isDebugCurl());
    EXPECT_FALSE(params.isAutoCompress());
}

TEST(Config, SectionExist) {
    Config s3cfg("data/s3test.conf");
    EXPECT_TRUE(s3cfg.SectionExist("special_switches"));
}

TEST(Config, SectionNotExist) {
    Config s3cfg("data/s3test.conf");
    EXPECT_FALSE(s3cfg.SectionExist("not_exist"));
}

TEST(Common, CheckEssentialConfig) {
    S3Params params = InitConfig("s3://abc/a config=data/s3test.conf");

    S3Credential cred1 = {"keyid/foo", "", ""};
    params.setCred(cred1);
    EXPECT_THROW(CheckEssentialConfig(params), S3ConfigError);

    S3Credential cred2 = {"keyid/foo", "secret/bar", ""};
    params.setCred(cred2);
    s3ext_segnum = 0;
    EXPECT_THROW(CheckEssentialConfig(params), S3ConfigError);

    s3ext_segnum = 1;
}

TEST(Config, SkipVerify) {
    S3Params params = InitConfig("s3://abc/a config=data/s3test.conf section=skip_verify");
    EXPECT_FALSE(params.isVerifyCert());
}

TEST(Config, Proxy) {
    S3Params params = InitConfig("s3://abc/a config=data/s3test.conf section=proxy");
    EXPECT_EQ("https://127.0.0.1:8080", params.getProxy());
}

TEST(Config, Gpcheckcloud_eol) {
    S3Params params = InitConfig("s3://abc/a config=data/s3test.conf section=default");
    EXPECT_EQ("\n", params.getGpcheckcloud_newline());

    params = InitConfig("s3://abc/a config=data/s3test.conf section=gpcheckcloud_newline");
    EXPECT_EQ("\r\n", params.getGpcheckcloud_newline());

    EXPECT_THROW(
        InitConfig("s3://abc/a config=data/s3test.conf section=gpcheckcloud_newline_error"),
        S3ConfigError);
}
/* HttpParam test: because the unittest is compiled with S3_STANDLONE, so if we want to test
 * this case, we need change the code of s3conf.cpp line 67 like this:
 #if !defined(S3_STANDALONE)
      Config s3Cfg(configPath, httpUrl, DataDir);
 #elif defined(S3_UNITTEST)
      Config s3Cfg(configPath, httpUrl, "../../../gpAux/gpdemo/datadirs/qddir/demoDataDir-1/");
 #else
      Config s3Cfg(configPath);
 #endif
Alse add -DS3_UNITTEST in test/Makefile
Then recompile and then start dummyServer as the following test case and remove the DISABLED_
before each test case.
*/
/* Run './bin/dummyHTTPServer.py -f data/s3httptest.conf -t Parameter_Server' before enabling this test */
TEST(HttpParam, DISABLED_InitConfigWithHttpOK) {
    S3Params params = InitConfig("s3://abc/a config_server=http://127.0.0.1:8553 section=hello");
    EXPECT_EQ("\n", params.getGpcheckcloud_newline());
    S3Credential cred{"123","456","789"};
    EXPECT_EQ(cred, params.getCred());
    EXPECT_EQ(8388608, params.getChunkSize());
    EXPECT_EQ(4, params.getNumOfChunks());
    EXPECT_EQ(0, params.getKeySize());
    EXPECT_EQ(1, params.getLowSpeedLimit());
    EXPECT_EQ(2, params.getLowSpeedTime());
    EXPECT_EQ(false, params.isAutoCompress());
    EXPECT_EQ(false, params.isVerifyCert());
    EXPECT_EQ(SSE_NONE, params.getSSEType());
    EXPECT_EQ("S5://xxxx", params.getProxy());
}
/* Run './bin/dummyHTTPServer.py -f data/s3httptest.conf -t Parameter_Server -s' before enabling this test */
TEST(HttpParam, DISABLED_InitConfigWithHttpsOK) {
    S3Params params = InitConfig("s3://abc/a config_server=https://127.0.0.1:8553 section=hello");
    EXPECT_EQ("\n", params.getGpcheckcloud_newline());
    S3Credential cred{"123","456","789"};
    EXPECT_EQ(cred, params.getCred());
    EXPECT_EQ(8388608, params.getChunkSize());
    EXPECT_EQ(4, params.getNumOfChunks());
    EXPECT_EQ(0, params.getKeySize());
    EXPECT_EQ(1, params.getLowSpeedLimit());
    EXPECT_EQ(2, params.getLowSpeedTime());
    EXPECT_EQ(false, params.isAutoCompress());
    EXPECT_EQ(false, params.isVerifyCert());
    EXPECT_EQ(SSE_NONE, params.getSSEType());
    EXPECT_EQ("S5://xxxx", params.getProxy());
}
