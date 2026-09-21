load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "attendance_simulator",
    srcs = ["main.cpp"],
    deps = ["//lib:api"],
    linkopts = ["-lpaho-mqtt3c"]
)
