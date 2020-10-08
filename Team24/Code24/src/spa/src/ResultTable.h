#pragma once

#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace qpbackend {
/**
 * The hashing method of a pair of strings
 */
struct StringVectorHash {
    std::size_t operator()(const std::vector<std::string>& v) const {
        std::string combined = "";
        for (const auto& str : v) {
            combined += str;
            combined += "&";
        }
        return std::hash<std::string>()(combined);
    }
};

/**
 * string vector comparison
 */
struct CompareStrVec {
    bool operator()(const std::vector<std::string>& v1, const std::vector<std::string>& v2) const {
        if (v1.size() != v2.size()) {
            return v1.size() < v2.size();
        }
        for (size_t idx = 0; idx < v1.size(); idx++) {
            if (v1[idx] != v2[idx]) {
                return v1[idx] < v2[idx];
            }
        }
        return false; // equal
    }
};

/**
 * The data structure of result table
 */
class ResultTable {
  public:
    ResultTable() : colNum(0), rowNum(0), isInitialized(false) {
    }
    ResultTable(const std::string& synName, const std::unordered_set<std::string>& vals);
    ResultTable(const std::vector<std::string>& synNames,
                const std::unordered_set<std::vector<std::string>, StringVectorHash>& listOfTuples);

    bool isEmpty() const;

    /**
     * @param synonymName : the name of the synonym
     * @return : whether the synonym is in the table
     */
    bool isSynonymContained(const std::string& synonymName) const;

    /**
     * merge another table
     * @param other : another table to merge
     * @return whether the table is non-empty after merging
     */
    bool mergeTable(ResultTable&& other);

    /**
     * update the set of values of a synonym
     * @param synonymName : the name of the synonym
     * @param result : the set assumed to keep all values of the synonym
     * @return : whether the synonym name is in the table and the set has been updated
     */
    bool updateSynonymValueSet(const std::string& synonymName, std::unordered_set<std::string>& result) const;
    bool updateSynonymValueVector(const std::string& synonymName, std::vector<std::string>& result) const;

    /**
     * update the set of tuples of a group of synonyms
     * @param synonmNames : the list of synonyms inquired; tuples returned will comply with the order
     * @param result : the set assumed to keep all tuples of values of the synonyms
     * @return : whether the synonym name is in the table and the set has been updated
     */
    bool updateSynonymValueTupleSet(const std::vector<std::string>& synonymNames,
                                    std::unordered_set<std::vector<std::string>, StringVectorHash>& result) const;
    bool updateSynonymValueTupleVector(const std::vector<std::string>& synonymNames,
                                       std::vector<std::vector<std::string>>& result) const;

    // for testing only
    bool operator==(const ResultTable& other) const;
    friend std::ostream& operator<<(std::ostream& os, const ResultTable& rt);
    void sortTable();

  private:
    int colNum;
    int rowNum;
    bool isInitialized;
    std::unordered_map<std::string, int> colIndexTable; // table to store mapping between table and column index
    std::vector<std::vector<std::string>> table; // the main table

    /**
     * organize the table by a single column
     * @param index : the index of the column
     * @return : a map that map values of the column to the row indices
     */
    std::unordered_map<std::string, std::vector<int>> groupTableByProperty(int index) const;

    /**
     * organize the table by values of several columnss
     * e.g. suppose we want to organize the rows by the 1st and 4th column
     * this method will return a map
     * where the key is pairs of values of 1st and 4th columns,
     * the value is the list of index of vectors
     * @param indices : indices to look up
     * @return : a map that map values of the group of columns to row indices
     */
    std::unordered_map<std::vector<std::string>, std::vector<int>, StringVectorHash>
    groupTableByProperties(std::vector<int> indices) const;
};

void handleError(const std::string& msg);
} // namespace qpbackend
