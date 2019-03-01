#include "blueprint/ArrangeNodeOP.h"
#include "blueprint/TranslateNodeState.h"

#include <ee0/WxStagePage.h>

namespace bp
{

ArrangeNodeOP::ArrangeNodeOP(const std::shared_ptr<pt0::Camera>& camera,
                             ee0::WxStagePage& stage, ECS_WORLD_PARAM const ee2::ArrangeNodeCfg& cfg,
                             const std::shared_ptr<ee0::EditOP>& prev_op)
    : ee2::ArrangeNodeOP(camera, stage, cfg, prev_op)
{
    m_impl->SetTranslateOP(std::make_shared<TranslateNodeState>(stage, camera));
}

}