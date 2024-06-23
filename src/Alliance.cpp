/* Copyright (C) Daisy - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
* Written by Daisy <daisy@spitfire.pw>, February 2018
*/

#pragma once

#include "Alliance.h"
#include "spitfire.h"
#include "Client.h"
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/Data/Session.h>
#include <vector>
#include <sstream>

using namespace Poco::Data;
using namespace Poco::Data::Keywords;

// Constructor initializes the alliance with basic information
Alliance::Alliance(std::string name, std::string founder)
    : enemyactioncooldown(0), m_name(name), m_founder(founder),
      m_owner(""), m_currentmembers(0), m_maxmembers(500),
      m_prestige(0), m_honor(0), m_prestigerank(0), m_honorrank(0),
      m_membersrank(0), m_allicitycount(0)
{
    m_members.clear();
    m_enemies.clear();
    m_allies.clear();
    m_neutral.clear();
}

// Destructor
Alliance::~Alliance()
{
}

// Method to insert alliance data into the database
bool Alliance::InsertToDB()
{
    try
    {
        Poco::Data::Session ses(spitfire::GetSingleton().serverpool->get());

        // Insert basic alliance information into the database
        ses << "INSERT INTO `alliances` (name,founder,leader,created,note,intro,motd,allies,neutrals,enemies,members) VALUES (?,?,?,?,?,?,?,?,?,?,?);",
               use(m_name), use(m_founder), use(m_owner), use(Utils::time()), now;

        // Retrieve the last inserted ID
        Statement lastinsert = (ses << "SELECT LAST_INSERT_ID()", now);
        lastinsert.execute();
        RecordSet lsi(lastinsert);
        lsi.moveFirst();
        int64_t lsiv = lsi.value("LAST_INSERT_ID()").convert<int64_t>();
        if (lsiv > 0)
        {
            m_allianceid = lsiv;
        }
        else
        {
            spitfire::GetSingleton().log->error("Unable to create alliance.");
            return false;
        }

        return true;
    }
    SQLCATCH3(0, spitfire::GetSingleton());

    return false;
}

// Method to delete alliance data from the database
bool Alliance::DeleteFromDB()
{
    try
    {
        Poco::Data::Session ses(spitfire::GetSingleton().serverpool->get());
        ses << "DELETE FROM `alliances` WHERE id=?;", use(m_allianceid), now;
        return true;
    }
    SQLCATCH3(0, spitfire::GetSingleton());

    return false;
}

// Method to update alliance data in the database
bool Alliance::SaveToDB()
{
    try
    {
        Poco::Data::Session ses(spitfire::GetSingleton().serverpool->get());

        // Prepare the data to update in the database
        ses << "UPDATE `alliances` SET name=?,founder=?,leader=?,note=?,intro=?,motd=?,allies=?,neutrals=?,enemies=?,members=? WHERE id=?;",
               use(m_name), use(m_founder), use(m_owner), use(m_note), use(m_intro), use(m_motd),
               use(BuildStringList(m_allies)), use(BuildStringList(m_neutral)), use(BuildStringList(m_enemies)),
               use(BuildMemberList()), use(m_allianceid), now;
        return true;
    }
    SQLCATCH3(0, spitfire::GetSingleton());

    return false;
}

// Utility function to build a string list from a vector of integers
std::string Alliance::BuildStringList(const std::vector<int64_t>& ids)
{
    std::stringstream ss;
    for (size_t i = 0; i < ids.size(); ++i)
    {
        if (i > 0)
            ss << ",";
        ss << ids[i];
    }
    return ss.str();
}

// Utility function to build a member list for database storage
std::string Alliance::BuildMemberList()
{
    std::stringstream ss;
    for (auto& member : m_members)
    {
        ss << member.clientid << "," << member.rank << "|";
    }
    return ss.str();
}

// Method to convert alliance information to an AMF3 object
amf3object Alliance::ToObject()
{
    amf3object obj;
    obj["prestige"] = m_prestige;
    obj["rank"] = m_prestigerank;
    obj["creatorName"] = m_founder;
    obj["allianceNote"] = m_note;
    obj["allianceInfo"] = m_intro;
    obj["allianceName"] = m_name;
    obj["memberCount"] = m_currentmembers;
    obj["memberLimit"] = m_maxmembers;
    obj["leaderName"] = m_owner;
    return obj;
}

// Method to check if an alliance is marked as an enemy
bool Alliance::IsEnemy(int64_t allianceid)
{
    return std::find(m_enemies.begin(), m_enemies.end(), allianceid) != m_enemies.end();
}

// Method to check if an alliance is marked as an ally
bool Alliance::IsAlly(int64_t allianceid)
{
    return std::find(m_allies.begin(), m_allies.end(), allianceid) != m_allies.end();
}

// Method to check if an alliance is marked as neutral
bool Alliance::IsNeutral(int64_t allianceid)
{
    return std::find(m_neutral.begin(), m_neutral.end(), allianceid) != m_neutral.end();
}

// Method to add a member to the alliance
bool Alliance::AddMember(uint64_t clientid, uint8_t rank)
{
    if (HasMember(clientid))
        return false;

    if (m_currentmembers >= m_maxmembers)
        return false;

    stMember member;
    member.clientid = clientid;
    member.rank = rank;
    m_members.push_back(member);
    ++m_currentmembers;
    return true;
}

// Method to check if a user is a member of the alliance by username
bool Alliance::HasMember(std::string username)
{
    Client* client = spitfire::GetSingleton().GetClientByName(username);
    if (!client)
        return false;
    return HasMember(client->accountid);
}

// Method to check if a user is a member of the alliance by client ID
bool Alliance::HasMember(uint64_t clientid)
{
    return std::find_if(m_members.begin(), m_members.end(),
                        [&](const stMember& m) { return m.clientid == clientid; }) != m_members.end();
}

// Method to remove a member from the alliance by client ID
bool Alliance::RemoveMember(uint64_t clientid)
{
    auto it = std::find_if(m_members.begin(), m_members.end(),
                           [&](const stMember& m) { return m.clientid == clientid; });
    if (it != m_members.end())
    {
        m_members.erase(it);
        --m_currentmembers;
        return true;
    }
    return false;
}

// Method to handle a request to join the alliance
void Alliance::RequestJoin(Client* client, uint64_t timestamp)
{
    stInviteList invite;
    invite.invitetime = timestamp;
    invite.client = client;
    m_invites.push_back(invite);
}

// Method to cancel a join request by client object
void Alliance::UnRequestJoin(Client* client)
{
    m_invites.remove_if([&](const stInviteList& invite) { return invite.client == client; });
}

// Method to cancel a join request by client username
void Alliance::UnRequestJoin(std::string username)
{
    m_invites.remove_if([&](const stInviteList& invite) { return invite.client->playername == username; });
}

// Method to parse member information from a string
void Alliance::ParseMembers(std::string str)
{
    std::vector<std::string> tokens;
    my_split(tokens, str, "|");

    for (const auto& token : tokens)
    {
        std::vector<std::string> innerTokens;
        my_split(innerTokens, token, ",");

        if (innerTokens.size() == 2)
        {
            uint64_t clientid = std::stoull(innerTokens[0]);
            uint8_t rank = std::stoi(innerTokens[1]);

            Client* client = spitfire::GetSingleton().GetClient(clientid);
            if (client)
            {
                AddMember(clientid, rank);
                client->allianceid = this->m_allianceid;
                client->alliancerank = rank;
            }
        }
    }
}

// Method to parse relation information from a string
void Alliance::ParseRelation(std::list<int64_t>& list, std::string str)
{
    std::vector<std::string> tokens;
    my_split(tokens, str, "|");

    for (const auto& token : tokens)
    {
        int64_t allianceid = std::stoll(token);
        list.push_back(allianceid);
    }
}

// Method to mark another alliance as an ally
void Alliance::Ally(int64_t allianceid)
{
    if (IsAlly(allianceid))
        return;

    if (IsNeutral(allianceid))
        UnNeutral(allianceid);

    if (IsEnemy(allianceid))
        UnEnemy(allianceid);

    m_allies.push_back(allianceid);

    Alliance* temp = spitfire::GetSingleton().m_alliances->AllianceById(allianceid);
    temp->SendAllianceMessage("Alliance [" + temp->m_name + "] recognizes Diplomatic Relationship with us as Ally.", false, false);
}

// Method to mark another alliance as neutral
void Alliance::Neutral(int64_t allianceid)
{
    if (IsNeutral(allianceid))
        return;

    if (IsAlly(allianceid))
        UnAlly(allianceid);

    if (IsEnemy(allianceid))
        UnEnemy(allianceid);

    m_neutral.push_back(allianceid);

    Alliance* temp = spitfire::GetSingleton().m_alliances->AllianceById(allianceid);
    temp->SendAllianceMessage("Alliance [" + temp->m_name + "] recognizes Diplomatic Relationship with us as Neutral.", false, false);
}

// Method to mark another alliance as an enemy
void Alliance::Enemy(int64_t allianceid, bool skip /* = false*/)
{
    if (IsEnemy(allianceid))
        return;

    enemyactioncooldown = Utils::time() + 1000 * 60 * 60 * 24;

    if (IsNeutral(allianceid))
        UnNeutral(allianceid);

    if (IsAlly(allianceid))
        UnAlly(allianceid);

    m_enemies.push_back(allianceid);

    if (skip)
        return;

    // Send global messages and notify the other alliance
    Alliance* temp = spitfire::GetSingleton().m_alliances->AllianceById(allianceid);
    spitfire::GetSingleton().MassMessage("Alliance " + this->m_name + " declares war against alliance " + temp->m_name + ". Diplomatic Relationship between each other alters to Hostile automatically.");
    temp->Enemy(m_allianceid, true);
    temp->SendAllianceMessage("Alliance [" + m_name + "] recognizes Diplomatic Relationship with us as Enemy.", false, false);
}

// Method to remove another alliance from the ally list
void Alliance::UnAlly(int64_t allianceid)
{
    m_allies.remove(allianceid);
}

// Method to remove another alliance from the neutral list
void Alliance::UnNeutral(int64_t allianceid)
{
    m_neutral.remove(allianceid);
}

// Method to remove another alliance from the enemy list
void Alliance::UnEnemy(int64_t allianceid)
{
    m_enemies.remove(allianceid);
}

// Method to send an alliance message to all members
void Alliance::SendAllianceMessage(std::string msg, bool tv, bool nosender)
{
    amf3object obj;
    obj["cmd"] = "server.SystemInfoMsg";
    obj["data"]["alliance"] = true;
    obj["data"]["sender"] = "Alliance Msg";
    obj["data"]["tV"] = tv;
    obj["data"]["msg"] = msg;
    obj["data"]["noSenderSystemInfo"] = nosender;

    for (const auto& client : m_members)
    {
        spitfire::GetSingleton().SendObject(spitfire::GetSingleton().GetClient(client.clientid), obj);
    }
}
