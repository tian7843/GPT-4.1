#pragma once
#include <QString>
#include <vector>
class DcmLoader {
public:
    // 读取16位灰度DICOM
    static bool loadDcm(const QString& fname, int& w, int& h, std::vector<uint16_t>& data);
};
