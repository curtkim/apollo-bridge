import asyncio

from enum import IntEnum
class OP(IntEnum):
    REGISTER_DESC = 1
    ADD_READER = 2
    ADD_WRITER = 3
    PUBLISH = 4


async def handle_echo(reader, writer):

    while(True):
        buf = await reader.read(1)
        op = int.from_bytes(buf, byteorder='little')
        # print(op)

        if op == OP.REGISTER_DESC:
            print("REGISTER_DESC")
            buf = await reader.read(4)
            count = int.from_bytes(buf, byteorder='little')
            #print(op, count)

            for i in range(count):
                buf = await reader.read(4)
                size = int.from_bytes(buf, byteorder='little')
                print(op, count, size)
                buf = await reader.read(size)
                #print(buf4)
                print("-----", i)
                desc = buf.decode(encoding="ascii", errors="ignore")
                #print(desc)

        elif op == OP.ADD_READER:
            print("ADD_READER")
            buf = await reader.read(4)
            channel_length = int.from_bytes(buf, byteorder='little')
            buf = await reader.read(channel_length)
            channel = buf.decode()

            buf = await reader.read(4)
            type_length = int.from_bytes(buf, byteorder='little')
            buf = await reader.read(type_length)
            type = buf.decode()

            print("READER", channel, type)

        elif op == OP.ADD_WRITER:
            print("ADD_WRITER")

            buf = await reader.read(4)
            channel_length = int.from_bytes(buf, byteorder='little')
            buf = await reader.read(channel_length)
            channel = buf.decode()

            buf = await reader.read(4)
            type_length = int.from_bytes(buf, byteorder='little')
            buf = await reader.read(type_length)
            type = buf.decode()

            print("WRITER", channel, type)

        elif op == OP.PUBLISH:
            buf = await reader.read(4)
            channel_length = int.from_bytes(buf, byteorder='little')
            buf = await reader.read(channel_length)
            channel = buf.decode()

            buf = await reader.read(4)
            message_length = int.from_bytes(buf, byteorder='little')
            (read_length, buffers) = await read_exactly(reader, message_length)
            print("PUBLISH", channel, message_length, read_length)

        else:
            print("Unknown", op)

    print("Close the connection")
    writer.close()


async def read_exactly(reader, length):
    count = 0
    buffers = []
    while count < length:
        buf = await reader.read(length - count)
        count += len(buf)
        buffers.append(buf)
    return count, buffers


"""
    data = await reader.read(100)
    message = data.decode()
    addr = writer.get_extra_info('peername')

    print(f"Received {message!r} from {addr!r}")

    print(f"Send: {message!r}")
    writer.write(data)
    await writer.drain()

    print("Close the connection")
    writer.close()
"""

async def main():
    server = await asyncio.start_server(
        handle_echo, '127.0.0.1', 9090)

    addr = server.sockets[0].getsockname()
    print(f'Serving on {addr}')

    async with server:
        await server.serve_forever()

asyncio.run(main())