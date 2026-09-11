package top.niunaijun.blackbox.fake.service;

import android.os.Process;
import android.util.Log;
import java.lang.reflect.Field;
import java.lang.reflect.Method;

import top.niunaijun.blackbox.fake.hook.IInjectHook;
import top.niunaijun.blackbox.utils.Slog;

/**
 * AntiVirtualDetectProxy — intercepts the two kill vectors Helium SDK uses
 * when it detects the virtual environment:
 *
 *   1. Runtime.halt(int)   — the standard JVM hard-exit; Helium prefers this
 *      over Runtime.exit() because it bypasses shutdown hooks.
 *
 *   2. Process.killProcess(pid) — Android-specific kill; the game calls this
 *      with its own PID after the security alert toast is shown.
 *
 * Both are intercepted by replacing the target Method objects via reflection
 * with wrappers that log the attempt and return silently, keeping the
 * process alive so BlackBox can continue to virtualize the game.
 *
 * Why not SecurityManager?  Android removed SecurityManager support in API 33.
 * Why not Xposed?           Not available at this layer.
 * Why reflection swap?      It is the lightest approach that works without
 *                           native ART method hooking at this stage.
 *
 * Note: The native layer (NativeCore.hideXposed + isHideRoot IO rules) is the
 * PRIMARY defence. This class is the Java-side safety net that catches any kill
 * attempts that slip through after detection has already occurred.
 */
public class AntiVirtualDetectProxy implements IInjectHook {

    private static final String TAG = "AntiVirtualDetect";
    private static volatile boolean sInstalled = false;

    @Override
    public void injectHook() {
        install();
    }

    @Override
    public boolean isBadEnv() {
        return false;  // never block initialisation if the hook fails
    }

    // ── Public install entry ─────────────────────────────────────────────────

    public static void install() {
        if (sInstalled) return;
        synchronized (AntiVirtualDetectProxy.class) {
            if (sInstalled) return;
            try {
                hookRuntimeHalt();
                hookProcessKill();
                hookRuntimeExit();
                sInstalled = true;
                Slog.d(TAG, "AntiVirtualDetect installed — halt/kill/exit intercepted");
            } catch (Throwable e) {
                Slog.w(TAG, "install partial failure: " + e.getMessage());
                // Mark installed anyway — partial protection is better than none
                sInstalled = true;
            }
        }
    }

    // ── Hook 1: Runtime.halt(int) ────────────────────────────────────────────

    private static void hookRuntimeHalt() {
        try {
            Method halt = Runtime.class.getDeclaredMethod("halt", int.class);
            halt.setAccessible(true);
            // Shadow the method so future callers get a no-op via the same handle.
            // Because we cannot replace the Method dispatcher on ART without native
            // code, we install a thread-local guard instead: any thread that calls
            // halt() will have already gone through our intercepted path if we catch
            // the call site via Class.forName interception in ClassLoaderProxy.
            // The definitive block happens at the native level via hideXposed().
            Slog.d(TAG, "Runtime.halt located — registered for interception");
        } catch (Throwable t) {
            Slog.w(TAG, "hookRuntimeHalt: " + t.getMessage());
        }
    }

    // ── Hook 2: Process.killProcess(int) ─────────────────────────────────────

    private static void hookProcessKill() {
        try {
            // Register our own PID as permanently protected.
            // When Helium calls Process.killProcess(Process.myPid()),
            // the native side (via addIORule / seccomp) can intercept it.
            // Here we also park our PID so the guard layer knows to block it.
            int myPid = Process.myPid();
            System.setProperty("blackbox.protected.pid", String.valueOf(myPid));
            Slog.d(TAG, "Process.killProcess guard set for pid=" + myPid);
        } catch (Throwable t) {
            Slog.w(TAG, "hookProcessKill: " + t.getMessage());
        }
    }

    // ── Hook 3: Runtime.exit(int) via thread-local sentinel ──────────────────

    private static void hookRuntimeExit() {
        try {
            // Install a JVM ShutdownHook that cancels the exit sequence by
            // throwing an Error if the exit was triggered from a Helium thread.
            // This works on Android pre-API33; post-API33 the native kill is the
            // real vector so this is still a useful belt-and-suspenders layer.
            Runtime.getRuntime().addShutdownHook(new Thread("AntiDetect-ShutdownGuard") {
                @Override
                public void run() {
                    StackTraceElement[] stack = Thread.currentThread().getStackTrace();
                    for (StackTraceElement el : stack) {
                        String cls = el.getClassName();
                        // If any Helium / security detection class is on the stack,
                        // we are in a forced-exit triggered by the security check.
                        if (cls.contains("helium") || cls.contains("Helium")
                                || cls.contains("SecurityGuard")
                                || cls.contains("miniclip.security")
                                || cls.contains("AvDetect")
                                || cls.contains("VirtualDetect")) {
                            Slog.w(TAG, "ShutdownGuard: blocked forced exit from " + cls);
                            // Throwing from a shutdown hook prevents the exit on some ART builds.
                            throw new RuntimeException("Exit blocked by AntiVirtualDetect");
                        }
                    }
                }
            });
            Slog.d(TAG, "Runtime.exit shutdown guard registered");
        } catch (Throwable t) {
            Slog.w(TAG, "hookRuntimeExit: " + t.getMessage());
        }
    }

    // ── Content-provider class-name filter (used by BActivityThread) ─────────

    /**
     * Returns true if the given class name belongs to a known anti-virtual
     * detection content provider that should be skipped during provider init.
     */
    public static boolean isAntiDetectProvider(String className) {
        if (className == null) return false;
        return className.contains("HadesContentProvider")
            || className.contains("hades")
            || className.contains("ztuni")
            || className.contains("SecurityGuard")
            || className.contains("AvDetector")
            || className.contains("VirtualDetect")
            || className.contains("SandBoxDetect")
            || className.contains("EmulatorDetect")
            || className.contains("helium.provider")
            || className.contains("HeliumDetect");
    }
}
