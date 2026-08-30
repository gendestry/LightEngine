#pragma once
#include <cassert>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include "Utils/Colors/Font.h"
#include "Utils/Traits/Stringify.h"

namespace LightEngine::Engine
{

template <class T>
class Pool : public Utils::Traits::Stringify
{
    using Ptr = std::shared_ptr<T>;
    std::map<uint32_t, Ptr> m_items;
    std::unordered_map<std::string, uint32_t> m_byName;
    std::string m_name;

    void index(const std::string &name, uint32_t num)
    {
        if (!name.empty())
        {
            m_byName[name] = num;
        }
    }
    void unindex(const std::string &name) { m_byName.erase(name); }

public:
    Pool(std::string name = "PoolObject") : m_name(name) {}
    // ---- store ----------------------------------------------------------

    // Store into an explicit slot. Overwrites whatever was there.
    // Stamps the object's number so object and key always agree.
    T &store(uint32_t num, Ptr obj)
    {
        assert(obj && "Pool::store: null object");
        if (auto old = get(num)) // overwriting: drop the replaced name
            unindex(old->name());
        obj->setNumber(num);
        index(obj->name(), num); // pick up any name it arrived with
        auto &slot = m_items[num];
        slot = std::move(obj);
        return *slot;
    }

    // Store into the next free slot.
    T &store(Ptr obj) { return store(nextFree(), std::move(obj)); }

    bool rename(uint32_t num, const std::string &name)
    {
        auto it = m_items.find(num);
        if (it == m_items.end())
            return false;
        unindex(it->second->name());
        it->second->setName(name); // allowed: Pool is a friend
        index(name, num);
        return true;
    }

    // Construct a T in place at an explicit slot / the next free slot.
    // Note: emplaceAt vs emplace are deliberately distinct names - a single
    // overload set can't disambiguate `emplace(5)` (slot 5? or ctor arg 5?).
    template <class... Args>
    T &emplaceAt(uint32_t num, Args &&...args)
    {
        return store(num, std::make_shared<T>(std::forward<Args>(args)...));
    }
    template <class... Args>
    T &emplace(Args &&...args)
    {
        return store(nextFree(),
                     std::make_shared<T>(std::forward<Args>(args)...));
    }

    // ---- lookup ---------------------------------------------------------

    [[nodiscard]] Ptr get(uint32_t num) const
    {
        auto it = m_items.find(num);
        return it == m_items.end() ? nullptr : it->second;
    }

    [[nodiscard]] bool contains(uint32_t num) const
    {
        return m_items.count(num) != 0;
    }

    [[nodiscard]] Ptr get(const std::string &name) const
    {
        auto it = m_byName.find(name);
        return it == m_byName.end() ? nullptr : get(it->second);
    }

    // [[nodiscard]] bool contains(uint32_t num) const
    // {
    //     return m_items.count(num) != 0;
    // }

    // ---- mutate ---------------------------------------------------------

    // Remove a slot. Returns true if something was erased.
    bool remove(uint32_t num)
    {
        auto it = m_items.find(num);
        if (it == m_items.end())
            return false;
        unindex(it->second->name());
        m_items.erase(it);
        return true;
    }

    void clear()
    {
        m_items.clear();
        m_byName.clear();
    }

    // Move an object to a new number (e.g. "Move Group 3 at 10").
    // No-op if `from` is empty; overwrites `to` if occupied.
    bool move(uint32_t from, uint32_t to)
    {
        if (from == to)
            return contains(from);
        auto it = m_items.find(from);
        if (it == m_items.end())
            return false;
        store(to, std::move(it->second)); // re-stamps number + reindexes name
        m_items.erase(it);
        return true;
    }

    // ---- numbering ------------------------------------------------------

    // First gap in the ascending key sequence (1-indexed).
    [[nodiscard]] uint32_t nextFree() const
    {
        uint32_t n = 1;
        for (const auto &[num, _] : m_items)
        {
            if (num > n)
                break; // gap: n is free
            if (num == n)
                ++n; // taken: advance
        }
        return n;
    }

    // ---- query / iterate ------------------------------------------------

    [[nodiscard]] std::size_t size() const { return m_items.size(); }
    [[nodiscard]] bool empty() const { return m_items.empty(); }
    [[nodiscard]] const std::string &name() const { return m_name; }

    // Sorted iteration by number (map guarantees ascending order).
    [[nodiscard]] auto begin() const { return m_items.begin(); }
    [[nodiscard]] auto end() const { return m_items.end(); }

    [[nodiscard]] const std::map<uint32_t, Ptr> &items() const
    {
        return m_items;
    }

    // Multi-line dump: a header with the count, then each item's own describe()
    // indented, in ascending number order.
    [[nodiscard]] std::string toString() const override
    {
        std::string s = Utils::Font::bold + "Pool" + Utils::Font::reset +
                        Utils::Font::colorDim + " [" +
                        std::to_string(m_items.size()) + "]" +
                        Utils::Font::reset;
        for (const auto &[num, obj] : m_items)
        {
            s += "\n  " + obj->toString();
        }
        return s;
    }
};

} // namespace LightEngine::Engine