#include "blueprint/Connecting.h"
#include "blueprint/Pins.h"
#include "blueprint/NodeLayout.h"

#include <SM_Calc.h>

namespace bp
{

Connecting::Connecting(const std::shared_ptr<Pins>& from,
	                   const std::shared_ptr<Pins>& to)
	: m_from(from)
	, m_to(to)
{
	UpdateCurve();
}

void Connecting::UpdateCurve()
{
	UpdateCurveShape();
	UpdateCurveColor();
}

void Connecting::UpdateCurveShape()
{
	sm::vec2 v0 = NodeLayout::GetPinsPos(*m_from);
	sm::vec2 v3 = NodeLayout::GetPinsPos(*m_to);
	float d = fabs((v3.x - v0.x) * NodeLayout::CONNECTING_BEZIER_DIST);
	auto v1 = v0 + sm::vec2(d, 0);
	auto v2 = v3 - sm::vec2(d, 0);
	m_curve.shape.SetCtrlPos(v0, v1, v2, v3);
}

void Connecting::UpdateCurveColor()
{
	m_curve.color.clear();

	if (m_from->GetType() == m_to->GetType()) {
		return;
	}

	auto& vertices = m_curve.shape.GetVertices();
	int n = vertices.size();

	m_curve.color.resize(n);
	std::vector<float> lens;
	lens.resize(n - 1);
	float tot_len = 0;
	for (int i = 0; i < n - 1; ++i)
	{
		float len = sm::dis_pos_to_pos(vertices[i], vertices[i+1]);
		lens[i] = len;
		tot_len += len;
	}
	float curr_len = 0;

	auto& s = m_from->GetColor();
	auto& d = m_to->GetColor();
	sm::vec4 d_col;
	d_col.x = (d.r - s.r) / tot_len;
	d_col.y = (d.g - s.g) / tot_len;
	d_col.z = (d.b - s.b) / tot_len;
	d_col.w = (d.a - s.a) / tot_len;
	for (int i = 0; i < n; ++i)
	{
		float p = curr_len / tot_len;
		auto c4 = d_col * curr_len;
		pt2::Color col = m_from->GetColor();
		col.r += static_cast<int>(c4.x);
		col.g += static_cast<int>(c4.y);
		col.b += static_cast<int>(c4.z);
		col.a += static_cast<int>(c4.w);
		m_curve.color[i] = col.ToABGR();
		if (i != n - 1) {
			curr_len += lens[i];
		}
	}
}

std::shared_ptr<Connecting> make_connecting(const std::shared_ptr<Pins>& from,
	                                        const std::shared_ptr<Pins>& to)
{
	auto& to_conn = to->GetConnecting();
	if (!to_conn.empty())
	{
		assert(to_conn.size() == 1);
		auto& conn = to_conn[0];
		if (conn->GetFrom() == from &&
			conn->GetTo() == to)
		{
			return conn;
		}
		else
		{
			conn->GetFrom()->RemoveConnecting(conn);
			conn->GetTo()->RemoveConnecting(conn);
		}
	}

	auto conn = std::make_shared<Connecting>(from, to);
	from->AddConnecting(conn);
	to->AddConnecting(conn);
	return conn;
}

void disconnect(const std::shared_ptr<Connecting>& conn)
{
	auto& f = conn->GetFrom();
	auto& t = conn->GetTo();

	f->RemoveConnecting(conn);
	t->RemoveConnecting(conn);

	auto& fn = f->GetParent();
	if (fn.IsStyleOnlyTitle()) {
		fn.SetStyleOnlyTitle(false);
		bp::NodeLayout::UpdateNodeStyle(const_cast<Node&>(fn));
		fn.SetSizeChanging(true);
	}
	auto& tn = t->GetParent();
	if (tn.IsStyleOnlyTitle()) {
		tn.SetStyleOnlyTitle(false);
		bp::NodeLayout::UpdateNodeStyle(const_cast<Node&>(tn));
		tn.SetSizeChanging(true);
	}
}

}