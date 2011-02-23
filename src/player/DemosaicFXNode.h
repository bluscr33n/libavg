#ifndef _DemosaicFXNode_H_
#define _DemosaicFXNode_H_

#include "../api.h"

#include "FXNode.h"
#include "../graphics/GPUDemosaic.h"

#include <boost/shared_ptr.hpp>

namespace avg {

class SDLDisplayEngine;

class AVG_API DemosaicFXNode: public FXNode {
public:
    DemosaicFXNode();
    virtual ~DemosaicFXNode();

    virtual void disconnect();
    void setParam(double stdDev);

private:
    virtual GPUFilterPtr createFilter(const IntPoint& size);

    GPUDemosaicPtr m_pFilter;

    double m_StdDev;
};

typedef boost::shared_ptr<DemosaicFXNode> DemosaicFXNodePtr;

}

#endif
