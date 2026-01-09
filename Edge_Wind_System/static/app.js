/**
 * 风电场DC系统故障监测与智能诊断平台 - 前端逻辑
 */

// 全局变量
let waveformChart = null;
let knowledgeGraphChart = null;
let currentSelectedDevice = null;
let pollingInterval = null;

// 初始化
$(document).ready(function() {
    initCharts();
    startPolling();
    
    // 搜索功能
    $('#workorder-search').on('input', function() {
        filterWorkOrders($(this).val());
    });
});

// ========== 初始化图表 ==========

function initCharts() {
    // 波形图
    waveformChart = echarts.init(document.getElementById('waveform-chart'));
    waveformChart.setOption({
        tooltip: {
            trigger: 'axis',
            axisPointer: { type: 'cross' }
        },
        grid: {
            left: '3%',
            right: '4%',
            bottom: '10%',
            containLabel: true
        },
        dataZoom: [
            { 
                type: 'inside', 
                xAxisIndex: [0],
                filterMode: 'filter'
            },
            { 
                type: 'inside', 
                yAxisIndex: [0],
                filterMode: 'empty'
            }
        ],
        xAxis: {
            type: 'value',
            name: '时间 (ms)',
            nameLocation: 'middle',
            nameGap: 30
        },
        yAxis: {
            type: 'value',
            name: '电压 (V)',
            nameLocation: 'middle',
            nameGap: 50
        },
        series: [{
            name: '电压波形',
            type: 'line',
            data: [],
            smooth: true,
            lineStyle: { color: '#2563eb', width: 1.5 },
            symbol: 'none',
            sampling: 'lttb',
            areaStyle: {
                color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
                    { offset: 0, color: 'rgba(37, 99, 235, 0.3)' },
                    { offset: 1, color: 'rgba(37, 99, 235, 0.05)' }
                ])
            }
        }]
    });

    // 知识图谱
    knowledgeGraphChart = echarts.init(document.getElementById('knowledge-graph-chart'));
    knowledgeGraphChart.setOption({
        title: {
            text: '故障诊断知识图谱',
            left: 'center',
            top: 10,
            textStyle: { fontSize: 16 }
        },
        tooltip: {
            trigger: 'item',
            formatter: function(params) {
                if (params.dataType === 'node') {
                    return `<b>${params.data.name}</b><br/>${params.data.value || ''}`;
                }
                return '';
            }
        },
        legend: {
            data: ['故障', '根本原因', '解决方案'],
            bottom: 10,
            left: 'center'
        },
        series: [{
            type: 'graph',
            layout: 'force',
            force: {
                repulsion: 1000,
                gravity: 0.1,
                edgeLength: 150,
                layoutAnimation: true
            },
            roam: true,
            label: {
                show: true,
                position: 'right',
                formatter: '{b}'
            },
            edgeSymbol: ['none', 'arrow'],
            edgeSymbolSize: [0, 10],
            categories: [
                { name: '故障', itemStyle: { color: '#ef4444' } },
                { name: '根本原因', itemStyle: { color: '#f59e0b' } },
                { name: '解决方案', itemStyle: { color: '#10b981' } }
            ],
            lineStyle: {
                color: 'source',
                curveness: 0.3
            },
            emphasis: {
                focus: 'adjacency',
                lineStyle: { width: 4 }
            },
            data: [],
            links: []
        }]
    });

    // 应用统一的缩放功能（以鼠标为中心的缩放、X/Y轴独立缩放、Grab模式拖拽）
    const waveformChartDom = document.getElementById('waveform-chart');
    if (waveformChart && waveformChartDom && typeof setupChartZoom === 'function') {
        setupChartZoom(waveformChart, waveformChartDom);
    }
    
    // 响应式
    window.addEventListener('resize', function() {
        waveformChart?.resize();
        knowledgeGraphChart?.resize();
    });
}

// ========== 数据轮询 ==========

function startPolling() {
    // 立即加载一次
    fetchDevices();
    updateWorkOrders();
    
    // 每1秒更新设备列表（优化响应速度）
    pollingInterval = setInterval(function() {
        fetchDevices();
    }, 1000);
    
    // 每1秒更新工单列表（优化响应速度）
    setInterval(function() {
        updateWorkOrders();
    }, 1000);
}

// ========== API 调用 ==========

function fetchDevices() {
    $.ajax({
        url: '/api/devices',
        method: 'GET',
        success: function(response) {
            updateStatistics(response.statistics);
            updateNodeList(response.devices);
            
            // 如果选中了设备，继续更新波形
            if (currentSelectedDevice) {
                loadWaveformData(currentSelectedDevice);
            }
        },
        error: function(xhr) {
            console.error('获取设备列表失败:', xhr);
        }
    });
}

function loadWaveformData(deviceId) {
    $.ajax({
        url: `/api/device/${deviceId}/data`,
        method: 'GET',
        success: function(response) {
            updateWaveformChart(response);
            
            // 检查故障
            if (response.status === 'fault' && response.fault_code && response.fault_code !== 'E00') {
                showFaultAlert(response.device_id, response.fault_code);
                showKnowledgeGraph(response.fault_code);
            } else {
                hideKnowledgeGraph();
            }
        },
        error: function(xhr) {
            console.error('加载波形数据失败:', xhr);
            waveformChart.setOption({
                title: {
                    text: '数据加载失败',
                    left: 'center',
                    textStyle: { color: '#ef4444' }
                },
                series: [{ data: [] }]
            });
        }
    });
}

// ========== 更新UI ==========

function updateStatistics(stats) {
    $('#stat-total').text(stats.total || 0);
    $('#stat-online').text(stats.online || 0);
    $('#stat-faulty').text(stats.faulty || 0);
    $('#stat-offline').text(stats.offline || 0);
}

function updateNodeList(devices) {
    const container = $('#node-list');
    
    if (!devices || devices.length === 0) {
        container.html('<div class="empty-state"><i class="bi bi-inbox"></i><div>暂无设备</div></div>');
        return;
    }

    const html = devices.map(device => {
        const statusClass = device.status === 'online' ? 'online' : 
                          device.status === 'faulty' ? 'faulty' : 'offline';
        const statusText = device.status === 'online' ? '在线' : 
                          device.status === 'faulty' ? '故障' : '离线';
        const isActive = currentSelectedDevice === device.device_id ? 'active' : '';
        
        return `
            <div class="node-card ${isActive}" onclick="selectDevice('${device.device_id}')">
                <div class="node-header">
                    <span class="node-id">
                        <span class="status-dot ${statusClass}"></span>
                        ${device.device_id}
                    </span>
                    <span class="status-badge ${statusClass}">${statusText}</span>
                </div>
                <div class="node-location">${device.location}</div>
            </div>
        `;
    }).join('');

    container.html(html);
}

function selectDevice(deviceId) {
    currentSelectedDevice = deviceId;
    
    // 更新节点列表高亮
    $('.node-card').removeClass('active');
    $(`.node-card:contains('${deviceId}')`).addClass('active');
    
    // 更新标题
    $('#waveform-title').text(`${deviceId} - 实时波形监控 (24位ADC)`);
    
    // 立即加载数据
    loadWaveformData(deviceId);
}

function updateWaveformChart(data) {
    const waveform = data.waveform || [];
    
    if (waveform.length === 0) {
        waveformChart.setOption({
            title: {
                text: '暂无数据',
                left: 'center',
                textStyle: { color: '#94a3b8' }
            },
            series: [{ data: [] }]
        });
        return;
    }
    
    // 生成时间轴（1024个点，每个点1ms）
    const timeAxis = waveform.map((_, index) => index);
    const chartData = waveform.map((v, i) => [timeAxis[i], v]);
    
    waveformChart.setOption({
        title: {
            show: false
        },
        series: [{
            data: chartData
        }]
    });
    
    // 更新副标题
    if (data.timestamp) {
        const date = new Date(data.timestamp);
        const tsBj = date.toLocaleString('zh-CN', { timeZone: 'Asia/Shanghai' });
        $('#waveform-subtitle').text(`数据最后更新: ${tsBj} | 采样点数: ${waveform.length}`);
    } else {
        $('#waveform-subtitle').text(`采样点数: ${waveform.length}`);
    }
}

function refreshWaveform() {
    if (currentSelectedDevice) {
        loadWaveformData(currentSelectedDevice);
    } else {
        alert('请先选择一个节点');
    }
}

function showFaultAlert(deviceId, faultCode) {
    const faultNames = {
        'E01': '交流窜入',
        'E02': '绝缘故障'
    };
    const faultName = faultNames[faultCode] || faultCode;
    
    $('#alert-text').text(`设备 ${deviceId} 检测到故障: ${faultCode} (${faultName})。请立即查看工单系统进行处理。`);
    $('#alert-banner').addClass('show');
}

function showKnowledgeGraph(faultCode) {
    $.ajax({
        url: `/api/knowledge_graph/${faultCode}`,
        method: 'GET',
        success: function(response) {
            if (response.nodes && response.links) {
                knowledgeGraphChart.setOption({
                    series: [{
                        data: response.nodes,
                        links: response.links,
                        categories: response.categories
                    }]
                });
                $('#knowledge-graph-container').addClass('show');
                
                // 滚动到知识图谱
                $('html, body').animate({
                    scrollTop: $('#knowledge-graph-container').offset().top - 100
                }, 500);
            }
        },
        error: function(xhr) {
            console.error('加载知识图谱失败:', xhr);
        }
    });
}

function hideKnowledgeGraph() {
    $('#knowledge-graph-container').removeClass('show');
}

function scrollToFaults() {
    $('html, body').animate({
        scrollTop: $('.workorder-card').offset().top - 100
    }, 500);
}

function updateWorkOrders() {
    $.ajax({
        url: '/api/workorders',
        method: 'GET',
        success: function(response) {
            renderWorkOrders(response.work_orders || []);
        },
        error: function(xhr) {
            console.error('加载工单失败:', xhr);
        }
    });
}

function renderWorkOrders(orders) {
    const tbody = $('#workorder-tbody');
    
    if (orders.length === 0) {
        tbody.html('<tr><td colspan="7" class="text-center text-muted py-4">暂无工单</td></tr>');
        return;
    }

    const html = orders.map(order => {
        const statusBadge = order.status === 'pending' ? 
            '<span class="badge bg-warning">待处理</span>' : 
            '<span class="badge bg-success">已修复</span>';
        
        // 确定严重度
        let severity = 'general';
        let severityText = '一般';
        if (order.fault_type.includes('交流窜入') || order.fault_type.includes('绝缘故障')) {
            severity = 'severe';
            severityText = '严重';
        }
        
        const faultTime = new Date(order.fault_time).toLocaleString('zh-CN', { timeZone: 'Asia/Shanghai' });
        
        // 截断AI推荐（如果太长）
        const recommendation = order.ai_recommendation || '';
        const shortRecommendation = recommendation.length > 50 ? 
            recommendation.substring(0, 50) + '...' : recommendation;
        
        return `
            <tr>
                <td><small>${faultTime}</small></td>
                <td><code>${order.device_id}</code></td>
                <td><small>${order.location || order.device_id}</small></td>
                <td>
                    <strong>${order.fault_type}</strong>
                    <br><small class="text-muted">${order.fault_type}</small>
                </td>
                <td>
                    <small>${shortRecommendation}</small>
                    ${recommendation.length > 50 ? '<button class="btn btn-link btn-sm p-0 ms-1" onclick="showFullRecommendation(\'' + order.id + '\')">查看详情</button>' : ''}
                </td>
                <td>
                    <span class="severity-badge ${severity}">${severityText}</span>
                    ${statusBadge}
                </td>
                <td>
                    ${order.status === 'pending' ? 
                        `<button class="btn btn-sm btn-success" onclick="markFixed(${order.id})">
                            <i class="bi bi-check-circle"></i> 标记已修复
                        </button>` : 
                        '<span class="text-muted">已处理</span>'}
                </td>
            </tr>
        `;
    }).join('');

    tbody.html(html);
}

function filterWorkOrders(keyword) {
    const rows = $('#workorder-tbody tr');
    if (!keyword) {
        rows.show();
        return;
    }
    
    keyword = keyword.toLowerCase();
    rows.each(function() {
        const text = $(this).text().toLowerCase();
        $(this).toggle(text.includes(keyword));
    });
}

function markFixed(orderId) {
    if (!confirm('确认将此工单标记为已修复？')) {
        return;
    }
    
    $.ajax({
        url: `/api/workorder/${orderId}/update`,
        method: 'POST',
        contentType: 'application/json',
        data: JSON.stringify({ status: 'fixed' }),
        success: function() {
            updateWorkOrders();
            // 如果当前选中的设备故障已修复，隐藏知识图谱
            hideKnowledgeGraph();
        },
        error: function(xhr) {
            alert('更新失败: ' + (xhr.responseJSON?.error || '未知错误'));
        }
    });
}

function showFullRecommendation(orderId) {
    // 简单实现：从工单列表中获取完整推荐
    // 实际项目中可以调用API获取详细信息
    $.ajax({
        url: '/api/workorders',
        method: 'GET',
        success: function(response) {
            const order = response.work_orders.find(o => o.id === orderId);
            if (order && order.ai_recommendation) {
                alert(order.ai_recommendation);
            }
        }
    });
}
