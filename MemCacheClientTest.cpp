/*! @file   TestMemCacheClient.cpp
    @brief  Test program for MemCacheClient
*/

#ifdef WIN32
# include <winsock2.h>
# pragma comment(lib, "ws2_32.lib")
#endif

#include <memory>
#include <stdexcept>
#include <stdio.h>

#include "MemCacheClient.h"

#define STR(x)      #x
#define VERIFY(x)   if (!(x)) throw std::runtime_error(STR(x));

#ifdef WIN32
# define MilliSleep(x)  Sleep(x)
#else
# define MilliSleep(x)  usleep((x) * 1000)
#endif

static int TestServerParsing()
{
    // ensure that adding servers correctly parses the name
    try {
        std::auto_ptr<MemCacheClient> pClient(new MemCacheClient);

        VERIFY(!pClient->AddServer("foo"));
        VERIFY(!pClient->AddServer("localhost"));
        VERIFY(!pClient->AddServer("12.12.12.12.12"));
        VERIFY(!pClient->AddServer("www.microsoft.com"));
        VERIFY(!pClient->AddServer(""));
        VERIFY(!pClient->AddServer(NULL));
        VERIFY(!pClient->AddServer("255.255.255.255:12345"));

        VERIFY(pClient->AddServer("127.0.0.1"));
        VERIFY(pClient->AddServer("127.0.0.1"));
        VERIFY(pClient->AddServer("127.0.0.1:11212"));
        VERIFY(pClient->AddServer("127.0.0.1:11212"));
        VERIFY(pClient->AddServer("1.2.3.4:12345"));

        printf("PASSED: TestServerParsing\n");
        return 0;
    }
    catch (const std::exception & e) {
        printf("FAILED: TestServerParsing: %s\n", e.what());
        return 1;
    }
}

static int TestAdd(MemCacheClient * pClient)
{
    MemCacheClient::MemRequest oItem;
    try {
        VERIFY(pClient->FlushAll() > 0);

        oItem.mKey = "TestAdd";
        oItem.mData.WriteBytes("TestAdd", sizeof("TestAdd"));

        VERIFY(pClient->Add(oItem) == 1);
        VERIFY(oItem.mResult == MCERR_OK);

        VERIFY(pClient->Add(oItem) == 1);
        VERIFY(oItem.mResult == MCERR_NOTSTORED);

        printf("PASSED: TestAdd\n");
        return 0;
    }
    catch (const std::exception & e) {
        printf("FAILED: TestAdd: %s\n", e.what());
        return 1;
    }
}

static int TestSet(MemCacheClient * pClient)
{
    MemCacheClient::MemRequest oItem;
    try {
        VERIFY(pClient->FlushAll() > 0);

        oItem.mKey = "TestSet";
        oItem.mData.WriteBytes("TestSet", sizeof("TestSet"));

        VERIFY(pClient->Set(oItem) == 1);
        VERIFY(oItem.mResult == MCERR_OK);

        VERIFY(pClient->Set(oItem) == 1);
        VERIFY(oItem.mResult == MCERR_OK);

        printf("PASSED: TestSet\n");
        return 0;
    }
    catch (const std::exception & e) {
        printf("FAILED: TestSet: %s\n", e.what());
        return 1;
    }
}

static int TestReplace(MemCacheClient * pClient)
{
    MemCacheClient::MemRequest oItem;
    try {
        VERIFY(pClient->FlushAll() > 0);

        oItem.mKey = "TestReplace";
        oItem.mData.WriteBytes("TestReplace", sizeof("TestReplace"));

        VERIFY(pClient->Replace(oItem) == 1);
        VERIFY(oItem.mResult == MCERR_NOTSTORED);

        VERIFY(pClient->Set(oItem) == 1);
        VERIFY(oItem.mResult == MCERR_OK);

        VERIFY(pClient->Replace(oItem) == 1);
        VERIFY(oItem.mResult == MCERR_OK);

        printf("PASSED: TestReplace\n");
        return 0;
    }
    catch (const std::exception & e) {
        printf("FAILED: TestReplace: %s\n", e.what());
        return 1;
    }
}

static int TestDel(MemCacheClient * pClient)
{
    MemCacheClient::MemRequest oItem;
    try {
        VERIFY(pClient->FlushAll() > 0);

        oItem.mKey = "TestDel";
        oItem.mData.WriteBytes("TestDel", sizeof("TestDel"));

        VERIFY(pClient->Add(oItem) == 1);
        VERIFY(oItem.mResult == MCERR_OK);

        VERIFY(pClient->Del(oItem) == 1);
        VERIFY(oItem.mResult == MCERR_OK);

        VERIFY(pClient->Add(oItem) == 1);
        VERIFY(oItem.mResult == MCERR_OK);

        printf("PASSED: TestDel\n");
        return 0;
    }
    catch (const std::exception & e) {
        printf("FAILED: TestDel: %s\n", e.what());
        return 1;
    }
}

static int TestIncrement(MemCacheClient * pClient)
{
    MemCacheClient::MemRequest oItem;
    MemCacheClient::uint64_t nValue = 999;
    try {
        VERIFY(pClient->FlushAll() > 0);

        VERIFY(pClient->Increment("TestIncrement", &nValue, 1, true) == MCERR_NOTFOUND);
        VERIFY(nValue == 999);

        oItem.mKey = "TestIncrement";
        oItem.mData.WriteBytes("999", 3);
        VERIFY(pClient->Add(oItem) == 1);
        VERIFY(oItem.mResult == MCERR_OK);

        VERIFY(pClient->Increment("TestIncrement", &nValue, 1, true) == MCERR_OK);
        VERIFY(nValue == 1000);

        VERIFY(pClient->Increment("TestIncrement", &nValue, 100, true) == MCERR_OK);
        VERIFY(nValue == 1100);

        VERIFY(pClient->Increment("TestIncrement", &nValue, 50, false) == MCERR_NOREPLY);
        VERIFY(nValue == 1100); // not updated as a_bWantReply is false

        VERIFY(pClient->Increment("TestIncrement", &nValue, 25, true) == MCERR_OK);
        VERIFY(nValue == 1175); 

        VERIFY(pClient->Increment("TestIncrement", NULL, 10, true) == MCERR_OK);
        VERIFY(nValue == 1175); // not updated

        VERIFY(pClient->Increment("TestIncrement", &nValue, 5, true) == MCERR_OK);
        VERIFY(nValue == 1190);

        printf("PASSED: TestIncrement\n");
        return 0;
    }
    catch (const std::exception & e) {
        printf("FAILED: TestIncrement: %s\n", e.what());
        return 1;
    }
}

static int TestDecrement(MemCacheClient * pClient)
{
    MemCacheClient::MemRequest oItem;
    MemCacheClient::uint64_t nValue = 999;
    try {
        VERIFY(pClient->FlushAll() > 0);

        VERIFY(pClient->Decrement("TestDecrement", &nValue, 1, true) == MCERR_NOTFOUND);
        VERIFY(nValue == 999);

        oItem.mKey = "TestDecrement";
        oItem.mData.WriteBytes("999", 3);
        VERIFY(pClient->Add(oItem) == 1);
        VERIFY(oItem.mResult == MCERR_OK);

        VERIFY(pClient->Decrement("TestDecrement", &nValue, 1, true) == MCERR_OK);
        VERIFY(nValue == 998);

        VERIFY(pClient->Decrement("TestDecrement", &nValue, 100, true) == MCERR_OK);
        VERIFY(nValue == 898);

        VERIFY(pClient->Decrement("TestDecrement", &nValue, 50, false) == MCERR_NOREPLY);
        VERIFY(nValue == 898); // not updated as a_bWantReply is false

        VERIFY(pClient->Decrement("TestDecrement", &nValue, 25, true) == MCERR_OK);
        VERIFY(nValue == 823); 

        VERIFY(pClient->Decrement("TestDecrement", NULL, 10, true) == MCERR_OK);
        VERIFY(nValue == 823); // not updated

        VERIFY(pClient->Decrement("TestDecrement", &nValue, 5, true) == MCERR_OK);
        VERIFY(nValue == 808);

        VERIFY(pClient->Decrement("TestDecrement", &nValue, 900, true) == MCERR_OK);
        VERIFY(nValue == 0);

        printf("PASSED: TestDecrement\n");
        return 0;
    }
    catch (const std::exception & e) {
        printf("FAILED: TestDecrement: %s\n", e.what());
        return 1;
    }
}

static int TestDelTimeout(MemCacheClient * pClient)
{
    MemCacheClient::MemRequest oItem;
    try {
        VERIFY(pClient->FlushAll() > 0);

        oItem.mKey = "TestDelTimeout";
        oItem.mData.WriteBytes("TestDelTimeout", sizeof("TestDelTimeout"));

        VERIFY(pClient->Add(oItem) == 1);
        VERIFY(oItem.mResult == MCERR_OK);

        u_long nStart = xplatform::GetCurrentTickCount();

        oItem.mExpiry = 5; // 5 seconds from now
        VERIFY(pClient->Del(oItem) == 1);
        VERIFY(oItem.mResult == MCERR_OK);
        
        oItem.mExpiry = 0;
        VERIFY(pClient->Add(oItem) == 1);
        VERIFY(oItem.mResult == MCERR_NOTSTORED);

        while (oItem.mResult == MCERR_NOTSTORED) {
            MilliSleep(100);
            VERIFY(pClient->Add(oItem) == 1);
        }

        u_long nFinish = xplatform::GetCurrentTickCount();

        VERIFY(oItem.mResult == MCERR_OK);

        u_long nPeriod = nFinish - nStart;
        VERIFY(nPeriod >= 4000); // took 5 seconds (+-20%)

        printf("PASSED: TestDelTimeout\n");
        return 0;
    }
    catch (const std::exception & e) {
        printf("FAILED: TestDelTimeout: %s\n", e.what());
        return 1;
    }
}

int main(int argc, char ** argv)
{
#ifdef WIN32
    WSADATA wsaData;
    int rc = WSAStartup(MAKEWORD(2,0), &wsaData);
    if (rc != 0) {
        printf("Failed to start winsock\n");
        return 1;
    }
#endif

    MemCacheClient * pClient = new MemCacheClient;
    for (int n = 1; n < argc; ++n) {
        if (!pClient->AddServer(argv[n])) {
            printf("Failed to add server: %s\n", argv[n]);
        }
    }
    if (argc < 2) {
        printf("No servers supplied on command line. Using localhost.\n\n");
        pClient->AddServer("127.0.0.1:11211");
    }

    std::vector<MemCacheClient::string_t> rgServers;
    pClient->GetServers(rgServers);
    for (size_t n = 0; n < rgServers.size(); ++n) {
        printf("Using server: %s\n", rgServers[n].data());
    }
    printf("\n");

    int nFails = 0;
    
    nFails += TestServerParsing();
    nFails += TestAdd(pClient);
    nFails += TestSet(pClient);
    nFails += TestReplace(pClient);
    nFails += TestDel(pClient);
    nFails += TestIncrement(pClient);
    nFails += TestDecrement(pClient);
    nFails += TestDelTimeout(pClient);

    delete pClient;

    printf("\n");
    if (nFails == 0) {
        printf("All tests passed.\n");
    }
    else {
        printf("%d tests failed\n", nFails);
        nFails = 1;
    }

#ifdef WIN32
    WSACleanup();
#endif
    return nFails;
}
