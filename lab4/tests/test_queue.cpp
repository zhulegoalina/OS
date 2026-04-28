#include <gtest/gtest.h>
#include <fstream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <vector>
#include <string>
#include <cstring>
#include "common.h"

class QueueTest : public ::testing::Test {
protected:
    void SetUp() override {
        testFilename = "test_queue.bin";
        capacity = 5;

        // Ñמחהאול עוסעמגûי פאיכ
        std::ofstream file(testFilename, std::ios::binary | std::ios::trunc);
        if (file.is_open()) {
            QueueHeader header;
            header.capacity = capacity;
            file.write(reinterpret_cast<char*>(&header), sizeof(QueueHeader));

            Message emptyMsg;
            for (int i = 0; i < capacity; i++) {
                file.write(reinterpret_cast<char*>(&emptyMsg), sizeof(Message));
            }
            file.close();
        }
    }

    void TearDown() override {
        std::remove(testFilename.c_str());
    }

    bool WriteMessage(const std::string& message, int& msgId) {
        std::fstream file(testFilename, std::ios::binary | std::ios::in | std::ios::out);
        if (!file) return false;

        QueueHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(QueueHeader));

        if (header.count >= header.capacity) {
            file.close();
            return false;
        }

        Message msg;
        strncpy_s(msg.data, MAX_MESSAGE_LEN + 1, message.c_str(), MAX_MESSAGE_LEN);
        msg.isValid = TRUE;

        std::streampos msgPos = sizeof(QueueHeader) + header.writeIndex * sizeof(Message);
        file.seekp(msgPos);
        file.write(reinterpret_cast<char*>(&msg), sizeof(Message));

        msgId = header.nextMsgId + 1;
        header.writeIndex = (header.writeIndex + 1) % header.capacity;
        header.count++;
        header.nextMsgId++;

        file.seekp(0);
        file.write(reinterpret_cast<char*>(&header), sizeof(QueueHeader));

        file.close();
        return true;
    }

    bool ReadMessage(std::string& message, int& msgId) {
        std::fstream file(testFilename, std::ios::binary | std::ios::in | std::ios::out);
        if (!file) return false;

        QueueHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(QueueHeader));

        if (header.count == 0) {
            file.close();
            return false;
        }

        std::streampos msgPos = sizeof(QueueHeader) + header.readIndex * sizeof(Message);
        file.seekg(msgPos);

        Message msg;
        file.read(reinterpret_cast<char*>(&msg), sizeof(Message));

        msgId = header.nextMsgId - header.count + 1;
        message = msg.data;

        msg.isValid = FALSE;
        file.seekp(msgPos);
        file.write(reinterpret_cast<char*>(&msg), sizeof(Message));

        header.readIndex = (header.readIndex + 1) % header.capacity;
        header.count--;

        file.seekp(0);
        file.write(reinterpret_cast<char*>(&header), sizeof(QueueHeader));

        file.close();
        return true;
    }

    std::string testFilename;
    int capacity;
};

TEST_F(QueueTest, FIFOOrder) {
    std::string messages[] = { "First", "Second", "Third" };
    int msgIds[3];

    for (int i = 0; i < 3; i++) {
        EXPECT_TRUE(WriteMessage(messages[i], msgIds[i]));
        EXPECT_EQ(msgIds[i], i + 1);
    }

    for (int i = 0; i < 3; i++) {
        std::string readMsg;
        int readId;
        EXPECT_TRUE(ReadMessage(readMsg, readId));
        EXPECT_EQ(messages[i], readMsg);
        EXPECT_EQ(readId, i + 1);
    }
}

TEST_F(QueueTest, TestWriteRead) {
    std::string testMsg = "Hello Test";
    int msgId;

    EXPECT_TRUE(WriteMessage(testMsg, msgId));

    std::string readMsg;
    int readId;
    EXPECT_TRUE(ReadMessage(readMsg, readId));

    EXPECT_EQ(testMsg, readMsg);
    EXPECT_EQ(msgId, readId);
}

TEST_F(QueueTest, QueueFull) {
    for (int i = 0; i < capacity; i++) {
        int msgId;
        EXPECT_TRUE(WriteMessage("Msg" + std::to_string(i), msgId));
    }

    int msgId;
    EXPECT_FALSE(WriteMessage("Extra", msgId));
}

TEST_F(QueueTest, EmptyQueue) {
    std::string readMsg;
    int readId;
    EXPECT_FALSE(ReadMessage(readMsg, readId));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}