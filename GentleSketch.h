#ifndef _Gentle_SKETCH_H
#define _Gentle_SKETCH_H

#include <string>
#define CHECK_TOP
#include <cstdint>
#include <algorithm>
#include <functional>
#include <iostream>
#include <limits.h>
#include <stdint.h>
#include <vector>
#include "BOBHASH64.h"
#include "BaseSketch.h"
#include "LossyStrategy.h"

namespace sketch {
    namespace GentleSketch {
        const int MAX_ROW = 2;
        const int MAX_ENTRY = 5;

        class Entry {
        public:
            Entry() : ID(""), count(0), carry(-1) {}
            Entry(const std::string& id, uint16_t fp, uint32_t cnt)
                : ID(id), count(cnt), carry(fp) {}
            uint16_t Empty();

            bool Equalcarry(uint16_t fp); 
            bool EqualID(const std::string& str); 
            void Insert();
            void Insert(uint16_t& fp, const std::string& id, uint32_t cnt = 1);
            void Lossy(std::function<void(uint32_t&)>& lossy_func);


            uint16_t get_carry();
            uint32_t get_count();
            std::string get_ID();
            void Remove();
            bool operator<(const Entry& e) const { return count > e.count; }
            void reset(uint16_t& fp);
        private:
            std::string ID;

            uint16_t carry;
            uint32_t count;
        };

        class Bucket {
        public:
            Bucket() { entries.resize(MAX_ENTRY); }
            void Clear();
            bool Empty(int index); 
            bool Full(int index);  
            void Insert(int index);
            void Insert(int index, const Entry& entry);
            void Insert(int index, uint16_t fp,
                const std::string& id); 
            
            void Lossy(int index, std::function<void(uint32_t&)>
                & lossy_func); 
            void EntrySwap(int index1,int index2);
            bool Equal(int index, uint16_t fp);
            bool Equal(int index, const std::string& str);

            uint16_t get_fp(int index);
            string get_ID(int index);
            Entry get_entry(int index);
            int get_col_index();
            uint32_t get_entry_count(int index);
            void down_stairs(int index);
            void reset(int index, uint16_t& fp);
            void Remove(int index);
        private:
            std::vector<Entry> entries;
            int col_index;
        };

        class GentleSketch : public BaseSketch {
        public:
            GentleSketch(int K, int MEM);
            ~GentleSketch();
            void clear();
            void Insert(const std::string& str) override;
            std::pair<std::string, int> Query(int k);
            
            void work();
            std::string get_name();
            

        private:
            uint64_t Hash(const std::string& str);
            uint64_t Hash(uint8_t fp);
            bool kickout(int kick_num, Bucket& cur_bucket,
                int entry_index, int array_index, uint16_t location);
            uint16_t _bucket_num;
            BOBHash64* _bobhash;
            std::vector<std::vector<Bucket>> _buckets;
            Lossy::BaseStrategy* _lossy;
            std::function<void(uint32_t&)> _lossy_func;
            std::vector<Entry> _ret;
            int _K;
            
        };
        
        const int MAX_KICK_OUT = 8;

        // Entry
        uint16_t Entry::Empty() { return count == 0; }



        bool Entry::Equalcarry(uint16_t fp) { return fp == carry; }
        bool Entry::EqualID(const std::string& str)
        {
            return str == ID;
        } 
        void Entry::Insert() { ++count; }

        void Entry::Insert(uint16_t& fp, const std::string& id, uint32_t cnt) {
            carry = fp;

            this->ID = id;
            this->count = cnt;
        }

        void Entry::Lossy(std::function<void(uint32_t&)>& lossy_func) {
            lossy_func(count);
        }



        uint16_t Entry::get_carry() { return carry; }

        uint32_t Entry::get_count() { return count; }

        std::string Entry::get_ID() { return ID; }

        void Entry::Remove() {
            carry = -1;
            this->ID = "";
            this->count = 0;
        }
        void Entry::reset(uint16_t& fp) {
            this->carry = fp;

        }
        // Bucket:
        // | entry  | carry | count|
        // | entry0 | 32bit | 32bit|
        // | entry1 | 32bit | 16bit|
        // | entry2 | 16bit | 8bit |
        // | entry3 | 16bit | 4bit |
        // | entry3 | 16bit | 4bit |
        void Bucket::Clear() {
            entries.clear();
            entries.resize(MAX_ENTRY);
        }

        bool Bucket::Empty(int index) { return entries[index].Empty(); }

        bool Bucket::Full(int index) {
            switch (index) {
            case 0:
                return entries[index].get_count() == 0xffffffff;
            case 1:
                return entries[index].get_count() == 0xffff;
            case 2:
                return entries[index].get_count() == 0xff;
            case 3:
            case 4:
                return entries[index].get_count() == 0xf;
            }
            return false;
        }

        // for match
        void Bucket::Insert(int index) { 
                if (Bucket::Full(index)) return;
                else entries[index].Insert();
         }

        // for kick entry
        void Bucket::Insert(int index, const Entry& entry) { entries[index] = entry; }
        
        // for empty entry
        void Bucket::Insert(int index, uint16_t fp, const std::string& id) {
            entries[index].Insert(fp, id);
        }
        
        

        void Bucket::Lossy(int index, std::function<void(uint32_t&)>& lossy_func) {
            entries[index].Lossy(lossy_func);
        }

        void Bucket::EntrySwap(int index1,int index2) {
            
                std::swap(entries[index1], entries[index2]);
            }
       

        bool Bucket::Equal(int index, uint16_t fp) {

            return entries[index].Equalcarry(fp);


           
        }
        bool Bucket::Equal(int index, const std::string& str) {

            return entries[index].EqualID(str);
 
        }
        uint16_t Bucket::get_fp(int index) {
           
            return entries[index].get_carry();
        }
        string Bucket::get_ID(int index) {
           
            return entries[index].get_ID();
        }

        Entry Bucket::get_entry(int index) { return entries[index]; }

        int Bucket::get_col_index() { return col_index; }

        uint32_t Bucket::get_entry_count(int index) { return entries[index].get_count(); }

        void Bucket::reset(int index, uint16_t& fp) {
            entries[index].reset(fp);
        }
        void Bucket::Remove(int index) {
            entries[index].Remove();
        }
        // GentleSketch
        GentleSketch::GentleSketch(int K, int MEM) {
            const int BUCKETSIZE = 176;
            _bucket_num = 0;
            for (; _bucket_num * BUCKETSIZE * MAX_ROW <= MEM * 1024 * 8; ++_bucket_num);
            --_bucket_num;
            _buckets.resize(MAX_ROW, std::vector<Bucket>(_bucket_num));
            _K = K;
          
            _bobhash = new BOBHash64(1005);
            _lossy_func = Lossy::MinusOneStrategy{};
        }

        GentleSketch::~GentleSketch() {
            // delete _bobhash;
        }

        void GentleSketch::clear() {
            _buckets.clear();
            _buckets.resize(MAX_ROW, std::vector<Bucket>(_bucket_num));
        }


    
        void GentleSketch::Insert(const std::string& str) {
          
           
            uint64_t hash_key = Hash(str);
            uint32_t fp = hash_key >> 32;

            uint32_t n = fp % (_bucket_num * (_bucket_num - 1));
            uint16_t keys[2] = { n % _bucket_num, n / _bucket_num };

            Bucket& bucket0 = _buckets[0][keys[0]];
            Bucket& bucket1 = _buckets[1][keys[1]];

            
            // entry 0:
            if (bucket0.Equal(0, str)) {
                bucket0.Insert(0);
                return;
            }
            if (bucket0.Empty(0)) {
                bucket0.Insert(0, keys[1], str);
                return;
            }

            
            if (bucket1.Equal(0, str)) {
                bucket1.Insert(0);
                return;
            }
            if (bucket1.Empty(0)) {
                bucket1.Insert(0, keys[0], str);
                return;
            }

            
            
            if (bucket0.Equal(1, str)) {
                
                
                if(bucket0.Full(1)){
                    
                    if(bucket0.get_entry_count(0)<bucket0.get_entry_count(1)){
                        bucket0.EntrySwap(0,1);
                        bucket0.Insert(0);
                    }
                    else{
                        bool flag=kickout(MAX_KICK_OUT, bucket0, 1, 0, keys[0]);
                       
                    }
                }else{
                    bucket0.Insert(1);
                }
                
                return;
            }
            if (bucket0.Empty(1)) {
                bucket0.Insert(1, keys[1], str);
                return;
            }

            
            if (bucket1.Equal(1, str)) {
                
                
                if(bucket1.Full(1)){
                    if(bucket1.get_entry_count(0)<bucket1.get_entry_count(1)){
                        bucket1.EntrySwap(0,1);
                        bucket1.Insert(0);
                    }
                    else{
                        bool flag=kickout(MAX_KICK_OUT, bucket1, 1, 1, keys[1]);
                        
                    }
                }else{
                    bucket1.Insert(1);
                }
                return;
            }
            if (bucket1.Empty(1)) {
                bucket1.Insert(1, keys[0], str);
                return;
            }

            
            for (int i = 2; i < MAX_ENTRY; i++) {
                
                if (bucket0.Equal(i, keys[1])) {
                    
                    if(bucket0.Full(i)){
                        for(int k=i-1;k>=0;k--){
                            if(bucket0.get_entry_count(k)<bucket0.get_entry_count(i)){
                                bucket0.EntrySwap(k,i);
                                bucket0.Insert(k);
                                return;
                            }
                        }
                       
                        
                        bool flag=kickout(MAX_KICK_OUT, bucket0, i, 0, keys[0]);
                               
                    }else{
                        bucket0.Insert(i);
                    }
                    return;
                }
                if (bucket0.Empty(i)) {
                    bucket0.Insert(i, keys[1], str);
                    return;
                }
                
                if (bucket1.Equal(i, keys[0])) {
                    
                    if(bucket1.Full(i)){
                        for(int k=i-1;k>=0;k--){
                            if(bucket1.get_entry_count(k)<bucket1.get_entry_count(i)){
                                bucket1.EntrySwap(k,i);
                                bucket1.Insert(k);
                                return;
                            }
                        }
                        bool flag=kickout(MAX_KICK_OUT, bucket1, i, 1, keys[1]); 
                        
                    }else{
                        bucket1.Insert(i);
                    }
                    return;
                }
                if (bucket1.Empty(i)) {
                    bucket1.Insert(i, keys[0], str);
                    return;
                }
            }
            
            
            if (bucket0.get_entry_count(MAX_ENTRY - 1) <
                bucket1.get_entry_count(MAX_ENTRY - 1)) {
                bucket0.Lossy(MAX_ENTRY - 1, _lossy_func);
            }
            else {
                bucket1.Lossy(MAX_ENTRY - 1, _lossy_func);
            }
        }
    
        std::pair<std::string, int> GentleSketch::Query(int k) {
            if (k < _ret.size()) {
                return std::make_pair(_ret[k].get_ID(), _ret[k].get_count());
            }
            return { 0, 0 };
        }
        void GentleSketch::work() {
            int k=0;
            _ret.resize(_bucket_num * MAX_ROW*2);
            for (int i = 0; i < MAX_ROW; i++) {
                for (int j = 0; j < _bucket_num;j++) {
                    _ret[k++] = _buckets[i][j].get_entry(0);
                    _ret[k++] = _buckets[i][j].get_entry(1);
                    
                }
            }
            sort(_ret.begin(), _ret.end());
        }

        std::string GentleSketch::get_name() { return "GentleSketch"; }

        uint64_t GentleSketch::Hash(const std::string& str) {
            return _bobhash->run(str.c_str(), str.length());
        }

        uint64_t GentleSketch::Hash(uint8_t fp) {
            return _bobhash->run((char*)&fp, 1);
        }

        bool GentleSketch::kickout(int kick_num,
            Bucket& cur_bucket, int entry_index,
            int array_index, uint16_t location) {
            
            if (kick_num == 0) {
                int index=entry_index;
                uint16_t fp = cur_bucket.get_fp(entry_index);
                Bucket& next_bucket =_buckets[1 - array_index][fp];
                if(next_bucket.get_entry(MAX_ENTRY-1)<cur_bucket.get_entry(entry_index)){
                    next_bucket.reset(MAX_ENTRY-1,location);
                    cur_bucket.Remove(entry_index);
                }
                return true;
            }
            int index=entry_index;
            uint16_t fp = cur_bucket.get_fp(entry_index);
            if(cur_bucket.get_entry_count(entry_index)<0xf){
                index=MAX_ENTRY-1;
            }
            else if(cur_bucket.get_entry_count(entry_index)<0xff){
                index=2;
            }
            else if(cur_bucket.get_entry_count(entry_index)<0xffff){
                index=1;
            }
            else{
                index=0;
            }

            Bucket& next_bucket =_buckets[1 - array_index][fp];

            for(int k=0;k<=index;k++){
                if (next_bucket.Empty(k)) {
                    cur_bucket.reset(entry_index, location);
                    next_bucket.Insert(k, cur_bucket.get_entry(entry_index));
                    next_bucket.Insert(k);
                    cur_bucket.Remove(entry_index);
                    return true;
                }

            }
            for(int k=0;k<=index;k++){
                if (next_bucket.get_entry_count(k)<cur_bucket.get_entry_count(entry_index)) {
                        //
                        if (kickout(kick_num - 1, next_bucket, k ,1 - array_index, fp)) {
                            cur_bucket.reset(entry_index, location);
                            next_bucket.Insert(k, cur_bucket.get_entry(entry_index));
                            next_bucket.Insert(k);
                            cur_bucket.Remove(entry_index);
                            return true;
                        }
                        
                    }
              
            }
            if (kickout(kick_num - 1, next_bucket, index ,1 - array_index, fp)) {
                cur_bucket.reset(entry_index, location);
                next_bucket.Insert(index, cur_bucket.get_entry(entry_index));
                next_bucket.Insert(index);
                cur_bucket.Remove(entry_index);
                return true;
            }
            return false;
        }
      
       
    } // namespace GentleSketch
} // namespace sketch

#endif