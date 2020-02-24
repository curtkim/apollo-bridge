/**
 * Copyright (c) 2019 LG Electronics, Inc.
 *
 * This software contains code licensed as described in LICENSE.
 *
 */
#include "node.h"
#include "client.h"

#include "cyber/cyber.h"
#include "cyber/common/log.h"
#include "cyber/node/node.h"
#include "cyber/node/reader.h"
#include "cyber/node/writer.h"
#include "cyber/message/py_message.h"

#include "modules/drivers/proto/sensor_image.pb.h"
#include "modules/drivers/proto/pointcloud.pb.h"
#include "modules/drivers/gnss/proto/imu.pb.h"


Node::Node()
    : node(apollo::cyber::CreateNode("bridge"))
{
}

Node::~Node()
{
}

void Node::remove(std::shared_ptr<Client> client)
{
    for (auto it = writers.begin(); it != writers.end(); /* empty */)
    {
        if (it->second.clients.find(client) != it->second.clients.end())
        {
            ADEBUG << "Removing client writer";
            it->second.clients.erase(client);
            if (it->second.clients.empty())
            {
                ADEBUG << "Removing Cyber writer";
                it = writers.erase(it);
            }
            else
            {
                it++;
            }
        }
        else
        {
            it++;
        }
    }

    std::lock_guard<std::mutex> lock(mutex);

    for (auto it = readers.begin(); it != readers.end(); /* empty */)
    {
        if (it->second.clients.find(client) != it->second.clients.end())
        {
            ADEBUG << "Removing client reader";
            it->second.clients.erase(client);
            if (it->second.clients.empty())
            {
                ADEBUG << "Removing Cyber reader";
                it = readers.erase(it);
            }
            else
            {
                it++;
            }
        }
        else
        {
            it++;
        }
    }
}

void Node::add_reader(const std::string& channel, const std::string& type, std::shared_ptr<Client> client)
{
    std::cout << "add_reader " << channel << " " << type << std::endl; 

    auto rit = readers.find(channel);
    if (rit != readers.end())
    {
        ADEBUG << "Adding client to existing " << channel;
        rit->second.clients.insert(client);
        return;
    }

    auto cb = [this, channel](const std::shared_ptr<const apollo::cyber::message::PyMessageWrap>& msg)
    {
        ADEBUG << "New message on " << channel;

        const std::string& data = msg->data();

        std::lock_guard<std::mutex> lock(mutex);

        auto it = readers.find(channel);
        if (it != readers.end())
        {
            for (auto client : it->second.clients)
            {
                client->publish(channel, data);
            }
        }
    };

    ADEBUG << "Adding new reader to " << channel;
    Reader reader;
    reader.reader = node->CreateReader<apollo::cyber::message::PyMessageWrap>(channel, cb);
    reader.clients.insert(client);

    std::lock_guard<std::mutex> lock(mutex);
    readers.insert(std::make_pair(channel, reader));
}

void Node::add_writer(const std::string& channel, const std::string& type, std::shared_ptr<Client> client)
{
    std::cout << "add_writer " << channel << " " << type << std::endl;

    auto wit = writers.find(channel);
    if (wit != writers.end())
    {
        wit->second.clients.insert(client);
        return;
    }

    Writer writer;
    writer.type = type;

    apollo::cyber::message::ProtobufFactory::Instance()->GetDescriptorString(type, &writer.desc);
    if (writer.desc.empty())
    {
        AWARN << "Cannot find proto descriptor for message type " << type;
        return;
    }

    apollo::cyber::proto::RoleAttributes role;
    role.set_channel_name(channel);
    role.set_message_type(type);
    role.set_proto_desc(writer.desc);

    auto qos_profile = role.mutable_qos_profile();
    qos_profile->set_depth(1);
    writer.writer = node->CreateWriter<apollo::cyber::message::PyMessageWrap>(role);
    writer.clients.insert(client);

    writers.insert(std::make_pair(channel, writer));
}

void Node::publish(const std::string& channel, const std::string& data)
{
    auto writer = writers.find(channel);
    if (writer == writers.end())
    {
        AWARN << "No writer registered on channel " << channel;
        return;
    }

    if( writer->second.type == "apollo.drivers.PointCloud") {
        apollo::drivers::PointCloud pc;

        std::stringstream ss(data, std::ios::in | std::ios::binary);
        pc.ParseFromIstream(&ss);
        std::cout << pc.width() << " ";

        if( pc.point_size() > 0){        
            apollo::drivers::PointXYZIT pt = pc.point(0);
            std::cout << pt.x() << " ";
        }
        std::cout << std::endl;
    }
    else if( writer->second.type == "apollo.drivers.gnss.Imu") {
        apollo::drivers::gnss::Imu imu;
        std::stringstream ss(data, std::ios::in | std::ios::binary);
        imu.ParseFromIstream(&ss);
        std::cout << imu.linear_acceleration().x() << std::endl;
        std::cout << data << std::endl;
    }
    else if( writer->second.type == "apollo.drivers.CompressedImage") {
        apollo::drivers::CompressedImage img;
        std::stringstream ss(data, std::ios::in | std::ios::binary);
        img.ParseFromIstream(&ss);
        std::cout << img.format() << std::endl;
    }

    std::cout << "publish " << channel << " " << writer->second.type << " " << data.size() << std::endl;

    auto message = std::make_shared<apollo::cyber::message::PyMessageWrap>(data, writer->second.type);
    writer->second.writer->Write(message);
}
