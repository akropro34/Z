# ── AKRO LOADER ProGuard Rules ──────────────────────────────────────────────

# 1. Aggressive optimization
-optimizationpasses 5
-allowaccessmodification
-overloadaggressively
-repackageclasses ''

# 2. Keep annotations alive (black-reflection NEEDS these at runtime)
-keepattributes *Annotation*
-keepattributes *,!Signature,!InnerClasses,!EnclosingMethod,!SourceFile,!LineNumberTable

# 3. Kill logs
-assumenosideeffects class android.util.Log {
    public static *** d(...);
    public static *** v(...);
    public static *** i(...);
    public static *** w(...);
    public static *** e(...);
}

# 4. Keep our main app classes
-keep public class com.fs4ip.delta.** { *; }

# 5. JNI protection
-keepclasseswithmembernames class * {
    native <methods>;
}

# 6. BlackBox / black-reflection — MUST survive ProGuard
-keep class top.niunaijun.blackbox.** { *; }
-keep class top.niunaijun.jnihook.** { *; }
-keep class top.niunaijun.blackreflection.** { *; }
-keep class mirror.** { *; }
-keep class android.** { *; }
-keep class com.android.** { *; }

# 7. Keep ALL @BClass and @BClassName annotated classes WITH their members
#    (this is what BlackBoxCore scans for at startup)
-keep @top.niunaijun.blackreflection.annotation.BClass class * { *; }
-keep @top.niunaijun.blackreflection.annotation.BClassName class * { *; }
-keep @top.niunaijun.blackreflection.annotation.BClassNameNotProcess class * { *; }

-keepclasseswithmembernames class * {
    @top.niunaijun.blackreflection.annotation.BField* <methods>;
    @top.niunaijun.blackreflection.annotation.BFieldNotProcess* <methods>;
    @top.niunaijun.blackreflection.annotation.BFieldSetNotProcess* <methods>;
    @top.niunaijun.blackreflection.annotation.BFieldCheckNotProcess* <methods>;
    @top.niunaijun.blackreflection.annotation.BMethod* <methods>;
    @top.niunaijun.blackreflection.annotation.BStaticField* <methods>;
    @top.niunaijun.blackreflection.annotation.BStaticMethod* <methods>;
    @top.niunaijun.blackreflection.annotation.BMethodCheckNotProcess* <methods>;
    @top.niunaijun.blackreflection.annotation.BConstructor* <methods>;
    @top.niunaijun.blackreflection.annotation.BConstructorNotProcess* <methods>;
}

# 8. Entry point + BlackBox system services
-keep class top.niunaijun.blackbox.closecode.Entry { *; }
-keep class top.niunaijun.blackbox.core.system.BlackBoxSystemServer { *; }
-keep class top.niunaijun.blackbox.core.BlackBoxContentProvider { *; }

# 9. AIDL interfaces
-keep interface top.niunaijun.blackbox.** { *; }
