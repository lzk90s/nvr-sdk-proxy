add_rules("mode.debug", "mode.release")

set_languages("cxx11")
set_warnings("all")
add_includedirs("$(projectdir)", "$(projectdir)/thirdparty", "/usr/local/include")
add_syslinks("dl", "pthread")

target("dahua_stub_impl")
	set_kind("shared")
	add_files("3rdsdk/vendor/dahua/**.cc")
	add_links("DPSDK_Core", "dhplay", "asound")
	local sdk_root="/root/dahua_sdk"
	add_includedirs(sdk_root .. "/Include")
	add_linkdirs(sdk_root .. "/lib")

target("dahuanvr_stub_impl")
	set_kind("shared")
	add_files("3rdsdk/vendor/dahuanvr/**.cc")
	add_links("dhnetsdk", "avnetsdk")
	local sdk_root="/root/dahuanvr_sdk"
	add_includedirs(sdk_root .. "/include")
	add_linkdirs(sdk_root .. "/lib")

target("hikvision_stub_impl")
	set_kind("shared")
	add_files("3rdsdk/vendor/hikvision/**.cc")
	local sdk_root="/root/hikvision_sdk"
	add_includedirs(sdk_root .. "/include")
	add_linkdirs(sdk_root .. "/lib")

target("hikvisionnvr_stub_impl")
	set_kind("shared")
	add_files("3rdsdk/vendor/hikvisionnvr/**.cc")
	add_links("hcnetsdk", "HCCore", "PlayCtrl")
	local sdk_root="/root/hikvisionnvr_sdk"
	add_includedirs(sdk_root .. "/include")
	add_linkdirs(sdk_root .. "/lib", sdk_root .. "/lib/HCNetSDKCom")

target("sdk_proxy_server")
	set_kind("binary")
	add_files("server/**.cc")
	add_deps("dahuanvr_stub_impl", "hikvision_stub_impl", "hikvisionnvr_stub_impl", "dahua_stub_impl")
	add_links("brpc", "gflags", "protobuf", "leveldb", "z", "ssl", "crypto")