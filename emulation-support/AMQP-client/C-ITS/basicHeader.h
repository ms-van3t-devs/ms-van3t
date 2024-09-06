//
// Created by carlos on 08/12/23.
//

#ifndef ZMQ_PROXY_BASICHEADER_H
#define ZMQ_PROXY_BASICHEADER_H
#include <cstdint>
#include <vector>

class basicHeader {
public:
    basicHeader();
    ~basicHeader();
    void removeHeader(unsigned char * buffer);

    //Getters
    [[nodiscard]] uint8_t GetVersion() const {return m_version;}
    [[nodiscard]] uint8_t GetNextHeader() const {return m_nextHeader;}
    [[nodiscard]] uint8_t GetLifeTime() const {return m_lifeTime;}
    [[nodiscard]] uint8_t GetRemainingHL() const{return m_remainingHopLimit;}

    //Setters
    void SetVersion(uint8_t version) {m_version = version;}
    void SetNextHeader(uint8_t NH) {m_nextHeader = NH;}
    void SetLifeTime(uint8_t LT) {m_lifeTime = LT;}
    void SetRemainingHL(uint8_t RHL){m_remainingHopLimit = RHL;}

    void addHeader(std::vector<unsigned char>& buffer);


private:
    uint8_t m_version: 4;
    uint8_t m_nextHeader: 4;
    uint8_t m_reserved;
    uint8_t m_lifeTime;
    uint8_t m_remainingHopLimit;
};


#endif //ZMQ_PROXY_BASICHEADER_H
