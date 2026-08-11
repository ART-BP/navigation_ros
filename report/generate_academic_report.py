#!/usr/bin/env python3

import os

import uno

from generate_navigation_report import (
    BLACK,
    BLUE,
    GOLD,
    GRAY,
    GREEN,
    LIGHT_BLUE,
    LIGHT_GOLD,
    LIGHT_GRAY,
    LIGHT_GREEN,
    LIGHT_RED,
    LIGHT_TEAL,
    NAVY,
    PALE_BLUE,
    RED,
    TEAL,
    WHITE,
    FONT_SANS,
    Writer,
    configure_document_styles,
    connect_office,
    prop,
    set_if_available,
    style_all_text,
)
from com.sun.star.style.ParagraphAdjust import CENTER, LEFT
from com.sun.star.text.ControlCharacter import PARAGRAPH_BREAK


ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
OUTPUT_DIR = os.path.join(ROOT, "report")
DOCX_PATH = os.path.join(
    OUTPUT_DIR, "面向动态会车与特殊地形的移动机器人分层自主导航方法_学术汇报版.docx"
)
PDF_PATH = os.path.join(
    OUTPUT_DIR, "面向动态会车与特殊地形的移动机器人分层自主导航方法_学术汇报版.pdf"
)


def update_academic_header(doc):
    page_styles = doc.StyleFamilies.getByName("PageStyles")
    page_names = list(page_styles.ElementNames)
    default_name = (
        "Default Page Style"
        if "Default Page Style" in page_names
        else page_names[0]
    )
    page = page_styles.getByName(default_name)
    header = page.HeaderText
    header.String = "面向动态会车与特殊地形的移动机器人分层自主导航方法研究"
    cursor = header.createTextCursor()
    cursor.gotoEnd(True)
    style_all_text(cursor, FONT_SANS, 8.5, GRAY)
    cursor.ParaAdjust = 1


def add_cover(w):
    w.paragraph("", style="Text Body", indent=False, bottom=850)
    w.paragraph(
        "面向动态会车与特殊地形的",
        style="Title",
        align=CENTER,
        indent=False,
        size=25.0,
        color=NAVY,
        bold=True,
        bottom=150,
    )
    w.paragraph(
        "移动机器人分层自主导航方法研究",
        style="Title",
        align=CENTER,
        indent=False,
        size=27.0,
        color=BLUE,
        bold=True,
        bottom=420,
    )
    w.paragraph(
        "算法建模、行为决策与系统验证",
        style="Subtitle",
        align=CENTER,
        indent=False,
        size=14.5,
        color=TEAL,
        bottom=850,
    )
    w.table(
        [],
        [
            ["研究对象", "园区长距离巡航中的动态车辆会车与台阶通行"],
            ["核心方法", "动态轨迹优化与场景级混合决策相结合的分层导航"],
            ["代码基础", "navigation_ws/src"],
            ["日期", "2026 年 7 月"],
        ],
        widths=[22, 78],
        font_size=10.5,
        first_col_bold=True,
        row_colors=[LIGHT_BLUE, WHITE, LIGHT_GRAY, WHITE],
    )
    w.paragraph("", style="Text Body", indent=False, bottom=420)
    w.paragraph(
        "汇报人：____________________        指导教师：____________________",
        style="Text Body",
        align=CENTER,
        indent=False,
        size=11.0,
        color=GRAY,
        bottom=200,
    )


def add_abstract(w):
    w.heading("摘要", 1)
    w.paragraph(
        "面向园区长距离巡航，传统导航系统能够完成定位、全局路径搜索和局部避障，"
        "但难以直接表达窄路会车、主动靠边、等待车辆通过及特殊地形通行等场景规则。"
        "为此，本文提出一种分层自主导航方法：底层以 TEB 为主要局部轨迹优化器，"
        "利用带速度的动态目标完成时空避障；上层构建道路让行混合状态机，"
        "依据道路几何、目标语义、相对纵向位置、相对运动方向和滑动时间窗识别有效来车，"
        "并依次执行巡航中断、避让点选择、靠边等待、安全清空和路线恢复。"
    )
    w.paragraph(
        "在算法实现上，本文以道路横截面序列构造分段四边形走廊，"
        "以机器人指向当前导航目标的单位向量建立局部道路语义坐标，"
        "从而使同一套道路数据适用于折返和反向巡航。"
        "为提高行为可靠性，提出最近候选排序、距离进展看门狗、事件内黑名单和安全停止机制。"
        "针对台阶场景，设计感知—规划联合切换变量，同时调整前方点云高度阈值与 TEB 运动参数，"
        "并在正常退出时恢复原配置。"
    )
    w.paragraph(
        "实验采用纯算法单元测试、动态障碍仿真和 A1/A2 路线验证。"
        "道路几何、来车判定、时间窗和候选切换等核心测试均通过；"
        "实车部署记录表明系统能够完成带台阶巡航和道路来车让行。"
        "结果说明，该方法将连续轨迹优化与离散行为决策有效解耦，"
        "在保持 ROS Navigation 兼容性的同时，提高了复杂场景导航的可解释性、可恢复性与可迁移性。"
    )
    w.paragraph(
        "关键词：移动机器人；分层导航；动态障碍；道路让行；混合状态机；TEB；特殊地形",
        style="Text Body",
        align=LEFT,
        indent=False,
        size=10.5,
        color=BLUE,
        bold=True,
        bottom=180,
    )

    w.heading("主要学术贡献", 1)
    w.table(
        ["贡献", "方法概述", "理论或实际意义"],
        [
            [
                "目标导向的道路语义坐标",
                "以机器人—当前目标方向定义前后关系和车辆纵向速度",
                "避免固定道路方向在折返任务中的失效，提高道路数据复用性",
            ],
            [
                "多约束来车确认模型",
                "联合道路区域、类别、位置、速度方向和 K-of-W 时间窗",
                "将几何可见目标转化为具有交通意义的行为触发条件",
            ],
            [
                "轨迹层—行为层双层决策",
                "TEB 处理连续时空避障，混合状态机处理靠边、等待和恢复",
                "解决局部规划器难以表达完整会车策略的问题",
            ],
            [
                "故障感知型避让与地形切换",
                "进展看门狗、候选黑名单、安全停止及感知—规划联合台阶模式",
                "提升阻塞、感知中断和特殊地形条件下的可恢复性",
            ],
        ],
        widths=[24, 40, 36],
        font_size=9.0,
        first_col_bold=True,
        row_colors=[LIGHT_BLUE, LIGHT_GOLD, LIGHT_TEAL, LIGHT_GREEN],
    )


def add_toc(doc, w):
    w.page_break()
    toc = doc.createInstance("com.sun.star.text.ContentIndex")
    toc.Title = "目录"
    set_if_available(toc, "CreateFromOutline", True)
    set_if_available(toc, "Level", 3)
    w.end()
    w.text.insertTextContent(w.cursor, toc, False)
    w.end()
    w.text.insertControlCharacter(w.cursor, PARAGRAPH_BREAK, False)
    w.paragraph(
        "目录可在 Word 或 LibreOffice 中更新。",
        style="Text Body",
        align=LEFT,
        indent=False,
        size=9.3,
        color=GRAY,
        bottom=120,
    )
    w.page_break()
    return toc


def add_problem_formulation(w):
    w.heading("1  研究问题与总体思路", 1)
    w.heading("1.1  研究背景", 2)
    w.paragraph(
        "移动机器人在结构化室内环境中通常采用“定位—全局规划—局部规划—控制”链路。"
        "当任务扩展到开放园区，环境中出现车辆、道路边界、靠边区域和台阶，"
        "导航问题便由单纯的几何可达性转化为带场景规则的序贯决策问题。"
        "机器人不仅要避免碰撞，还要判断何时让行、向何处让行、何时恢复以及异常时如何停止。"
    )
    w.heading("1.2  问题定义", 2)
    w.paragraph(
        "给定静态地图 M、巡航目标序列 G、道路走廊 R、候选避让点集合 Q，"
        "以及时变动态目标集合 O(t)，目标是在满足碰撞约束和任务规则的前提下，"
        "生成控制序列 U，使机器人完成巡航，并在检测到有效来车时执行安全让行。"
    )
    w.formula(
        "min  J_task(U) + λ₁J_motion(U) + λ₂J_risk(U, O)",
        "J_task 表示路线完成代价，J_motion 表示时间与控制平滑性，J_risk 表示静态及动态碰撞风险。",
    )
    w.formula(
        "s.t.  xₖ₊₁ = f(xₖ,uₖ)， d(xₖ,M∪Oₖ) ≥ d_min， H(xₖ,Oₖ,qₖ)=0",
        "H 表示会车、等待、地形切换和恢复等离散行为约束。",
    )
    w.paragraph(
        "该问题同时包含连续变量与离散状态：机器人位姿和速度连续变化，"
        "巡航、让行准备、靠边、等待、退出和安全停止则属于离散模式。"
        "因此，本文采用分层优化与混合状态机相结合的方法。"
    )

    w.heading("1.3  研究假设与适用边界", 2)
    w.paragraph(
        "为使问题可计算并与当前系统实现一致，本文采用以下假设。"
        "这些假设限定了结论的适用范围，也为后续扩展提供明确起点。"
    )
    w.table(
        ["假设", "内容", "影响"],
        [
            [
                "A1 定位可用",
                "机器人能够通过 TF 获得地图坐标系下的二维位置",
                "定位短时缺失时暂停进展判定，不生成新的空间结论",
            ],
            [
                "A2 目标短时可跟踪",
                "动态目标具有稳定 ID，位置与速度处于同一参考时刻",
                "允许在局部规划时域内采用常速度外推",
            ],
            [
                "A3 道路结构已知",
                "让行路段可由左右横截面与巡航索引离线描述",
                "适用于固定园区路线，不直接覆盖完全未知道路",
            ],
            [
                "A4 候选点有限",
                "每个让行路段预先给出有限个可停靠点",
                "可证明单次事件候选尝试有限终止",
            ],
            [
                "A5 局部规划器闭环稳定",
                "给定可达目标时，move_base 能持续输出控制或报告失败",
                "上层只管理目标与模式，不直接替代底层速度控制",
            ],
        ],
        widths=[18, 45, 37],
        font_size=8.8,
        first_col_bold=True,
        row_colors=[LIGHT_BLUE, LIGHT_GRAY, LIGHT_GOLD, LIGHT_GREEN, LIGHT_TEAL],
    )

    w.heading("1.4  总体思路", 2)
    w.flow_box(
        [
            ("感知与表征", "地图、TF、点云、激光和带语义/速度的动态目标"),
            ("连续轨迹层", "全局路径 + TEB/NeuPAN 局部规划，输出实时速度"),
            ("场景决策层", "道路语义判定 + 滑动时间窗 + 道路让行混合状态机"),
            ("模式与安全层", "台阶切换、候选换点、超时重试、SAFE_STOP"),
            ("反馈闭环", "机器人位姿、目标结果、感知新鲜度和距离进展"),
        ]
    )
    w.paragraph(
        "连续轨迹层回答“如何运动”，场景决策层回答“当前应执行何种行为”。"
        "两层通过导航目标、动态障碍、状态反馈和模式变量交互，"
        "既保留成熟局部规划器的实时性，又使交通行为具备显式逻辑。"
    )


def add_model_and_notation(w):
    w.heading("2  系统模型与符号定义", 1)
    w.heading("2.1  机器人运动模型", 2)
    w.paragraph(
        "以平面差速运动为控制抽象。机器人状态和控制分别定义为"
        " x=[x,y,θ]ᵀ 与 u=[v,ω]ᵀ。离散时间步长为 Δt，则："
    )
    w.formula(
        "xₖ₊₁ = xₖ + Δt·[vₖcosθₖ, vₖsinθₖ, ωₖ]ᵀ",
        "实际系统由 move_base 局部规划器生成 v 与 ω；模型用于统一描述轨迹和约束。",
    )
    w.paragraph(
        "控制满足速度与加速度界：|vₖ|≤v_max、|ωₖ|≤ω_max、"
        "|vₖ−vₖ₋₁|≤a_vΔt、|ωₖ−ωₖ₋₁|≤a_ωΔt。"
        "台阶模式下采用更小的速度上界与更强的前向运动约束。"
    )

    w.heading("2.2  动态目标模型", 2)
    w.paragraph(
        "第 i 个动态目标表示为 oᵢ=(idᵢ,cᵢ,pᵢ,vᵢ,Sᵢ,ξᵢ)，"
        "其中 idᵢ 为稳定跟踪标识，cᵢ 为语义类别，pᵢ 和 vᵢ 为位置与速度，"
        "Sᵢ 为几何形状，ξᵢ 为跟踪与运动状态。短时预测采用常速度模型："
    )
    w.formula("p̂ᵢ(t+τ) = pᵢ(t) + vᵢ(t)τ")
    w.paragraph(
        "若目标为局部多边形顶点 q̄ᵢⱼ，其世界坐标为"
        " qᵢⱼ=R(ψᵢ)q̄ᵢⱼ+pᵢ。包围盒、圆柱和一般多边形均由此统一映射到 TEB 障碍表示。"
    )

    w.heading("2.3  道路走廊模型", 2)
    w.paragraph(
        "道路由有序横截面序列 B={b₁,…,b_L} 表示，bⱼ=(lⱼ,rⱼ)，"
        "其中 lⱼ 与 rⱼ 分别为第 j 个截面的左右边界。"
        "相邻横截面构成四边形单元："
    )
    w.formula(
        "Qⱼ = conv{lⱼ, lⱼ₊₁, rⱼ₊₁, rⱼ}，     R = ⋃ⱼ Qⱼ",
        "点是否位于 Qⱼ 内部通过两个三角形的有向叉积符号判断。",
    )
    w.paragraph(
        "该模型能够近似弯曲道路。道路中心线只用于投影和里程估计；"
        "前进方向不由横截面顺序固定，而由当前导航目标动态确定。"
    )

    w.heading("2.4  主要符号", 2)
    w.table(
        ["符号", "定义", "符号", "定义"],
        [
            ["r(t)", "机器人二维位置", "g(t)", "当前巡航目标"],
            ["e(t)", "机器人指向目标的单位向量", "pᵢ,vᵢ", "车辆位置与速度"],
            ["sᵢ", "车辆相对纵向距离", "vᵢ∥", "车辆沿行驶方向速度"],
            ["W,K", "时间窗长度与确认阈值", "L", "恢复所需连续清空帧数"],
            ["Q", "候选避让点集合", "B", "当前事件黑名单"],
            ["σ", "普通/台阶模式变量", "S", "混合状态机离散状态集"],
        ],
        widths=[12, 38, 12, 38],
        font_size=9.2,
        first_col_bold=True,
    )

    w.heading("2.5  任务完成与安全判据", 2)
    w.paragraph(
        "本文将系统目标分为三类：运动可行性、行为一致性和失效安全性。"
        "运动可行性要求局部轨迹满足速度、加速度和障碍距离约束；"
        "行为一致性要求让行触发、等待和恢复遵守状态守卫；"
        "失效安全性要求在候选耗尽、目标超时或关键依赖异常时停止继续推进任务。"
    )
    w.formula(
        "Goal = FeasibleMotion ∧ ConsistentBehavior ∧ FailSafe"
    )
    w.paragraph(
        "这里的 FailSafe 表示发生已知异常时进入确定的停止模式，"
        "并不意味着对所有未知故障给出形式化安全证明。"
        "该区分避免把工程故障处理过度表述为严格的系统安全性定理。"
    )


def add_method(w):
    w.heading("3  分层自主导航方法", 1)
    w.heading("3.1  动态障碍适配与局部轨迹优化", 2)
    w.paragraph(
        "感知端输出的目标含语义、三维形状和不确定性，而 TEB 接收二维几何与速度。"
        "适配过程首先校验位置、姿态和速度，再依据形状类型构造二维圆或多边形，"
        "最后附加稳定 ID 与速度。几何信息保证空间占用正确，速度信息用于时间维预测。"
    )
    w.paragraph(
        "设局部轨迹为 Z={z₀,…,z_N,ΔT₀,…,ΔT_N₋₁}。"
        "TEB 的优化思想可写为："
    )
    w.formula(
        "Z* = arg min_Z  w_tJ_t + w_sJ_static + w_dJ_dynamic + w_kJ_kin + w_pJ_path",
        "各项依次表示时间、静态障碍、动态障碍、运动学可行性和参考路径偏差。",
    )
    w.formula(
        "J_dynamic = ΣₖΣᵢ φ(d(zₖ, Ŝᵢ(tₖ)) − d_safe)",
        "Ŝᵢ(tₖ) 为由 p̂ᵢ(tₖ) 平移后的预测形状，φ 为对安全距离违约的惩罚函数。",
    )
    w.paragraph(
        "局部轨迹优化适合连续、短时的避障，但无法独立表达“主动靠边并等待”这一长时行为。"
        "因此，动态目标同时被送入上层场景判定。"
    )

    w.heading("3.2  目标导向的道路语义坐标", 2)
    w.paragraph(
        "设机器人位置为 r，当前目标为 g。定义局部行驶方向："
    )
    w.formula("e = (g − r) / ‖g − r‖₂")
    w.paragraph(
        "对车辆 i，纵向距离和纵向速度分别为："
    )
    w.formula("sᵢ = (pᵢ − r)ᵀe，       vᵢ∥ = vᵢᵀe")
    w.paragraph(
        "sᵢ>m_a 表示车辆在前方，sᵢ<−m_b 表示车辆在后方。"
        "该定义随目标变化，因此道路反向使用时无需修改边界数据。"
        "与直接使用地图 x/y 轴相比，该坐标更符合当前任务语义。"
    )

    w.heading("3.3  空间—语义—运动联合来车判据", 2)
    w.paragraph(
        "车辆是否构成让行触发，不由单一距离决定。定义车辆类别集合 C_v。"
        "目标 i 的单帧有效来车指示量为："
    )
    w.formula(
        "aᵢ(t)=𝟙[cᵢ∈Cᵥ]·𝟙[pᵢ∈R]·𝟙[sᵢ∈(mₐ,dₐ)]·χᵢ(t)"
    )
    w.formula(
        "χᵢ=𝟙[stationary ∨ no_velocity ∨ ‖vᵢ‖≤ε_s ∨ vᵢ∥≤−ε_o]",
        "χᵢ 同时接受静止、速度未知和明确对向车辆，采用保守安全策略。",
    )
    w.paragraph(
        "道路外目标、后方目标、同向车辆和近似横向运动目标不会触发。"
        "单帧场景观测定义为 z_t=max_i aᵢ(t)。"
    )

    w.heading("3.4  K-of-W 时间一致性判定", 2)
    w.paragraph(
        "为抑制遮挡和误检，维护长度为 W 的布尔窗口。确认量为："
    )
    w.formula(
        "yₜ = 𝟙[Σⱼ₌₀ᵂ⁻¹ zₜ₋ⱼ ≥ K]",
        "当前配置 W=5、K=3，即最近 5 帧中至少 3 帧存在有效来车。",
    )
    w.paragraph(
        "若单帧误触发概率为 p，且各帧近似独立，则窗口误触发概率为："
    )
    w.formula(
        "P_FP = Σ(j=K,…,W) C(W,j)pʲ(1−p)ᵂ⁻ʲ"
    )
    w.paragraph(
        "例如 p=0.1、W=5、K=3 时，P_FP≈0.00856。"
        "该计算仅用于解释多数窗的降噪作用；实际帧间相关性会使概率偏离独立模型。"
    )

    w.heading("3.5  避让点选择与无进展检测", 2)
    w.paragraph(
        "每个避让点 q_j 具有适用巡航索引区间 [b_j,e_j]。"
        "在当前巡航索引 m 下，可行候选集为："
    )
    w.formula("A(m)={q_j | b_j≤m≤e_j}")
    w.paragraph(
        "对当前事件黑名单 B，选择最近未失败候选："
    )
    w.formula("j* = arg min_{j:q_j∈A(m)\\B} ‖r−q_j‖₂")
    w.paragraph(
        "令 d(t)=‖r(t)−q_{j*}‖₂。每当 d_ref−d(t)≥δ 时更新参考距离和进展时刻；"
        "若持续 T_p 时间未达到 δ，则认为目标无进展："
    )
    w.formula(
        "t−t_progress ≥ T_p  ∧  d_ref−d(t)<δ  ⇒  B←B∪{j*}",
        "当前配置 δ=0.25 m、T_p=8 s。之后立即选择下一候选。",
    )
    w.paragraph(
        "若机器人位姿暂时不可用，进展计时暂停；若所有保留候选均失败，系统进入安全停止。"
    )

    w.heading("3.6  让行恢复判据", 2)
    w.paragraph(
        "到达避让点时记录 t_arr。恢复不仅要求等待时间满足，还要求连续 L 条新鲜消息中，"
        "前方无来车且后方无配置车辆。设 f_t 表示消息新鲜，b_t 表示后方车辆，则："
    )
    w.formula(
        "cₜ = 𝟙[t−t_arr≥T_wait] · ∏ⱼ₌₀ᴸ⁻¹ 𝟙[fₜ₋ⱼ=1 ∧ zₜ₋ⱼ=0 ∧ bₜ₋ⱼ=0]"
    )
    w.paragraph(
        "感知过期会使连续清空计数归零。该非对称设计使触发较灵敏、恢复更保守，"
        "避免车辆尚未完全通过时提前回到道路中央。"
    )

    w.heading("3.7  混合状态机", 2)
    w.paragraph(
        "完整任务建模为混合系统 H=(S,X,U,G,R)。"
        "S 为离散模式集合，X 为机器人和感知连续状态，G 为状态转移守卫，R 为重置映射。"
    )
    w.formula(
        "S={INIT, ROUTE, PREP_ROUTE_STAIR, PREP_YIELD, GO_YIELD, WAIT_CLEAR, EXIT, RESUME, FINISHED, SAFE_STOP}"
    )
    w.table(
        ["转移", "守卫条件", "主要动作"],
        [
            ["INIT → ROUTE", "导航依赖就绪；按配置满足感知条件", "启动当前巡航航点"],
            ["ROUTE → PREP_YIELD", "y_t=1 且位于有效检测路线段", "取消巡航，生成候选集"],
            ["PREP_YIELD → GO_YIELD", "模式切换与预热完成", "发送最近候选避让点"],
            ["GO_YIELD → WAIT_CLEAR", "避让目标成功", "记录到达时刻，清零恢复计数"],
            ["GO_YIELD → GO_YIELD", "目标失败或无进展且仍有候选", "拉黑当前点并切换"],
            ["WAIT_CLEAR → EXIT/RESUME", "c_t=1", "驶向退出点或直接恢复"],
            ["RESUME → ROUTE", "事件变量完成重置", "重新发送被中断巡航点"],
            ["任意关键状态 → SAFE_STOP", "依赖失败或候选耗尽", "取消目标并停止任务"],
        ],
        widths=[26, 43, 31],
        font_size=8.9,
        first_col_bold=True,
        row_colors=[
            LIGHT_GRAY,
            LIGHT_GOLD,
            LIGHT_TEAL,
            LIGHT_GREEN,
            LIGHT_BLUE,
            LIGHT_BLUE,
            LIGHT_GREEN,
            LIGHT_RED,
        ],
    )

    w.heading("3.8  台阶模式的切换模型", 2)
    w.paragraph(
        "定义模式变量 σ∈{0,1}，σ=0 为普通模式，σ=1 为台阶/靠边模式。"
        "对点云点 p=(x,y,z)，前方区域 Ω_f 内的最小高度阈值为："
    )
    w.formula(
        "h_min(p,σ)= { max(h₀,h_s),  σ=1且p∈Ω_f；  h₀,  其他 }"
    )
    w.paragraph(
        "同时将 TEB 参数从 θ₀ 切换为 θ_s："
    )
    w.formula(
        "θ_TEB(σ)=(1−σ)θ₀+σθ_s",
        "θ 包含速度、加速度、前向运动权重、时间权重和路径点权重等受控参数。",
    )
    w.paragraph(
        "局部 ROI 只过滤正前方可能属于台阶结构的低矮点，侧向障碍仍被保留。"
        "正常退出后恢复 θ₀；模式消息带超时保护，降低异常退出造成的持续误配置风险。"
    )

    w.heading("3.9  局部规划器的统一抽象", 2)
    w.paragraph(
        "为避免上层方法依赖特定规划器，将局部规划统一表示为策略映射："
    )
    w.formula(
        "π_local : (x, G_local, O_local, Θ) ↦ (u, status)"
    )
    w.paragraph(
        "其中 G_local 为局部参考路径，O_local 为局部障碍，Θ 为规划参数，"
        "status 包含到达、停止或失败状态。TEB 直接实现 nav_core 接口；"
        "NeuPAN 通过 C++ 插件与 Python 服务端实现同一映射。"
        "因此，上层混合状态机只依赖目标结果与机器人反馈，可在不改变行为逻辑的情况下替换局部规划器。"
    )
    w.paragraph(
        "服务化 NeuPAN 时，setPlan() 负责路径更新，computeVelocityCommands() 负责单周期控制求解，"
        "速度只由 move_base 发布。该约束保证控制输出源唯一，避免服务端与导航主进程产生指令竞争。"
    )

    w.heading("3.10  完整算法流程", 2)
    w.table(
        ["阶段", "输入", "计算", "输出"],
        [
            [
                "感知更新",
                "O(t)、TF、当前目标 g",
                "坐标变换、道路包含、sᵢ、vᵢ∥、z_t",
                "感知快照与 y_t",
            ],
            [
                "巡航决策",
                "y_t、路线索引、导航状态",
                "判断继续巡航、提前通过航点或触发让行",
                "巡航目标或 PREP_YIELD",
            ],
            [
                "避让准备",
                "r、A(m)、B、σ",
                "候选排序、模式切换与预热",
                "避让目标 q*",
            ],
            [
                "避让执行",
                "d(t)、action 结果",
                "成功判断、超时和无进展检测",
                "WAIT_CLEAR、换点或 SAFE_STOP",
            ],
            [
                "安全恢复",
                "t_arr、f_t、z_t、b_t",
                "最小等待与 L 帧连续清空判断",
                "退出目标或 RESUME",
            ],
            [
                "事件重置",
                "完成的让行事件",
                "清空候选、黑名单、计时与失败计数",
                "恢复被中断巡航点",
            ],
        ],
        widths=[16, 24, 38, 22],
        font_size=8.6,
        first_col_bold=True,
        row_colors=[LIGHT_BLUE, LIGHT_GRAY, LIGHT_GOLD, LIGHT_TEAL, LIGHT_GREEN, PALE_BLUE],
    )


def add_properties(w):
    w.heading("4  方法性质与复杂度分析", 1)
    w.heading("4.1  方向自适应性", 2)
    w.quote_box(
        "性质 1：目标方向一致性",
        "只要当前目标 g 与机器人位置 r 不重合，e=(g−r)/‖g−r‖ 唯一确定。"
        "当路线反向时，e 的方向相应反转，sᵢ 与 vᵢ∥ 的符号同步变化，"
        "因此同一车辆会根据新的任务方向重新获得“前方/后方”和“同向/对向”语义，"
        "无需反转道路横截面数据。",
        BLUE,
        LIGHT_BLUE,
    )

    w.heading("4.2  时间窗鲁棒性", 2)
    w.paragraph(
        "K-of-W 判定可容忍 W−K 帧阴性观测而保持确认，也要求至少 K 帧证据才触发。"
        "当 W=5、K=3 时，窗口可容忍 2 帧漏检；"
        "相较单帧判定，误触发概率在独立近似下由 p 降至三阶量级 O(p³)。"
    )

    w.heading("4.3  候选切换的有限终止性", 2)
    w.quote_box(
        "性质 2：单次事件有限候选尝试",
        "候选点数量有限，且每次失败都会向黑名单加入一个此前未失败的候选。"
        "因此，在不重复尝试的条件下，单次让行事件最多执行 min(K_c,|A(m)|) 次候选导航；"
        "成功时进入等待状态，候选耗尽时进入 SAFE_STOP，不会在失败候选之间无限循环。",
        GREEN,
        LIGHT_GREEN,
    )

    w.heading("4.4  恢复安全性", 2)
    w.paragraph(
        "恢复守卫 c_t 同时包含最小等待时间、新鲜感知和连续清空条件。"
        "因此，旧消息、单帧清空或等待时间不足均不能触发恢复。"
        "该性质不等价于形式化碰撞安全证明，但排除了三类常见的不安全恢复原因。"
    )

    w.heading("4.5  计算复杂度", 2)
    w.paragraph(
        "设动态目标数为 N_o，道路四边形单元数为 N_r，动态目标总顶点数为 V，"
        "候选避让点数为 N_q。各核心步骤复杂度如下："
    )
    w.table(
        ["步骤", "时间复杂度", "说明"],
        [
            ["目标几何适配", "O(V)", "每个形状顶点仅执行一次旋转和平移"],
            ["道路内目标筛选", "O(N_oN_r)", "逐目标遍历道路单元；可用空间索引进一步优化"],
            ["纵向位置与速度判定", "O(N_o)", "每个目标执行常数次点积与阈值比较"],
            ["滑动时间窗", "O(1)", "使用双端队列和正样本计数增量更新"],
            ["候选避让点排序", "O(N_q log N_q)", "每次让行事件开始时执行一次"],
            ["状态机更新", "O(1)", "固定数量状态和守卫条件"],
        ],
        widths=[30, 22, 48],
        font_size=9.0,
        first_col_bold=True,
    )
    w.paragraph(
        "在当前固定路线中 N_r 与 N_q 均较小，主要计算负担仍位于局部轨迹优化和感知系统。"
    )

    w.heading("4.6  参数敏感性与设计权衡", 2)
    w.paragraph(
        "关键参数并非彼此独立。时间窗决定触发延迟与抗噪性，"
        "空间阈值决定关注范围，进展参数决定换点速度，"
        "台阶参数则影响通过能力与控制平滑性。"
    )
    w.table(
        ["参数", "增大后的主要影响", "减小后的主要影响", "设计原则"],
        [
            ["W", "时间平滑增强，但确认延迟增大", "响应更快，但易受单帧波动影响", "与感知频率共同确定实际时间尺度"],
            ["K/W", "触发更保守，漏检风险上升", "触发更灵敏，误触发风险上升", "按安全优先级选择多数阈值"],
            ["m_a", "忽略更近车辆，减少机器人自身附近噪声", "更早关注近前方目标", "应大于定位与检测抖动尺度"],
            ["d_a", "关注更远车辆，计算与误触发范围增大", "只响应近距离来车", "与制动距离和道路可视范围匹配"],
            ["δ", "要求更明显进展，换点更积极", "微小移动即可重置计时，可能长时间等待", "应高于定位噪声且低于正常 8 s 位移"],
            ["T_p", "更容忍暂时阻塞，但故障切换较慢", "快速换点，但可能误判短时停顿", "与局部规划恢复周期协调"],
            ["h_s", "更易忽略台阶低矮点，也可能遗漏真实低障碍", "保留更多障碍，但台阶可能阻塞规划", "仅在前方 ROI 内使用并设置模式超时"],
        ],
        widths=[12, 30, 30, 28],
        font_size=8.2,
        first_col_bold=True,
        row_colors=[LIGHT_BLUE, LIGHT_GRAY, LIGHT_GOLD, LIGHT_TEAL, LIGHT_GREEN, PALE_BLUE, LIGHT_RED],
    )
    w.paragraph(
        "因此，参数整定应采用场景级指标，而不能仅依据单次轨迹是否通过。"
        "较合理的流程是先固定感知频率与安全距离，再整定 W、K 和空间范围，"
        "最后根据失败切换时间调整 δ、T_p 与台阶模式参数。"
    )


def add_architecture_mapping(w):
    w.heading("5  算法模块与实现映射", 1)
    w.paragraph(
        "算法按职责划分为环境表征、局部规划、场景决策、模式控制和执行反馈五层。"
        "模块边界与数学对象保持对应，便于验证与替换。"
    )
    w.table(
        ["算法层", "数学对象/职责", "实现模块", "输入—输出"],
        [
            [
                "动态目标表征",
                "oᵢ、Sᵢ、p̂ᵢ(t+τ)",
                "dynamic_obstacles",
                "TrackedObjectArray → TEB ObstacleArrayMsg",
            ],
            [
                "道路语义",
                "R、e、sᵢ、vᵢ∥",
                "road_geometry / vehicle_monitor",
                "目标与路线 → 感知快照",
            ],
            [
                "时间一致性",
                "z_t、y_t、c_t",
                "detection_window / vehicle_monitor",
                "帧级观测 → 来车确认/清空条件",
            ],
            [
                "行为决策",
                "H=(S,X,U,G,R)",
                "road_yield_manager",
                "感知与导航反馈 → 状态转移/目标",
            ],
            [
                "执行与反馈",
                "目标结果、超时、d(t)",
                "navigation_client / pose_provider",
                "NavigationPose ↔ move_base action",
            ],
            [
                "模式切换",
                "σ、h_min、θ_TEB",
                "stair_controller / pointcloud_to_laserscan",
                "任务模式 → 点云与规划参数",
            ],
            [
                "规划扩展",
                "局部轨迹与控制 u",
                "neupan_service",
                "move_base 插件 ↔ Python 推理服务",
            ],
        ],
        widths=[18, 24, 28, 30],
        font_size=8.6,
        first_col_bold=True,
    )
    w.heading("5.1  模块耦合关系", 2)
    w.paragraph(
        "dynamic_obstacles 与 vehicle_monitor 共享同一感知源，但输出目的不同："
        "前者提供连续轨迹优化所需的几何与速度，后者生成离散行为触发量。"
        "road_yield_manager 不直接计算速度，只通过 move_base 发送目标；"
        "局部规划器因此保持唯一的速度控制权。"
    )
    w.paragraph(
        "stair_controller 是行为层到感知层和规划层的反向通道。"
        "这种有限的跨层反馈比在各模块内复制场景逻辑更清晰，"
        "也使普通巡航、道路让行和台阶巡航共享同一导航执行框架。"
    )


def add_experiments(w):
    w.heading("6  实验设计与结果分析", 1)
    w.heading("6.1  验证层次", 2)
    w.table(
        ["层次", "目的", "主要内容"],
        [
            ["单元测试", "验证纯算法边界条件", "道路几何、方向反转、时间窗、运动判据、候选排序和进展看门狗"],
            ["仿真测试", "验证动态目标接口与轨迹响应", "多形状障碍、多路线运动、速度箭头、短期预测和 TEB 输入"],
            ["路线测试", "验证完整任务闭环", "A1 台阶巡航；A2 来车检测、靠边让行和原路线恢复"],
        ],
        widths=[18, 30, 52],
        font_size=9.1,
        first_col_bold=True,
        row_colors=[LIGHT_BLUE, LIGHT_GOLD, LIGHT_GREEN],
    )

    w.heading("6.2  单元测试", 2)
    w.paragraph(
        "当前工作区共执行 20 项 road_yield 测试。"
        "道路几何、来车判定、滑动窗口和避让策略等 14 项核心算法测试全部通过；"
        "配置加载组 6 项中通过 5 项。唯一失败项源于仿真避让点已由 7 个增加为 9 个，"
        "而测试期望未同步，属于测试基线维护问题。"
    )
    w.table(
        ["测试组", "测试数", "通过数", "关键覆盖"],
        [
            ["RoadGeometry", "4", "4", "空道路、区域内外、目标方向反转、弯曲走廊"],
            ["DetectionWindow / Oncoming", "6", "6", "3/5 窗、滑动更新、静止/对向/同向/横向/无速度"],
            ["AvoidanceStrategy", "4", "4", "无进展、进展重置、位姿缺失、候选排序"],
            ["ConfigLoader", "6", "5", "路线、道路、避让、台阶、纯巡航与仿真配置"],
        ],
        widths=[30, 12, 14, 44],
        font_size=9.0,
        first_col_bold=True,
        row_colors=[LIGHT_GREEN, LIGHT_GREEN, LIGHT_GREEN, LIGHT_GOLD],
    )

    w.heading("6.3  动态障碍仿真", 2)
    w.paragraph(
        "仿真器可生成圆、圆柱、包围盒、多边形和线段障碍，"
        "并支持方形、圆形、航点往返和静止路线。"
        "每个障碍具有独立 ID、速度、相位和朝向模式。"
        "系统同时显示实体、速度向量、完整路线和短期预测，用于检查几何转换和 TEB 动态障碍响应。"
    )
    w.paragraph(
        "该实验主要验证接口正确性与定性行为。"
        "由于仓库未保存统一的轨迹误差和最小距离日志，本文不构造缺乏证据的定量结论。"
    )

    w.heading("6.4  A1/A2 路线验证", 2)
    w.table(
        ["路线", "任务特征", "验证结果"],
        [
            [
                "A1",
                "预设航点巡航；指定航点切换台阶模式；完成后恢复普通参数",
                "部署记录表明路线与台阶模式测试成功",
            ],
            [
                "A2",
                "道路来车检测；中断巡航；驶向靠边点；满足恢复条件后继续原航点",
                "部署记录与提交记录表明道路让行测试成功",
            ],
        ],
        widths=[12, 56, 32],
        font_size=9.2,
        first_col_bold=True,
        row_colors=[LIGHT_BLUE, LIGHT_GREEN],
    )

    w.heading("6.5  建议的定量评价指标", 2)
    w.paragraph(
        "现有证据能够说明功能闭环成立，但不足以完成严格性能比较。"
        "后续实验应统一记录以下指标："
    )
    w.table(
        ["指标", "定义", "研究意义"],
        [
            ["任务成功率 SR", "成功完成巡航与让行的试验数 / 总试验数", "评价整体可靠性"],
            ["最小安全距离 d_min", "机器人与车辆在一次会车中的最小距离", "评价安全裕度"],
            ["误触发率 FTR", "无有效来车却进入让行的比例", "评价联合判据精度"],
            ["漏触发率 FNR", "有有效来车但未确认的比例", "评价感知与时间窗召回"],
            ["让行耗时 T_y", "触发到进入 WAIT_CLEAR 的时间", "评价避让效率"],
            ["恢复延迟 T_r", "道路实际清空到恢复巡航的时间", "评价保守性"],
            ["规划时延 T_p", "单周期局部规划计算时间", "评价实时性"],
        ],
        widths=[22, 46, 32],
        font_size=8.8,
        first_col_bold=True,
    )
    w.paragraph(
        "建议设置 TEB-only、TEB+单帧触发和本文完整方法三组对照，"
        "分别比较任务成功率、误触发率、最小安全距离和恢复时间。"
    )

    w.heading("6.6  对照与消融实验设计", 2)
    w.paragraph(
        "为区分各模块的独立贡献，可设置以下实验组。"
        "所有组使用相同地图、路线、车辆轨迹、初始位姿和 TEB 基础参数，"
        "每种来车速度与遮挡条件重复运行若干次。"
    )
    w.table(
        ["实验组", "保留模块", "研究问题", "预期观察"],
        [
            [
                "B0：TEB-only",
                "仅动态障碍局部轨迹优化",
                "局部规划是否能独立完成窄路会车",
                "可能在道路中央等待或反复局部绕行",
            ],
            [
                "B1：单帧行为触发",
                "TEB + 道路区域 + 单帧来车判定",
                "时间窗是否有效降低误触发",
                "触发更快，但对遮挡与偶发误检敏感",
            ],
            [
                "B2：无候选换点",
                "完整来车判定 + 固定避让点",
                "进展看门狗与多候选是否改善可恢复性",
                "首选点被阻塞时成功率下降",
            ],
            [
                "B3：无跨层台阶模式",
                "完整让行，但点云和 TEB 参数固定",
                "感知—规划联动对特殊地形的作用",
                "台阶误障碍或运动不稳定概率上升",
            ],
            [
                "Proposed",
                "本文全部模块",
                "完整方案的综合收益",
                "在成功率、安全距离和恢复能力之间取得平衡",
            ],
        ],
        widths=[20, 30, 28, 22],
        font_size=8.3,
        first_col_bold=True,
        row_colors=[LIGHT_GRAY, LIGHT_GOLD, LIGHT_GREEN, LIGHT_RED, LIGHT_BLUE],
    )
    w.paragraph(
        "统计分析可报告均值、标准差和 95% 置信区间。"
        "对成功/失败等二值指标可采用 Fisher 精确检验，"
        "对让行时间和最小距离可根据分布选择 t 检验或 Mann–Whitney U 检验。"
        "当前仓库尚无足够重复试验数据，因此该部分作为后续实验方案，而非既有结论。"
    )


def add_innovation_and_discussion(w):
    w.heading("7  创新性与学术讨论", 1)
    w.heading("7.1  创新点总结", 2)
    w.table(
        ["创新点", "核心思想", "区别于常规方法"],
        [
            [
                "双时间尺度决策",
                "短时连续轨迹优化与长时离散行为规划并行",
                "避免将完整会车行为压缩为局部障碍代价",
            ],
            [
                "目标导向道路坐标",
                "前后关系和对向速度随当前目标更新",
                "不依赖固定道路正方向，适应反向和折返路线",
            ],
            [
                "多域联合触发",
                "空间、语义、运动和时间证据共同确认",
                "比仅按距离或单帧检测更符合交通行为语义",
            ],
            [
                "故障感知型候选决策",
                "进展看门狗、事件黑名单和有限终止",
                "显式处理 action 活跃但机器人无实际进展的情况",
            ],
            [
                "感知—规划联合切换",
                "台阶模式同时修改点云观测模型与局部规划约束",
                "将特殊地形处理建模为跨层切换，而非孤立参数调节",
            ],
        ],
        widths=[24, 40, 36],
        font_size=8.9,
        first_col_bold=True,
        row_colors=[LIGHT_BLUE, LIGHT_TEAL, LIGHT_GOLD, LIGHT_GREEN, LIGHT_RED],
    )
    w.quote_box(
        "核心学术表述",
        "本文的主要贡献不是重新设计一个局部规划器，"
        "而是提出一套以目标导向道路语义为基础、"
        "将连续时空轨迹优化与离散交通行为决策耦合的分层方法。"
        "该方法使动态目标从“几何障碍”进一步成为“行为触发对象”，"
        "并通过有限状态、时间一致性与失效恢复机制形成完整决策闭环。",
        NAVY,
        LIGHT_BLUE,
    )

    w.heading("7.2  方法优势", 2)
    w.bullet("可解释：每次触发均可追溯到道路、类别、位置、速度和时间窗条件。")
    w.bullet("可恢复：候选失败、无进展、感知过期和导航超时均有确定处理。")
    w.bullet("可迁移：路线、道路和避让点由配置描述，算法不依赖单一地图坐标方向。")
    w.bullet("可扩展：TEB 与 NeuPAN 共享 move_base 接口，上层行为决策不依赖具体局部规划器。")

    w.heading("7.3  局限性", 2)
    w.numbered(
        "（1）",
        "动态目标采用常速度短时预测，未显式利用加速度、协方差和多模态意图。",
    )
    w.numbered(
        "（2）",
        "道路走廊和避让点依赖离线标注，尚不具备从在线地图自动生成语义道路的能力。",
    )
    w.numbered(
        "（3）",
        "当前感知发布端未稳定填充分类概率，判据尚未融合语义置信度。",
    )
    w.numbered(
        "（4）",
        "现有实车结果以功能验证为主，缺少多次重复试验和对照组的统计显著性分析。",
    )
    w.numbered(
        "（5）",
        "混合状态机的安全性主要由守卫条件和故障策略保证，尚未进行可达集或形式化验证。",
    )

    w.heading("7.4  后续研究方向", 2)
    w.table(
        ["方向", "方法设想", "预期收益"],
        [
            ["概率化来车风险", "融合类别概率、协方差和预测占用，构造 P(collision)>α 的风险触发", "提高不确定感知下的决策一致性"],
            ["多模态轨迹预测", "对车辆直行、转弯、停止等意图输出概率轨迹", "降低常速度模型在转弯场景的偏差"],
            ["在线道路语义", "由点云/地图自动提取道路走廊和临时避让区域", "减少离线 CSV 标注"],
            ["形式化安全验证", "对混合状态机建立时序逻辑性质与可达性分析", "给出恢复与停止策略的严格保证"],
            ["对比与消融实验", "移除时间窗、候选换点或台阶联动进行对照", "量化各创新模块的独立贡献"],
        ],
        widths=[24, 46, 30],
        font_size=8.8,
        first_col_bold=True,
    )

    w.heading("7.5  有效性威胁", 2)
    w.paragraph(
        "内部有效性方面，A1/A2 路线成功可能同时受到地图质量、定位稳定性和参数整定影响，"
        "不能将全部收益直接归因于单一算法模块；需要通过消融实验隔离变量。"
        "外部有效性方面，当前道路、候选点和机器人平台数量有限，"
        "对更宽道路、高速车辆和密集交通的推广仍需验证。"
    )
    w.paragraph(
        "测量有效性方面，现有部署记录以功能结果为主，缺少统一的最小距离、触发时间和计算时延日志。"
        "结论有效性方面，测试次数不足以支持统计显著性推断。"
        "因此，本文将当前结果定位为“方法可行性与系统闭环验证”，"
        "而不是对性能上界或普适优越性的最终证明。"
    )


def add_conclusion_and_appendix(w):
    w.heading("8  结论", 1)
    w.paragraph(
        "本文针对动态会车与特殊地形条件下的长距离巡航，"
        "提出一种连续轨迹优化与离散行为决策相结合的分层导航方法。"
        "方法以目标导向的道路语义坐标统一描述车辆前后关系与对向运动，"
        "通过空间—语义—运动—时间联合判据确认来车，"
        "并利用混合状态机完成靠边、等待、退出和恢复。"
    )
    w.paragraph(
        "为提高失效条件下的可靠性，方法引入候选避让点排序、距离进展看门狗、事件黑名单和安全停止；"
        "为处理台阶，采用感知阈值与规划参数同步切换。"
        "单元测试、仿真和 A1/A2 路线记录验证了核心逻辑与任务闭环。"
    )
    w.paragraph(
        "总体而言，本研究将动态目标从局部规划中的几何对象提升为场景决策中的语义对象，"
        "在不改变 ROS Navigation 基础架构的前提下增强了复杂场景行为能力。"
        "后续应通过概率预测、形式化验证和多次对照实验，进一步建立可量化的安全性与性能结论。"
    )

    w.page_break()
    w.heading("附录 A  核心算法伪代码", 1)
    w.paragraph(
        "算法 1：道路来车让行主循环",
        style="Text Body",
        align=LEFT,
        indent=False,
        size=11.0,
        color=NAVY,
        bold=True,
        bottom=100,
    )
    w.table(
        ["步骤", "操作"],
        [
            ["1", "读取机器人位置 r、当前目标 g 和最新感知 O(t)"],
            ["2", "计算 e=(g−r)/‖g−r‖；对每个目标计算道路包含、sᵢ 与 vᵢ∥"],
            ["3", "形成单帧观测 z_t，并更新 K-of-W 时间窗得到 y_t"],
            ["4", "若状态为 ROUTE 且 y_t=1：取消巡航，构造 A(m)，进入 PREP_YIELD"],
            ["5", "选择最近未拉黑候选 q*，完成模式预热后发送目标"],
            ["6", "若目标失败或 T_p 内进展不足 δ：B←B∪{q*}；有候选则换点，否则 SAFE_STOP"],
            ["7", "到达后记录 t_arr；满足等待时间和 L 帧清空条件时退出避让"],
            ["8", "清理事件变量，恢复被中断巡航点"],
        ],
        widths=[10, 90],
        font_size=9.2,
        first_col_bold=True,
        row_colors=[LIGHT_GRAY, WHITE, LIGHT_GRAY, LIGHT_GOLD, LIGHT_TEAL, LIGHT_RED, LIGHT_GREEN, LIGHT_BLUE],
    )

    w.heading("附录 B  关键参数", 1)
    w.table(
        ["参数", "当前值", "含义"],
        [
            ["W, K", "5, 3", "来车确认窗口与最小正帧数"],
            ["L", "5", "恢复所需连续清空帧数"],
            ["ε_s, ε_o", "0.1 m/s, 0.1 m/s", "静止与对向速度阈值"],
            ["m_a, m_b", "0.3 m, 0.3 m", "前后方纵向边界"],
            ["d_a, d_b", "30 m, 30 m", "最大前后检测距离"],
            ["K_c", "5", "单次事件保留的候选避让点数"],
            ["δ, T_p", "0.25 m, 8 s", "进展阈值与无进展超时"],
            ["T_wait", "3 s", "到达避让点后的最小等待时间"],
        ],
        widths=[22, 28, 50],
        font_size=9.1,
        first_col_bold=True,
    )

    w.heading("附录 C  代码索引", 1)
    w.table(
        ["主题", "主要文件"],
        [
            ["道路几何", "src/longdist_nav/road_yield/src/road_geometry.cpp"],
            ["来车判定", "src/longdist_nav/road_yield/src/vehicle_monitor.cpp"],
            ["时间窗", "src/longdist_nav/road_yield/include/road_yield/detection_window.h"],
            ["候选与看门狗", "src/longdist_nav/road_yield/include/road_yield/avoidance_strategy.h"],
            ["混合状态机", "src/longdist_nav/road_yield/src/road_yield_manager.cpp"],
            ["台阶切换", "src/longdist_nav/road_yield/src/stair_controller.cpp"],
            ["动态障碍适配", "src/longdist_nav/dynamic_obstacles/src/teb_obstacle_adapter.cpp"],
            ["NeuPAN 桥接", "src/longdist_nav/neupan_service"],
        ],
        widths=[28, 72],
        font_size=8.9,
        first_col_bold=True,
    )

    w.heading("参考文献", 1)
    refs = [
        "[1] Rösmann C., Hoffmann F., Bertram T. Integrated online trajectory planning and optimization in distinctive topologies. Robotics and Autonomous Systems, 2017, 88: 142–153.",
        "[2] Rösmann C., Feiten W., Wösch T., Hoffmann F., Bertram T. Trajectory modification considering dynamic constraints of autonomous robots. German Conference on Robotics, 2012.",
        "[3] Rösmann C., Feiten W., Wösch T., Hoffmann F., Bertram T. Efficient trajectory optimization using a sparse model. European Conference on Mobile Robots, 2013.",
        "[4] ROS Navigation Stack: move_base, costmap_2d, AMCL and nav_core software interfaces.",
        "[5] NeuPAN software package and runtime library; this project implements its ROS1 service and nav_core adapter.",
    ]
    for ref in refs:
        w.paragraph(
            ref,
            style="Text Body",
            align=LEFT,
            indent=False,
            size=9.2,
            bottom=85,
        )


def build_report(doc):
    default_page_style = configure_document_styles(doc)
    update_academic_header(doc)
    w = Writer(doc, default_page_style)

    props = doc.DocumentProperties
    props.Title = "面向动态会车与特殊地形的移动机器人分层自主导航方法研究"
    props.Subject = "问题建模、算法设计、性质分析、实验验证与创新贡献"
    props.Author = "navigation_ws 项目组"
    props.Description = "基于 navigation_ws/src 的学术型工作汇报。"

    add_cover(w)
    w.page_break()
    add_abstract(w)
    toc = add_toc(doc, w)
    add_problem_formulation(w)
    add_model_and_notation(w)
    add_method(w)
    add_properties(w)
    add_architecture_mapping(w)
    add_experiments(w)
    add_innovation_and_discussion(w)
    add_conclusion_and_appendix(w)

    try:
        toc.update()
    except Exception:
        pass
    if hasattr(doc, "calculateAll"):
        doc.calculateAll()


def save_report(doc):
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    doc.storeAsURL(
        uno.systemPathToFileUrl(DOCX_PATH),
        (prop("FilterName", "Office Open XML Text"), prop("Overwrite", True)),
    )
    doc.storeToURL(
        uno.systemPathToFileUrl(PDF_PATH),
        (prop("FilterName", "writer_pdf_Export"), prop("Overwrite", True)),
    )


def main():
    _, desktop = connect_office()
    doc = desktop.loadComponentFromURL(
        "private:factory/swriter",
        "_blank",
        0,
        (prop("Hidden", True),),
    )
    try:
        build_report(doc)
        save_report(doc)
    finally:
        doc.close(True)
    print(DOCX_PATH)
    print(PDF_PATH)


if __name__ == "__main__":
    main()
