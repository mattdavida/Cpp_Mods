local projectName = "MGSMod"

target(projectName)
    add_rules("ue4ss.mod")
    add_includedirs(".")
    add_includedirs("..") -- Include parent directory for UE4SSUtils
    add_files("dllmain.cpp")
    add_files("../UE4SSUtils.cpp") -- Include the utils implementation 