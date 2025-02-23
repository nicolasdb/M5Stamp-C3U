Import("env")

def before_build(source, target, env):
    env.Replace(MKSPIFFSTOOL=env.get("PROJECT_DIR") + '/.pio/packages/tool-mkspiffs/mkspiffs')
    env.Replace(SPIFFSTOOL=env.get("PROJECT_DIR") + '/.pio/packages/tool-mkspiffs/mkspiffs')

env.AddPreAction("buildfs", before_build)
env.AddPreAction("uploadfs", before_build)
