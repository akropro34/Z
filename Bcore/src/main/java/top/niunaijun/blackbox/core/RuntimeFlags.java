package top.niunaijun.blackbox.core;

import top.niunaijun.blackbox.core.env.BEnvironment;
import java.io.File;

/**
 * Runtime flags that control native-layer behaviour.
 * Mirrors android.MetaCore.RuntimeFlags from sources.zip exactly.
 *
 * sHideRoot:   triggers su-binary IO redirect rules in IOCore.enableRedirect()
 * sHideXposed: triggers NativeCore.hideXposed() in BActivityThread.loadXposed()
 */
public final class RuntimeFlags {

    /** Hide root binaries from /proc/self/maps and IO layer */
    public static volatile boolean sEnableDaemonService = false;

    /** Activate native IO-hook filter for su paths */
    public static volatile boolean sHideRoot    = true;

    /** Activate NativeCore.hideXposed() in BActivityThread.loadXposed() */
    public static volatile boolean sHideXposed  = true;

    public static final File JUNIT_JAR = BEnvironment.JUNIT_JAR;
    public static final File EMPTY_JAR = BEnvironment.EMPTY_JAR;

    private RuntimeFlags() {}
}
