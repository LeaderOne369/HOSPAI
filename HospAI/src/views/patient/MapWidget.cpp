#include "MapWidget.h"
#include <QListWidgetItem>
#include <QApplication>
#include <QMessageBox>
#include <QPainterPath>
#include <QLinearGradient>
#include <QEasingCurve>
#include <QDebug>
#include <QWheelEvent>
#include <QScrollBar>
#include <QVector2D>
#include <QPolygon>
#include <cmath>

// HospitalMapWidget 实现
HospitalMapWidget::HospitalMapWidget(QWidget* parent)
    : QWidget(parent)
    , m_zoomLevel(1.0)
    , m_pulseOpacity(1.0)
    , m_routeAnimationStep(0)
{
    setMinimumSize(800, 600);
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    
    // 初始化动画
    m_pulseAnimation = new QPropertyAnimation(this, "pulseOpacity");
    m_pulseAnimation->setDuration(1500);
    m_pulseAnimation->setEasingCurve(QEasingCurve::InOutSine);
    m_pulseAnimation->setLoopCount(-1);
    
    m_routeTimer = new QTimer(this);
    m_routeTimer->setInterval(100);
    connect(m_routeTimer, &QTimer::timeout, this, [this]() {
        m_routeAnimationStep = (m_routeAnimationStep + 1) % 20;
        update();
    });
}

void HospitalMapWidget::setDepartments(const QMap<QString, DepartmentInfo>& departments)
{
    m_departments = departments;
    update();
}

void HospitalMapWidget::highlightDepartment(const QString& departmentName)
{
    m_highlightedDepartment = departmentName;
    
    if (!departmentName.isEmpty()) {
        m_pulseAnimation->setStartValue(0.3);
        m_pulseAnimation->setEndValue(1.0);
        m_pulseAnimation->start();
    } else {
        m_pulseAnimation->stop();
        m_pulseOpacity = 1.0;
    }
    
    update();
}

void HospitalMapWidget::clearHighlight()
{
    m_highlightedDepartment.clear();
    m_hoveredDepartment.clear();
    m_pulseAnimation->stop();
    update();
}

void HospitalMapWidget::showRoute(const QString& from, const QString& to)
{
    m_routeFrom = from;
    m_routeTo = to;
    
    // 生成路径点（智能路径规划）
    if (m_departments.contains(from) && m_departments.contains(to)) {
        m_routePoints.clear();
        QRect fromRect = m_departments[from].mapRect;
        QRect toRect = m_departments[to].mapRect;
        
        QPoint start = fromRect.center();
        QPoint end = toRect.center();
        
        // 根据楼层差异规划路径
        QString fromFloor = m_departments[from].floor;
        QString toFloor = m_departments[to].floor;
        
        if (fromFloor == toFloor) {
            // 同楼层，直接连接或L型路径
            if (abs(start.x() - end.x()) > abs(start.y() - end.y())) {
                // 横向优先
                QPoint middle(end.x(), start.y());
                m_routePoints << start << middle << end;
            } else {
                // 纵向优先
                QPoint middle(start.x(), end.y());
                m_routePoints << start << middle << end;
            }
        } else {
            // 跨楼层，经过电梯或楼梯
            QPoint elevatorPoint(335, start.y()); // 电梯位置
            QPoint elevatorEnd(335, end.y());
            
            // 路径：起点 -> 电梯入口 -> 电梯出口 -> 终点
            m_routePoints << start;
            m_routePoints << QPoint(start.x(), elevatorPoint.y());
            m_routePoints << elevatorPoint;
            m_routePoints << elevatorEnd;
            m_routePoints << QPoint(end.x(), elevatorEnd.y());
            m_routePoints << end;
        }
        
        m_routeTimer->start();
    }
    
    update();
}

void HospitalMapWidget::setZoomLevel(double zoom)
{
    m_zoomLevel = qBound(0.5, zoom, 3.0);
    setMinimumSize(800 * m_zoomLevel, 600 * m_zoomLevel);
    update();
}

QSize HospitalMapWidget::sizeHint() const
{
    return QSize(800 * m_zoomLevel, 600 * m_zoomLevel);
}

void HospitalMapWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.scale(m_zoomLevel, m_zoomLevel);
    
    drawBackground(painter);
    drawBuildings(painter);
    drawDepartments(painter);
    drawRoute(painter);
    drawLabels(painter);
}

void HospitalMapWidget::mousePressEvent(QMouseEvent* event)
{
    QPoint scaledPos(event->pos().x() / m_zoomLevel, event->pos().y() / m_zoomLevel);
    QString department = getDepartmentAt(scaledPos);
    
    if (!department.isEmpty()) {
        emit departmentClicked(department);
    }
    
    QWidget::mousePressEvent(event);
}

void HospitalMapWidget::mouseMoveEvent(QMouseEvent* event)
{
    QPoint scaledPos(event->pos().x() / m_zoomLevel, event->pos().y() / m_zoomLevel);
    QString department = getDepartmentAt(scaledPos);
    
    if (department != m_hoveredDepartment) {
        m_hoveredDepartment = department;
        
        if (!department.isEmpty()) {
            DepartmentInfo info = m_departments[department];
            QString tooltip = QString("%1\n%2\n开放时间: %3")
                             .arg(info.name)
                             .arg(info.location)
                             .arg(info.hours);
            QToolTip::showText(event->globalPosition().toPoint(), tooltip);
        } else {
            QToolTip::hideText();
        }
        
        update();
    }
    
    QWidget::mouseMoveEvent(event);
}

void HospitalMapWidget::leaveEvent(QEvent* event)
{
    m_hoveredDepartment.clear();
    QToolTip::hideText();
    update();
    QWidget::leaveEvent(event);
}

void HospitalMapWidget::wheelEvent(QWheelEvent* event)
{
    const double scaleFactor = 1.15;
    double newZoom = m_zoomLevel;
    
    if (event->angleDelta().y() > 0) {
        newZoom *= scaleFactor;
    } else {
        newZoom /= scaleFactor;
    }
    
    setZoomLevel(newZoom);
    QWidget::wheelEvent(event);
}

void HospitalMapWidget::drawBackground(QPainter& painter)
{
    // 绘制背景
    QLinearGradient gradient(0, 0, 0, height() / m_zoomLevel);
    gradient.setColorAt(0, QColor("#F8F9FA"));
    gradient.setColorAt(1, QColor("#E9ECEF"));
    painter.fillRect(0, 0, 800, 600, gradient);
    
    // 绘制网格
    painter.setPen(QPen(QColor("#DEE2E6"), 1, Qt::DotLine));
    for (int x = 0; x < 800; x += 50) {
        painter.drawLine(x, 0, x, 600);
    }
    for (int y = 0; y < 600; y += 50) {
        painter.drawLine(0, y, 800, y);
    }
}

void HospitalMapWidget::drawBuildings(QPainter& painter)
{
    // 绘制建筑轮廓
    painter.setPen(QPen(QColor("#6C757D"), 2));
    painter.setBrush(QColor("#FFFFFF"));
    
    // 主楼
    QRect mainBuilding(50, 50, 700, 500);
    painter.drawRoundedRect(mainBuilding, 10, 10);
    
    // 门诊楼标识
    painter.setPen(QPen(QColor("#007AFF"), 2));
    painter.setFont(QFont("Microsoft YaHei", 16, QFont::Bold));
    painter.drawText(60, 80, "门诊医技楼");
    
    // 楼层分割线和标识
    painter.setPen(QPen(QColor("#DEE2E6"), 1));
    QStringList floors = {"5F", "4F", "3F", "2F", "1F"};
    QStringList floorDesc = {"住院部", "专家门诊", "医技科室", "普通门诊", "急诊大厅"};
    
    for (int floor = 1; floor <= 5; ++floor) {
        int y = 50 + floor * 100;
        painter.drawLine(50, y, 750, y);
        
        // 楼层标识
        painter.setPen(QColor("#6C757D"));
        painter.setFont(QFont("Microsoft YaHei", 12, QFont::Bold));
        painter.drawText(15, y - 50, floors[floor-1]);
        
        // 楼层描述
        painter.setFont(QFont("Microsoft YaHei", 10));
        painter.setPen(QColor("#8E8E93"));
        painter.drawText(15, y - 30, floorDesc[floor-1]);
    }
    
    // 绘制电梯和楼梯
    painter.setPen(QPen(QColor("#007AFF"), 2));
    painter.setBrush(QColor("#E3F2FD"));
    
    // 电梯
    QRect elevator(320, 80, 30, 420);
    painter.drawRect(elevator);
    painter.setPen(QColor("#007AFF"));
    painter.setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
    painter.drawText(322, 295, "电梯");
    
    // 楼梯
    QRect stairs(280, 80, 30, 420);
    painter.setPen(QPen(QColor("#FF9800"), 2));
    painter.setBrush(QColor("#FFF3E0"));
    painter.drawRect(stairs);
    painter.setPen(QColor("#FF9800"));
    painter.drawText(282, 295, "楼梯");
    
    // 主入口标识
    painter.setPen(QPen(QColor("#28A745"), 3));
    painter.setBrush(QColor("#D4EDDA"));
    QRect entrance(350, 530, 150, 20);
    painter.drawRect(entrance);
    painter.setPen(QColor("#28A745"));
    painter.setFont(QFont("Microsoft YaHei", 12, QFont::Bold));
    painter.drawText(380, 545, "🚪 主入口");
}

void HospitalMapWidget::drawDepartments(QPainter& painter)
{
    for (auto it = m_departments.begin(); it != m_departments.end(); ++it) {
        const QString& name = it.key();
        const DepartmentInfo& info = it.value();
        
        QRect rect = getScaledRect(info.mapRect);
        
        // 确定颜色
        QColor baseColor = info.highlightColor;
        if (name == m_highlightedDepartment) {
            baseColor.setAlphaF(m_pulseOpacity);
        } else if (name == m_hoveredDepartment) {
            baseColor = baseColor.lighter(130);
        } else {
            baseColor.setAlphaF(0.7);
        }
        
        // 绘制科室区域
        painter.setPen(QPen(baseColor.darker(120), 2));
        painter.setBrush(baseColor);
        painter.drawRoundedRect(rect, 8, 8);
        
        // 绘制科室名称
        painter.setPen(QColor("#212529"));
        painter.setFont(QFont("Arial", 10, QFont::Bold));
        
        QFontMetrics fm(painter.font());
        QString displayName = info.name;
        if (fm.horizontalAdvance(displayName) > rect.width() - 10) {
            displayName = fm.elidedText(displayName, Qt::ElideRight, rect.width() - 10);
        }
        
        painter.drawText(rect, Qt::AlignCenter, displayName);
    }
}

void HospitalMapWidget::drawRoute(QPainter& painter)
{
    if (m_routePoints.size() < 2) return;
    
    // 计算动画偏移
    double animationOffset = m_routeAnimationStep * 10.0;
    
    // 绘制路径背景（阴影效果）
    painter.setPen(QPen(QColor("#000000"), 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setOpacity(0.2);
    for (int i = 0; i < m_routePoints.size() - 1; ++i) {
        QPoint start = m_routePoints[i];
        QPoint end = m_routePoints[i + 1];
        painter.drawLine(start.x() + 2, start.y() + 2, end.x() + 2, end.y() + 2);
    }
    
    painter.setOpacity(1.0);
    
    // 绘制主路径
    painter.setPen(QPen(QColor("#FF6B6B"), 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    
    for (int i = 0; i < m_routePoints.size() - 1; ++i) {
        QPoint start = m_routePoints[i];
        QPoint end = m_routePoints[i + 1];
        painter.drawLine(start, end);
        
        // 绘制方向箭头
        QVector2D direction(end - start);
        if (direction.length() > 0) {
            direction.normalize();
            QVector2D perpendicular(-direction.y(), direction.x());
            
            QPoint arrowHead = end - QPoint(direction.x() * 15, direction.y() * 15);
            QPoint arrowLeft = arrowHead + QPoint(perpendicular.x() * 6, perpendicular.y() * 6);
            QPoint arrowRight = arrowHead - QPoint(perpendicular.x() * 6, perpendicular.y() * 6);
            
            QPolygon arrow;
            arrow << end << arrowLeft << arrowRight;
            painter.setBrush(QColor("#FF6B6B"));
            painter.drawPolygon(arrow);
        }
    }
    
    // 绘制动画流水线效果
    painter.setPen(QPen(QColor("#FFFFFF"), 2, Qt::DashLine));
    QPen pen = painter.pen();
    pen.setDashOffset(animationOffset);
    painter.setPen(pen);
    
    for (int i = 0; i < m_routePoints.size() - 1; ++i) {
        QPoint start = m_routePoints[i];
        QPoint end = m_routePoints[i + 1];
        painter.drawLine(start, end);
    }
    
    // 绘制起点标记
    if (!m_routePoints.isEmpty()) {
        QPoint startPoint = m_routePoints.first();
        painter.setPen(QPen(QColor("#28A745"), 2));
        painter.setBrush(QColor("#28A745"));
        painter.drawEllipse(startPoint.x() - 10, startPoint.y() - 10, 20, 20);
        
        painter.setPen(QColor("#FFFFFF"));
        painter.setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
        painter.drawText(startPoint.x() - 5, startPoint.y() + 3, "起");
    }
    
    // 绘制终点标记
    if (!m_routePoints.isEmpty()) {
        QPoint endPoint = m_routePoints.last();
        painter.setPen(QPen(QColor("#DC3545"), 2));
        painter.setBrush(QColor("#DC3545"));
        painter.drawEllipse(endPoint.x() - 10, endPoint.y() - 10, 20, 20);
        
        painter.setPen(QColor("#FFFFFF"));
        painter.setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
        painter.drawText(endPoint.x() - 5, endPoint.y() + 3, "终");
    }
    
    // 绘制路径节点
    painter.setPen(QPen(QColor("#007AFF"), 2));
    painter.setBrush(QColor("#007AFF"));
    for (int i = 1; i < m_routePoints.size() - 1; ++i) {
        QPoint point = m_routePoints[i];
        painter.drawEllipse(point.x() - 4, point.y() - 4, 8, 8);
    }
}

void HospitalMapWidget::drawLabels(QPainter& painter)
{
    // 绘制指北针
    painter.save();
    painter.translate(750, 50);
    painter.setPen(QPen(QColor("#6C757D"), 2));
    painter.setBrush(QColor("#FFFFFF"));
    painter.drawEllipse(-25, -25, 50, 50);
    
    painter.setPen(QColor("#DC3545"));
    painter.drawLine(0, -20, 0, -5);
    painter.drawText(-10, -25, "N");
    
    painter.restore();
}

QString HospitalMapWidget::getDepartmentAt(const QPoint& pos) const
{
    for (auto it = m_departments.begin(); it != m_departments.end(); ++it) {
        if (it.value().mapRect.contains(pos)) {
            return it.key();
        }
    }
    return QString();
}

QRect HospitalMapWidget::getScaledRect(const QRect& rect) const
{
    return QRect(rect.x(), rect.y(), rect.width(), rect.height());
}

// MapWidget 实现
MapWidget::MapWidget(QWidget *parent)
    : QWidget(parent)
    , m_currentZoom(1.0)
    , m_isRouteMode(false)
    , m_currentFloor("全部")
{
    initializeDepartments();
    setupUI();
    applyModernStyle();
    
    // 初始化搜索定时器
    m_searchTimer = new QTimer(this);
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(300);
    connect(m_searchTimer, &QTimer::timeout, this, &MapWidget::updateDepartmentList);
}

void MapWidget::setupUI()
{
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    m_mainLayout->setSpacing(10);
    
    // 创建分割器
    m_mainSplitter = new QSplitter(Qt::Horizontal);
    
    setupLeftPanel();
    setupRightPanel();
    
    m_mainSplitter->addWidget(m_leftPanel);
    m_mainSplitter->addWidget(m_rightPanel);
    m_mainSplitter->setStretchFactor(0, 1);
    m_mainSplitter->setStretchFactor(1, 3);
    
    m_mainLayout->addWidget(m_mainSplitter);
    
    // 初始加载
    updateDepartmentList();
}

void MapWidget::setupLeftPanel()
{
    m_leftPanel = new QWidget;
    m_leftPanel->setMaximumWidth(350);
    m_leftLayout = new QVBoxLayout(m_leftPanel);
    m_leftLayout->setSpacing(15);
    
    // 搜索区域
    m_searchGroup = new QGroupBox("🔍 查找科室");
    QVBoxLayout* searchLayout = new QVBoxLayout(m_searchGroup);
    
    // 搜索框
    QHBoxLayout* searchInputLayout = new QHBoxLayout;
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("输入科室名称或服务类型...");
    m_btnClearSearch = new QPushButton("✖");
    m_btnClearSearch->setFixedSize(30, 30);
    
    searchInputLayout->addWidget(m_searchEdit);
    searchInputLayout->addWidget(m_btnClearSearch);
    
    // 楼层筛选
    QHBoxLayout* floorLayout = new QHBoxLayout;
    floorLayout->addWidget(new QLabel("楼层:"));
    m_floorCombo = new QComboBox;
    m_floorCombo->addItems({"全部", "1楼", "2楼", "3楼", "4楼", "5楼"});
    floorLayout->addWidget(m_floorCombo);
    floorLayout->addStretch();
    
    searchLayout->addLayout(searchInputLayout);
    searchLayout->addLayout(floorLayout);
    
    // 科室列表
    m_departmentGroup = new QGroupBox("🏥 科室列表");
    QVBoxLayout* listLayout = new QVBoxLayout(m_departmentGroup);
    
    m_listInfo = new QLabel("显示全部科室");
    m_listInfo->setStyleSheet("color: #6C757D; font-size: 12px;");
    
    m_departmentList = new QListWidget;
    m_departmentList->setMinimumHeight(300);
    
    listLayout->addWidget(m_listInfo);
    listLayout->addWidget(m_departmentList);
    
    // 快捷功能
    m_quickGroup = new QGroupBox("⚡ 快捷导航");
    QGridLayout* quickLayout = new QGridLayout(m_quickGroup);
    
    m_btnEmergency = new QPushButton("🚑 急诊科");
    m_btnParking = new QPushButton("🅿️ 停车场");
    m_btnPharmacy = new QPushButton("💊 药房");
    m_btnCashier = new QPushButton("💳 收费处");
    
    quickLayout->addWidget(m_btnEmergency, 0, 0);
    quickLayout->addWidget(m_btnParking, 0, 1);
    quickLayout->addWidget(m_btnPharmacy, 1, 0);
    quickLayout->addWidget(m_btnCashier, 1, 1);
    
    // 添加到左侧布局
    m_leftLayout->addWidget(m_searchGroup);
    m_leftLayout->addWidget(m_departmentGroup);
    m_leftLayout->addWidget(m_quickGroup);
    m_leftLayout->addStretch();
    
    // 连接信号
    connect(m_searchEdit, &QLineEdit::textChanged, this, [this]() {
        m_searchTimer->start();
    });
    connect(m_btnClearSearch, &QPushButton::clicked, this, &MapWidget::onClearSelection);
    connect(m_floorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MapWidget::onFloorChanged);
    connect(m_departmentList, &QListWidget::itemClicked, this, &MapWidget::onDepartmentSelected);
    connect(m_departmentList, &QListWidget::itemDoubleClicked, this, &MapWidget::onDepartmentDoubleClicked);
    
    // 快捷按钮连接
    connect(m_btnEmergency, &QPushButton::clicked, [this]() {
        highlightDepartment("急诊科");
    });
    connect(m_btnParking, &QPushButton::clicked, [this]() {
        highlightDepartment("停车场A");
    });
    connect(m_btnPharmacy, &QPushButton::clicked, [this]() {
        highlightDepartment("药房");
    });
    connect(m_btnCashier, &QPushButton::clicked, [this]() {
        highlightDepartment("收费处");
    });
}

void MapWidget::setupRightPanel()
{
    m_rightPanel = new QWidget;
    m_rightLayout = new QVBoxLayout(m_rightPanel);
    m_rightLayout->setSpacing(10);
    
    setupToolbar();
    
    // 地图区域
    m_mapScrollArea = new QScrollArea;
    m_mapScrollArea->setWidgetResizable(true);
    m_mapScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_mapScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    m_mapWidget = new HospitalMapWidget;
    m_mapWidget->setDepartments(m_departments);
    m_mapScrollArea->setWidget(m_mapWidget);
    
    // 信息面板
    m_infoGroup = new QGroupBox("ℹ️ 位置信息");
    m_infoGroup->setMaximumHeight(150);
    QVBoxLayout* infoLayout = new QVBoxLayout(m_infoGroup);
    
    m_infoTitle = new QLabel("请选择科室查看详细信息");
    m_infoTitle->setStyleSheet("font-weight: bold; font-size: 14px; color: #007AFF;");
    
    m_infoContent = new QLabel("点击左侧科室列表或地图上的科室区域");
    m_infoContent->setWordWrap(true);
    m_infoContent->setStyleSheet("color: #6C757D;");
    
    m_btnShowRoute = new QPushButton("🗺️ 显示路线");
    m_btnShowRoute->setEnabled(false);
    
    infoLayout->addWidget(m_infoTitle);
    infoLayout->addWidget(m_infoContent);
    infoLayout->addWidget(m_btnShowRoute);
    
    // 添加到右侧布局
    m_rightLayout->addWidget(m_toolbar);
    m_rightLayout->addWidget(m_mapScrollArea);
    m_rightLayout->addWidget(m_infoGroup);
    
    // 连接地图信号
    connect(m_mapWidget, &HospitalMapWidget::departmentClicked,
            this, &MapWidget::onMapDepartmentClicked);
    connect(m_btnShowRoute, &QPushButton::clicked, this, &MapWidget::onGetRoute);
}

void MapWidget::setupToolbar()
{
    m_toolbar = new QWidget;
    m_toolbarLayout = new QHBoxLayout(m_toolbar);
    m_toolbarLayout->setContentsMargins(0, 0, 0, 0);
    
    // 标题
    m_titleLabel = new QLabel("🏥 医院院内导航");
    m_titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #007AFF;");
    
    // 地图控制按钮
    m_btnZoomIn = new QPushButton("🔍+");
    m_btnZoomOut = new QPushButton("🔍-");
    m_btnResetView = new QPushButton("🏠");
    m_btnGetRoute = new QPushButton("🗺️ 路线规划");
    
    m_btnZoomIn->setFixedSize(40, 40);
    m_btnZoomOut->setFixedSize(40, 40);
    m_btnResetView->setFixedSize(40, 40);
    
    m_toolbarLayout->addWidget(m_titleLabel);
    m_toolbarLayout->addStretch();
    m_toolbarLayout->addWidget(m_btnGetRoute);
    m_toolbarLayout->addWidget(m_btnZoomOut);
    m_toolbarLayout->addWidget(m_btnZoomIn);
    m_toolbarLayout->addWidget(m_btnResetView);
    
    // 连接信号
    connect(m_btnZoomIn, &QPushButton::clicked, this, &MapWidget::onZoomIn);
    connect(m_btnZoomOut, &QPushButton::clicked, this, &MapWidget::onZoomOut);
    connect(m_btnResetView, &QPushButton::clicked, this, &MapWidget::onResetView);
    connect(m_btnGetRoute, &QPushButton::clicked, this, &MapWidget::onGetRoute);
}

void MapWidget::initializeDepartments()
{
    // 初始化科室数据
    m_departments.clear();
    
    // 1楼科室
    DepartmentInfo emergency;
    emergency.name = "急诊科";
    emergency.description = "24小时急诊医疗服务";
    emergency.location = "1楼东侧";
    emergency.hours = "24小时";
    emergency.phone = "120";
    emergency.mapRect = QRect(600, 450, 120, 80);
    emergency.highlightColor = QColor("#FF6B6B");
    emergency.floor = "1楼";
    emergency.building = "门诊楼";
    
    DepartmentInfo outpatient;
    outpatient.name = "门诊大厅";
    outpatient.description = "挂号、导诊、咨询服务";
    outpatient.location = "1楼中央";
    outpatient.hours = "8:00-17:00";
    outpatient.phone = "0571-12345";
    outpatient.mapRect = QRect(350, 450, 150, 80);
    outpatient.highlightColor = QColor("#4ECDC4");
    outpatient.floor = "1楼";
    
    DepartmentInfo pharmacy;
    pharmacy.name = "药房";
    pharmacy.description = "处方药品调配发放";
    pharmacy.location = "1楼南侧";
    pharmacy.hours = "8:00-17:00";
    pharmacy.phone = "0571-12346";
    pharmacy.mapRect = QRect(80, 450, 100, 80);
    pharmacy.highlightColor = QColor("#45B7D1");
    pharmacy.floor = "1楼";
    
    DepartmentInfo cashier;
    cashier.name = "收费处";
    cashier.description = "医疗费用结算";
    cashier.location = "1楼西侧";
    cashier.hours = "8:00-17:00";
    cashier.phone = "0571-12347";
    cashier.mapRect = QRect(200, 450, 120, 80);
    cashier.highlightColor = QColor("#96CEB4");
    cashier.floor = "1楼";
    
    // 2楼科室
    DepartmentInfo internal;
    internal.name = "内科";
    internal.description = "内科疾病诊治";
    internal.location = "2楼东侧";
    internal.hours = "8:00-17:00";
    internal.phone = "0571-12348";
    internal.mapRect = QRect(600, 350, 120, 80);
    internal.highlightColor = QColor("#FFEAA7");
    internal.floor = "2楼";
    
    DepartmentInfo surgery;
    surgery.name = "外科";
    surgery.description = "外科手术治疗";
    surgery.location = "2楼西侧";
    surgery.hours = "8:00-17:00";
    surgery.phone = "0571-12349";
    surgery.mapRect = QRect(80, 350, 120, 80);
    surgery.highlightColor = QColor("#DDA0DD");
    surgery.floor = "2楼";
    
    DepartmentInfo laboratory;
    laboratory.name = "检验科";
    laboratory.description = "医学检验服务";
    laboratory.location = "2楼中央";
    laboratory.hours = "8:00-17:00";
    laboratory.phone = "0571-12350";
    laboratory.mapRect = QRect(350, 350, 120, 80);
    laboratory.highlightColor = QColor("#74B9FF");
    laboratory.floor = "2楼";
    
    // 3楼科室
    DepartmentInfo radiology;
    radiology.name = "放射科";
    radiology.description = "医学影像诊断";
    radiology.location = "3楼东侧";
    radiology.hours = "8:00-17:00";
    radiology.phone = "0571-12351";
    radiology.mapRect = QRect(600, 250, 120, 80);
    radiology.highlightColor = QColor("#FFB8B8");
    radiology.floor = "3楼";
    
    DepartmentInfo pediatrics;
    pediatrics.name = "儿科";
    pediatrics.description = "儿童疾病诊治";
    pediatrics.location = "3楼西侧";
    pediatrics.hours = "8:00-17:00";
    pediatrics.phone = "0571-12352";
    pediatrics.mapRect = QRect(80, 250, 120, 80);
    pediatrics.highlightColor = QColor("#FFD93D");
    pediatrics.floor = "3楼";
    
    DepartmentInfo gynecology;
    gynecology.name = "妇产科";
    gynecology.description = "妇科产科诊疗";
    gynecology.location = "3楼中央";
    gynecology.hours = "8:00-17:00";
    gynecology.phone = "0571-12353";
    gynecology.mapRect = QRect(350, 250, 120, 80);
    gynecology.highlightColor = QColor("#FF8FA3");
    gynecology.floor = "3楼";
    
    // 停车场
    DepartmentInfo parkingA;
    parkingA.name = "停车场A";
    parkingA.description = "地下停车场";
    parkingA.location = "地下一层";
    parkingA.hours = "24小时";
    parkingA.phone = "0571-12354";
    parkingA.mapRect = QRect(80, 550, 300, 40);
    parkingA.highlightColor = QColor("#CCCCCC");
    parkingA.floor = "地下";
    
    // 添加到科室映射
    m_departments["急诊科"] = emergency;
    m_departments["门诊大厅"] = outpatient;
    m_departments["药房"] = pharmacy;
    m_departments["收费处"] = cashier;
    m_departments["内科"] = internal;
    m_departments["外科"] = surgery;
    m_departments["检验科"] = laboratory;
    m_departments["放射科"] = radiology;
    m_departments["儿科"] = pediatrics;
    m_departments["妇产科"] = gynecology;
    m_departments["停车场A"] = parkingA;
    
    // 按楼层分组
    m_floorDepartments["1楼"] = {"急诊科", "门诊大厅", "药房", "收费处"};
    m_floorDepartments["2楼"] = {"内科", "外科", "检验科"};
    m_floorDepartments["3楼"] = {"放射科", "儿科", "妇产科"};
    m_floorDepartments["地下"] = {"停车场A"};
}

void MapWidget::updateDepartmentList()
{
    m_departmentList->clear();
    
    QString keyword = m_searchEdit->text().trimmed();
    QString floor = m_floorCombo->currentText();
    
    QStringList filteredDepartments;
    
    // 筛选科室
    for (auto it = m_departments.begin(); it != m_departments.end(); ++it) {
        const QString& name = it.key();
        const DepartmentInfo& info = it.value();
        
        // 楼层筛选
        if (floor != "全部" && info.floor != floor) {
            continue;
        }
        
        // 关键词筛选
        if (!keyword.isEmpty()) {
            if (!name.contains(keyword, Qt::CaseInsensitive) &&
                !info.description.contains(keyword, Qt::CaseInsensitive)) {
                continue;
            }
        }
        
        filteredDepartments.append(name);
    }
    
    // 添加到列表
    for (const QString& name : filteredDepartments) {
        QListWidgetItem* item = new QListWidgetItem(name);
        const DepartmentInfo& info = m_departments[name];
        
        item->setToolTip(QString("%1\n%2\n%3").arg(info.description, info.location, info.hours));
        item->setData(Qt::UserRole, name);
        
        m_departmentList->addItem(item);
    }
    
    // 更新信息标签
    if (keyword.isEmpty() && floor == "全部") {
        m_listInfo->setText(QString("显示全部 %1 个科室").arg(filteredDepartments.size()));
    } else {
        m_listInfo->setText(QString("筛选结果: %1 个科室").arg(filteredDepartments.size()));
    }
}

void MapWidget::highlightDepartment(const QString& departmentName)
{
    if (m_departments.contains(departmentName)) {
        m_selectedDepartment = departmentName;
        m_mapWidget->highlightDepartment(departmentName);
        
        // 更新信息面板
        const DepartmentInfo& info = m_departments[departmentName];
        m_infoTitle->setText(info.name);
        m_infoContent->setText(formatDepartmentInfo(info));
        m_btnShowRoute->setEnabled(true);
        
        // 选中列表项
        for (int i = 0; i < m_departmentList->count(); ++i) {
            QListWidgetItem* item = m_departmentList->item(i);
            if (item->data(Qt::UserRole).toString() == departmentName) {
                m_departmentList->setCurrentItem(item);
                break;
            }
        }
    }
}

QString MapWidget::formatDepartmentInfo(const DepartmentInfo& info) const
{
    return QString("📍 位置: %1\n⏰ 时间: %2\n📞 电话: %3\n🏢 楼层: %4\n\n%5")
           .arg(info.location)
           .arg(info.hours)
           .arg(info.phone)
           .arg(info.floor)
           .arg(info.description);
}

void MapWidget::applyModernStyle()
{
    setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #E5E5EA;
            border-radius: 12px;
            margin-top: 12px;
            padding-top: 15px;
            background-color: white;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 15px;
            padding: 0 10px 0 10px;
            color: #1D1D1F;
            font-size: 14px;
        }
        QListWidget {
            border: 1px solid #D1D1D6;
            border-radius: 8px;
            background-color: white;
            outline: none;
        }
        QListWidget::item {
            padding: 12px 15px;
            border-bottom: 1px solid #F2F2F7;
            border-radius: 6px;
            margin: 2px;
        }
        QListWidget::item:hover {
            background-color: #F2F8FF;
            border-color: #007AFF;
        }
        QListWidget::item:selected {
            background-color: #007AFF;
            color: white;
        }
        QLineEdit {
            border: 2px solid #D1D1D6;
            border-radius: 8px;
            padding: 8px 12px;
            font-size: 14px;
            background-color: white;
        }
        QLineEdit:focus {
            border-color: #007AFF;
        }
        QComboBox {
            border: 2px solid #D1D1D6;
            border-radius: 8px;
            padding: 6px 12px;
            background-color: white;
            min-width: 100px;
        }
        QComboBox:focus {
            border-color: #007AFF;
        }
        QPushButton {
            background-color: #007AFF;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 16px;
            font-weight: bold;
            font-size: 13px;
        }
        QPushButton:hover {
            background-color: #0056CC;
            transform: translateY(-1px);
        }
        QPushButton:pressed {
            background-color: #004499;
        }
        QPushButton:disabled {
            background-color: #C7C7CC;
        }
        QScrollArea {
            border: 1px solid #E5E5EA;
            border-radius: 8px;
            background-color: white;
        }
        QLabel {
            color: #1D1D1F;
        }
    )");
}

// 槽函数实现
void MapWidget::onSearchTextChanged()
{
    m_searchTimer->start();
}

void MapWidget::onDepartmentSelected()
{
    QListWidgetItem* item = m_departmentList->currentItem();
    if (item) {
        QString departmentName = item->data(Qt::UserRole).toString();
        highlightDepartment(departmentName);
    }
}

void MapWidget::onDepartmentDoubleClicked()
{
    QListWidgetItem* item = m_departmentList->currentItem();
    if (item) {
        QString departmentName = item->data(Qt::UserRole).toString();
        showLocationDetails(departmentName);
    }
}

void MapWidget::onFloorChanged()
{
    updateDepartmentList();
}

void MapWidget::onClearSelection()
{
    m_searchEdit->clear();
    m_floorCombo->setCurrentIndex(0);
    m_departmentList->clearSelection();
    m_mapWidget->clearHighlight();
    m_selectedDepartment.clear();
    
    m_infoTitle->setText("请选择科室查看详细信息");
    m_infoContent->setText("点击左侧科室列表或地图上的科室区域");
    m_btnShowRoute->setEnabled(false);
}

void MapWidget::onMapDepartmentClicked(const QString& departmentName)
{
    highlightDepartment(departmentName);
}

void MapWidget::onZoomIn()
{
    m_currentZoom = qMin(3.0, m_currentZoom * 1.2);
    m_mapWidget->setZoomLevel(m_currentZoom);
}

void MapWidget::onZoomOut()
{
    m_currentZoom = qMax(0.5, m_currentZoom / 1.2);
    m_mapWidget->setZoomLevel(m_currentZoom);
}

void MapWidget::onResetView()
{
    m_currentZoom = 1.0;
    m_mapWidget->setZoomLevel(m_currentZoom);
    m_mapScrollArea->horizontalScrollBar()->setValue(0);
    m_mapScrollArea->verticalScrollBar()->setValue(0);
}

void MapWidget::onGetRoute()
{
    if (m_selectedDepartment.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择目标科室");
        return;
    }
    
    // 简化版路径规划对话框
    QString message = QString("路线规划\n\n从: 医院正门\n到: %1\n\n路线: 正门 → 门诊大厅 → %2")
                      .arg(m_selectedDepartment)
                      .arg(m_departments[m_selectedDepartment].location);
    
    QMessageBox::information(this, "路线规划", message);
    
    // 在地图上显示路径
    m_mapWidget->showRoute("门诊大厅", m_selectedDepartment);
}

// 占位方法实现
void MapWidget::onMapRouteRequested(const QString& from, const QString& to) 
{
    if (m_departments.contains(from) && m_departments.contains(to)) {
        // 计算并显示路径
        showRoute(from, to);
        
        // 更新信息面板显示路径信息
        QString routeInfo = QString("🗺️ 路径规划\n\n从: %1\n到: %2\n\n导航说明:\n1. 从 %3 出发\n2. 沿主走廊前行\n3. 到达 %4")
                           .arg(from, to, m_departments[from].location, m_departments[to].location);
        
        m_infoTitle->setText("路径导航");
        m_infoContent->setText(routeInfo);
    }
}

void MapWidget::onShowEmergencyRoute() 
{
    highlightDepartment("急诊科");
    m_mapWidget->showRoute("门诊大厅", "急诊科");
    
    // 显示紧急路线详情
    QString emergencyInfo = "🚨 紧急路线\n\n目标: 急诊科\n楼层: 1楼东侧\n\n快速导航:\n1. 从正门进入门诊大厅\n2. 右转走主走廊\n3. 直行至东侧急诊科\n\n⚠️ 急诊科24小时开放\n📞 急救电话: 120";
    m_infoTitle->setText("🚨 急诊科导航");
    m_infoContent->setText(emergencyInfo);
    m_btnShowRoute->setEnabled(true);
}

void MapWidget::updateRouteAnimation() 
{
    // 更新路径动画显示
    if (m_mapWidget && !m_selectedDepartment.isEmpty()) {
        m_mapWidget->update(); // 触发重绘以更新动画
    }
}

void MapWidget::showLocationDetails(const QString& departmentName) 
{
    if (!m_departments.contains(departmentName)) return;
    
    const DepartmentInfo& info = m_departments[departmentName];
    
    // 创建详细信息对话框
    QDialog* detailDialog = new QDialog(this);
    detailDialog->setWindowTitle(QString("%1 - 详细信息").arg(info.name));
    detailDialog->setFixedSize(400, 500);
    detailDialog->setAttribute(Qt::WA_DeleteOnClose);
    
    QVBoxLayout* layout = new QVBoxLayout(detailDialog);
    
    // 标题
    QLabel* titleLabel = new QLabel(info.name);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #007AFF; margin: 10px 0;");
    layout->addWidget(titleLabel);
    
    // 基本信息
    QGroupBox* basicGroup = new QGroupBox("📋 基本信息");
    QVBoxLayout* basicLayout = new QVBoxLayout(basicGroup);
    
    QLabel* locationLabel = new QLabel(QString("📍 位置: %1").arg(info.location));
    QLabel* floorLabel = new QLabel(QString("🏢 楼层: %1").arg(info.floor));
    QLabel* hoursLabel = new QLabel(QString("⏰ 开放时间: %1").arg(info.hours));
    QLabel* phoneLabel = new QLabel(QString("📞 联系电话: %1").arg(info.phone));
    
    basicLayout->addWidget(locationLabel);
    basicLayout->addWidget(floorLabel);
    basicLayout->addWidget(hoursLabel);
    basicLayout->addWidget(phoneLabel);
    layout->addWidget(basicGroup);
    
    // 服务说明
    QGroupBox* serviceGroup = new QGroupBox("ℹ️ 服务说明");
    QVBoxLayout* serviceLayout = new QVBoxLayout(serviceGroup);
    
    QLabel* descLabel = new QLabel(info.description);
    descLabel->setWordWrap(true);
    serviceLayout->addWidget(descLabel);
    layout->addWidget(serviceGroup);
    
    // 导航信息
    QGroupBox* navGroup = new QGroupBox("🗺️ 导航信息");
    QVBoxLayout* navLayout = new QVBoxLayout(navGroup);
    
    QString navInfo;
    if (info.floor == "1楼") {
        navInfo = "1. 从正门进入门诊大厅\n2. 根据科室位置指示前往\n3. 寻找对应科室标识";
    } else if (info.floor == "2楼") {
        navInfo = "1. 从正门进入门诊大厅\n2. 乘坐电梯或楼梯至2楼\n3. 根据楼层平面图找到科室";
    } else if (info.floor == "3楼") {
        navInfo = "1. 从正门进入门诊大厅\n2. 乘坐电梯或楼梯至3楼\n3. 根据楼层平面图找到科室";
    } else if (info.floor == "地下") {
        navInfo = "1. 从地面入口进入\n2. 乘坐电梯至地下层\n3. 按照停车场指示标识";
    }
    
    QLabel* navLabel = new QLabel(navInfo);
    navLabel->setWordWrap(true);
    navLayout->addWidget(navLabel);
    layout->addWidget(navGroup);
    
    // 按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    QPushButton* showRouteBtn = new QPushButton("🗺️ 显示路线");
    QPushButton* closeBtn = new QPushButton("关闭");
    
    buttonLayout->addWidget(showRouteBtn);
    buttonLayout->addWidget(closeBtn);
    layout->addLayout(buttonLayout);
    
    // 连接信号
    connect(showRouteBtn, &QPushButton::clicked, [this, departmentName, detailDialog]() {
        highlightDepartment(departmentName);
        m_mapWidget->showRoute("门诊大厅", departmentName);
        detailDialog->close();
    });
    
    connect(closeBtn, &QPushButton::clicked, detailDialog, &QDialog::close);
    
    // 应用样式
    basicGroup->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #D1D1D6; border-radius: 8px; margin: 5px 0; padding: 10px; }");
    serviceGroup->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #D1D1D6; border-radius: 8px; margin: 5px 0; padding: 10px; }");
    navGroup->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #D1D1D6; border-radius: 8px; margin: 5px 0; padding: 10px; }");
    
    detailDialog->show();
}

void MapWidget::filterDepartments(const QString& keyword) 
{
    m_searchEdit->setText(keyword);
    updateDepartmentList();
}

void MapWidget::showRoute(const QString& from, const QString& to) 
{
    if (m_departments.contains(from) && m_departments.contains(to)) {
        m_mapWidget->showRoute(from, to);
        
        // 更新路线状态
        m_routeStart = from;
        m_routeEnd = to;
        m_isRouteMode = true;
        
        // 启动路线动画
        updateRouteAnimation();
    }
}

void MapWidget::updateMapDisplay() 
{
    if (m_mapWidget) {
        m_mapWidget->update();
    }
}

QStringList MapWidget::searchDepartments(const QString& keyword) const 
{
    QStringList results;
    
    for (auto it = m_departments.begin(); it != m_departments.end(); ++it) {
        const QString& name = it.key();
        const DepartmentInfo& info = it.value();
        
        if (name.contains(keyword, Qt::CaseInsensitive) ||
            info.description.contains(keyword, Qt::CaseInsensitive) ||
            info.location.contains(keyword, Qt::CaseInsensitive)) {
            results << name;
        }
    }
    
    return results;
}

void MapWidget::loadFloorData() 
{
    // 重新组织楼层数据
    m_floorDepartments.clear();
    
    for (auto it = m_departments.begin(); it != m_departments.end(); ++it) {
        const QString& name = it.key();
        const DepartmentInfo& info = it.value();
        
        if (!m_floorDepartments.contains(info.floor)) {
            m_floorDepartments[info.floor] = QStringList();
        }
        m_floorDepartments[info.floor].append(name);
    }
} 