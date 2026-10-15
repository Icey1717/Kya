#include <winsock2.h>
#include <gtest/gtest.h>
#include "../../DebugMenu/src/DebugSocket.h"
#include "renderer.h"
#include "VulkanRenderer.h"
#include <nlohmann/json.hpp>
#include <string>

namespace
{
	class DebugSocketTest : public testing::Test
	{
	protected:
		bool headless;
		SOCKET client = INVALID_SOCKET;
		void SetUp() override
		{
			headless = Renderer::gHeadless;
			Renderer::SetHeadless(true);
			ASSERT_TRUE(Debug::Socket::Start(0));
			Debug::Socket::ClearHistory();
			client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			ASSERT_NE(client, INVALID_SOCKET);
			sockaddr_in address{};
			address.sin_family = AF_INET;
			address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
			address.sin_port = htons(Debug::Socket::GetPort());
			ASSERT_EQ(connect(client, reinterpret_cast<sockaddr*>(&address), sizeof(address)), 0);
			u_long mode = 1;
			ASSERT_EQ(ioctlsocket(client, FIONBIO, &mode), 0);
		}
		void TearDown() override
		{
			if (client != INVALID_SOCKET) closesocket(client);
			Debug::Socket::Stop();
			Renderer::SetHeadless(headless);
		}
		nlohmann::json Exchange(const std::string& input)
		{
			EXPECT_EQ(send(client, input.data(), static_cast<int>(input.size()), 0), input.size());
			std::string response;
			for (int i = 0; i < 100; ++i) {
				Debug::Socket::Poll();
				char buffer[4096];
				const int count = recv(client, buffer, sizeof(buffer), 0);
				if (count > 0) response.append(buffer, count);
				if (response.find('\n') != std::string::npos) return nlohmann::json::parse(response);
				Sleep(1);
			}
			ADD_FAILURE() << "Socket response timed out";
			return {};
		}
	};
}

TEST_F(DebugSocketTest, FragmentedStatusRequestRetainsId)
{
	const std::string prefix = "{\"command\":\"sta";
	ASSERT_EQ(send(client, prefix.data(), int(prefix.size()), 0), prefix.size());
	Debug::Socket::Poll();
	const auto response = Exchange("tus\",\"id\":42}\n");
	EXPECT_EQ(response["id"], 42);
	EXPECT_EQ(response["ok"], true);
	EXPECT_EQ(response["protocol"], 1);
	EXPECT_EQ(response["bytes"], 0);
	EXPECT_EQ(response["pid"], GetCurrentProcessId());
	const auto requests = Debug::Socket::GetRequests();
	ASSERT_EQ(requests.size(), 1u);
	EXPECT_EQ(requests[0].command, "status");
	EXPECT_EQ(requests[0].requestId, "42");
	EXPECT_EQ(requests[0].state, "Completed");
	EXPECT_TRUE(requests[0].finished);
	EXPECT_TRUE(requests[0].error.empty());
	EXPECT_EQ(requests[0].sentBytes, requests[0].responseBytes);
	EXPECT_GT(requests[0].sentBytes, 0u);
	EXPECT_GE(requests[0].elapsedMs, requests[0].processingMs);
}

TEST_F(DebugSocketTest, MalformedJsonReturnsError)
{
	EXPECT_EQ(Exchange("not json\n")["ok"], false);
}

TEST_F(DebugSocketTest, UnknownCommandReturnsErrorAndId)
{
	const auto response = Exchange("{\"command\":\"unknown\",\"id\":\"test\"}\n");
	EXPECT_EQ(response["ok"], false);
	EXPECT_EQ(response["id"], "test");
	const auto requests = Debug::Socket::GetRequests();
	ASSERT_EQ(requests.size(), 1u);
	EXPECT_EQ(requests[0].state, "Error");
	EXPECT_EQ(requests[0].error, response["error"].get<std::string>());
}

TEST_F(DebugSocketTest, ClearingHistoryPreservesLiveRequests)
{
	Debug::Socket::Poll();
	auto requests = Debug::Socket::GetRequests();
	ASSERT_EQ(requests.size(), 1u);
	EXPECT_EQ(requests[0].state, "Receiving");
	EXPECT_FALSE(requests[0].finished);
	Debug::Socket::ClearHistory();
	EXPECT_EQ(Debug::Socket::GetRequests().size(), 1u);
	EXPECT_EQ(Exchange("{\"command\":\"status\"}\n")["ok"], true);
	Debug::Socket::ClearHistory();
	EXPECT_TRUE(Debug::Socket::GetRequests().empty());
}

TEST_F(DebugSocketTest, DisconnectedClientRemainsInHistory)
{
	Debug::Socket::Poll();
	closesocket(client);
	client = INVALID_SOCKET;
	for (int i = 0; i < 100; ++i) {
		Debug::Socket::Poll();
		const auto requests = Debug::Socket::GetRequests();
		if (!requests.empty() && requests[0].finished) {
			EXPECT_EQ(requests[0].state, "Disconnected");
			return;
		}
		Sleep(1);
	}
	FAIL() << "Disconnected client was not recorded";
}

TEST_F(DebugSocketTest, MissingFramebufferReturnsError)
{
	const auto response = Exchange("{\"command\":\"screenshot\"}\n");
	EXPECT_EQ(response["ok"], false);
	EXPECT_EQ(response["error"], "Game framebuffer is unavailable");
}

TEST_F(DebugSocketTest, OversizedRequestIsRejected)
{
	const auto response = Exchange(std::string(9000, 'x') + "\n");
	EXPECT_EQ(response["ok"], false);
	EXPECT_EQ(response["error"], "Request exceeds 8192 bytes");
}

TEST_F(DebugSocketTest, StopReleasesTheListener)
{
	const auto port = Debug::Socket::GetPort();
	closesocket(client);
	client = INVALID_SOCKET;
	Debug::Socket::Stop();
	EXPECT_EQ(Debug::Socket::GetPort(), 0);
	EXPECT_TRUE(Debug::Socket::Start(port));
}
