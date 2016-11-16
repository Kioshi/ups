package bomberman;

import java.util.Arrays;

/**
 * Created by Štěpán Martínek on 15.11.2016.
 */
public class Packet
{
    static byte LOGIN = 0;
    static byte SESSION = 1;

    Packet(byte _opcode, int _size, byte[] _data)
    {
        opcode = _opcode;
        size = (byte)_size;
        if (_data != null)
            data = Arrays.copyOf(_data, _data.length);
    }

    Packet(byte _opcode)
    {
        this(_opcode,0,null);
    }

    byte getSize()
    {
        return (byte)(1 + (size > 0 ? 1 : 0) + size);
    }

    byte opcode;
    byte size;
    byte data[];
}
