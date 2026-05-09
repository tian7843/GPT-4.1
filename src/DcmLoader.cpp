#include "DcmLoader.h"
#include <dcmtk/dcmdata/dctk.h>

bool DcmLoader::loadDcm(const QString& fname, int& w, int& h, std::vector<uint16_t>& data) {
    DcmFileFormat file;
    OFCondition cond = file.loadFile(fname.toStdString().c_str());
    if(!cond.good()) return false;
    DcmDataset *dataset = file.getDataset();
    Uint16 rows=0, cols=0;
    dataset->findAndGetUint16(DCM_Rows, rows);
    dataset->findAndGetUint16(DCM_Columns, cols);
    w=cols; h=rows;
    // 灰度数据
    Uint16 *pixelData=NULL; Uint32 count=0;
    dataset->findAndGetUint16Array(DCM_PixelData, pixelData, &count);
    if(!pixelData || count<w*h) return false;
    data.resize(w*h);
    for(int i=0;i<w*h;++i) data[i]=pixelData[i];
    return true;
}
