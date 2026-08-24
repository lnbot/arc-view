#
# This file is the default set of rules to compile a Pebble application.
#
# Feel free to customize this to your needs.
#
import os.path
import sys

top = '.'
out = 'build'


def options(ctx):
    ctx.load('pebble_sdk')


def configure(ctx):
    """
    This method is used to configure your build. ctx.load(`pebble_sdk`) automatically configures
    a build for each valid platform in `targetPlatforms`. Platform-specific configuration: add your
    change after calling ctx.load('pebble_sdk') and make sure to set the correct environment first.
    Universal configuration: add your change prior to calling ctx.load('pebble_sdk').
    """
    ctx.load('pebble_sdk')


def build(ctx):
    # ------------------------------------------------------------------
    # Pre-compile codegen: mirror the SDK-generated build/src/message_keys.auto.c
    # (produced by the "message_keys" feature) into a C99 enum header in the
    # build tree.  Declaring the .c as this task's input creates a dependency,
    # so waf runs the message_key_definitions task before us, and registering it
    # in its own group (created before ctx.load() adds the platform groups)
    # guarantees the header exists before any source that #includes it compiles.
    # The enum makes the ids usable as compile-time constants (case labels,
    # #if, initializers), prefixed MKEY_ to avoid clashing with the uint32_t
    # globals declared in message_keys.auto.h.
    # ------------------------------------------------------------------
    ctx.env.PYTHON = sys.executable
    ctx.add_group('__message_keys_enum_gen__')
    message_keys_c = ctx.path.get_bld().make_node('src/message_keys.auto.c')
    message_keys_enum = ctx.path.get_bld().make_node('include/message_keys.enum.h')
    gen_script = ctx.path.get_src().find_node('scripts/gen_message_keys_enum.py')
    ctx(
        rule='${PYTHON} ${SRC[0]} ${SRC[1]} ${TGT}',
        source=[gen_script, message_keys_c],
        target=message_keys_enum,
    )

    ctx.load('pebble_sdk')

    build_worker = os.path.exists('worker_src')
    binaries = []

    cached_env = ctx.env
    for platform in ctx.env.TARGET_PLATFORMS:
        ctx.env = ctx.all_envs[platform]
        ctx.set_group(ctx.env.PLATFORM_NAME)
        app_elf = '{}/pebble-app.elf'.format(ctx.env.BUILD_DIR)
        ctx.pbl_build(source=ctx.path.ant_glob('src/c/**/*.c'), target=app_elf, bin_type='app')

        if build_worker:
            worker_elf = '{}/pebble-worker.elf'.format(ctx.env.BUILD_DIR)
            binaries.append({'platform': platform, 'app_elf': app_elf, 'worker_elf': worker_elf})
            ctx.pbl_build(source=ctx.path.ant_glob('worker_src/c/**/*.c'),
                          target=worker_elf,
                          bin_type='worker')
        else:
            binaries.append({'platform': platform, 'app_elf': app_elf})
    ctx.env = cached_env

    ctx.set_group('bundle')
    ctx.pbl_bundle(binaries=binaries,
                   js=ctx.path.ant_glob(['src/pkjs/**/*.js',
                                         'src/pkjs/**/*.json',
                                         'src/common/**/*.js']),
                   js_entry_file='src/pkjs/index.js')
