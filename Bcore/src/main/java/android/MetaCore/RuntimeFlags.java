package android.MetaCore;

import top.niunaijun.blackbox.core.env.BEnvironment;
import java.io.File;

/**
 * ZOFAN/TIGER runtime flags — mirrors the Samurai Engine RuntimeFlags exactly.
 * These are read by Bcore's hide/spoof system at process startup.
 * sHideRoot and sHideXposed are TRUE by default so protection is always active.
 */
public final class RuntimeFlags {
    public static volatile boolean sEnableDaemonService = false;
    public static volatile boolean sHideRoot             = true;
    public static volatile boolean sHideXposed           = true;

    public static final File JUNIT_JAR = BEnvironment.JUNIT_JAR;
    public static final File EMPTY_JAR = BEnvironment.EMPTY_JAR;

    private RuntimeFlags() {}
}
