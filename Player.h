//
// Created by Stepan on 15.10.2016.
//

#ifndef SERVER_PLAYER_H
#define SERVER_PLAYER_H

enum ePlayerState
{
    READY = 0,
    WAITING_FOR_UPSTREAM,
    WAITING_FOR_DOWNSTREAM,
};

enum eStreams
{
    UPSTREAM = 0,
    DOWNSTREAM,
    MAX_STREAMS
};

class Player
{
    Player(Socket* upstream, string name, string session)
        : _name(name)
        , _session(session)
        , _state(WAITING_FOR_DOWNSTREAM)
    {
        memset(_streams,MAX_STREAMS, nullptr);
        setStream(upstream, UPSTREAM);
    }

    void setStream(Socket* stream, eStreams type)
    {
        assert(type >= MAX_STREAMS);
        if (_streams[type])
            delete _streams[type];
        _streams[type] = stream;
        _state &= ~(type+WAITING_FOR_UPSTREAM);
        stream.setOnClose(type, &onCloseStream);
    }

    void onCloseStream(eStreams type)
    {
        assert(type >= MAX_STREAMS);
        if (_streams[type])
            delete _streams[type];
        _streams[type] = nullptr;
        _state |= (type+WAITING_FOR_UPSTREAM);
    }

    void update()
    {

    }


private:
    ePlayerState _state;
    string _name;
    string _session;
    Socket* _streams[MAX_STREAMS];
};


#endif //SERVER_PLAYER_H
