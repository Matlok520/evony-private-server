/* Copyright (C) Daisy - All Rights Reserved
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
* Written by Daisy <daisy@spitfire.pw>, February 2018
*/

/* 
 * This header file contains the declaration of the Market class along with its member functions and data structures.
 * It is copyrighted by Daisy and unauthorized copying is strictly prohibited.
 * Written by Daisy <daisy@spitfire.pw> in February 2018.
 */

#pragma once // Ensures this header file is included only once during compilation

#include "structs.h" // Include necessary external header file

#include <mutex> // Include the mutex header for thread synchronization
#include <memory> // Include the memory header for smart pointers

class Market
{
private:
    // Define a struct to hold various market entries for different resources
    struct MarketEntries
    {
        std::list<stMarketEntry> foodBuy;
        std::list<stMarketEntry> foodSell;
        std::list<stMarketEntry> woodBuy;
        std::list<stMarketEntry> woodSell;
        std::list<stMarketEntry> stoneBuy;
        std::list<stMarketEntry> stoneSell;
        std::list<stMarketEntry> ironBuy;
        std::list<stMarketEntry> ironSell;
    };

    // Mutexes for thread-safe access to market and cache data
    std::mutex marketMutex;
    std::mutex cacheMutex;

    uint64_t tradeId; // Unique ID for trades

    MarketEntries liveEntries; // Struct to hold live market entries
    MarketEntries queueEntries; // Struct to hold queued market entries

    // Struct to cache market data for optimization
    struct MarketCache
    {
        std::list<stMarketEntry> foodBuy;
        std::list<stMarketEntry> foodSell;
        std::list<stMarketEntry> woodBuy;
        std::list<stMarketEntry> woodSell;
        std::list<stMarketEntry> stoneBuy;
        std::list<stMarketEntry> stoneSell;
        std::list<stMarketEntry> ironBuy;
        std::list<stMarketEntry> ironSell;

        uint64_t buyTime = 0;
        uint64_t sellTime = 0;
    } cache;

public:
    // Default constructor and destructor
    Market() : tradeId(0) {}
    ~Market() = default;

    // Function prototypes for initializing market, recalculating cache, and processing market data
    bool init();
    void recalculateCache();
    void Process();

    // Static functions for comparing market entry prices
    static bool comparePrice(const stMarketEntry& first, const stMarketEntry& second);
    static bool rcomparePrice(const stMarketEntry& first, const stMarketEntry& second);

    // Functions to add buy/sell entries for each resource type
    bool addFoodBuy(const stMarketEntry& entry);
    bool addFoodSell(const stMarketEntry& entry);
    bool addWoodBuy(const stMarketEntry& entry);
    bool addWoodSell(const stMarketEntry& entry);
    bool addStoneBuy(const stMarketEntry& entry);
    bool addStoneSell(const stMarketEntry& entry);
    bool addIronBuy(const stMarketEntry& entry);
    bool addIronSell(const stMarketEntry& entry);

    // Functions to retrieve buy/sell entries for each resource type
    std::list<stMarketEntry>& getFoodBuy();
    std::list<stMarketEntry>& getFoodSell();
    std::list<stMarketEntry>& getWoodBuy();
    std::list<stMarketEntry>& getWoodSell();
    std::list<stMarketEntry>& getStoneBuy();
    std::list<stMarketEntry>& getStoneSell();
    std::list<stMarketEntry>& getIronBuy();
    std::list<stMarketEntry>& getIronSell();

private:
    // Private function to update the market cache
    void updateCache();
};
```
```
