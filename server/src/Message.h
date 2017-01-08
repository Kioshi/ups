#pragma once
#include <string>
#include <vector>

enum Opcodes
{
    CMSG_LOGIN = 0,
    CMSG_SESSION,
    CMSG_LOBBY_LIST,
    CMSG_CREATE_LOBBY,
    CMSG_JOIN_LOBBY,
    CMSG_LEAVE_LOBBY,
    CMSG_START_GAME,
    CMSG_KICK_PLAYER,
    CMSG_FF,
    CMSG_DRAW,
    CMSG_CARDS,
    CMSG_PLAY,
    CMSG_DISCARD,
    CMSG_QUIT,

    SMSG_SESSION,
    SMSG_LOBBY_LIST,
    SMSG_CREATE_LOBBY,
    SMSG_JOIN_LOBBY,
    SMSG_LEAVE_LOBBY,
    SMSG_START_GAME,
    SMSG_KICK_PLAYER,
    SMSG_FF,
    SMSG_DRAW,
    SMSG_CARDS,
    SMSG_PLAY,
    SMSG_DISCARD,
    SMSG_MESSAGE,
    SMSG_GAME_END,

    UNUSED, //placehoder
};

class Message
{
public:
    Message(Opcodes _opcode, std::vector<std::string> tokens);

    Message(Opcodes _opcode, std::string _string);

    Message(Opcodes _opcode);

    explicit operator Opcodes() const;
    
    Opcodes opcode;
    std::string data;

    std::string toString();
};
