module("luci.controller.admin.smart_home", package.seeall)

function index()
  entry({"admin", "smart_home"}, alias("admin", "smart_home", "smart_home"), _("智能家居")).index = true
  entry({"admin", "smart_home", "smart_home"}, cbi("smart_home/weather"), _("天气"), 1)
end
