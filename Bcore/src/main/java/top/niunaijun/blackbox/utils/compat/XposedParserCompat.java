package top.niunaijun.blackbox.utils.compat;

import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import top.niunaijun.blackbox.BlackBoxCore;
import top.niunaijun.blackbox.entity.pm.InstalledModule;
import top.niunaijun.blackbox.utils.CloseUtils;
import top.niunaijun.blackbox.utils.ShellUtils;
import java.io.BufferedReader;
import java.io.Closeable;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

/* JADX INFO: loaded from: classes3.dex */
public class XposedParserCompat {
    public static InstalledModule parseModule(ApplicationInfo applicationInfo) {
        try {
            PackageManager packageManager = BlackBoxCore.getPackageManager();
            InstalledModule installedModule = new InstalledModule();
            installedModule.packageName = applicationInfo.packageName;
            installedModule.enable = false;
            installedModule.desc = applicationInfo.metaData.getString("xposeddescription");
            installedModule.name = applicationInfo.loadLabel(packageManager).toString();
            installedModule.main = readMain(applicationInfo.sourceDir);
            return installedModule;
        } catch (RuntimeException unused) {
            return null;
        }
    }

    public static boolean isXPModule(String str) {
        return readMain(str) != null;
    }

    /* JADX WARN: Not initialized variable reg: 3, insn: 0x0041: MOVE (r2 I:??[OBJECT, ARRAY]) = (r3 I:??[OBJECT, ARRAY]), block:B:21:0x0041 */
    private static String readMain(String str) throws Throwable {
        ZipFile zipFile;
        Closeable closeable;
        Closeable closeable2 = null;
        try {
            try {
                zipFile = new ZipFile(new File(str));
                try {
                    ZipEntry entry = zipFile.getEntry("assets/xposed_init");
                    if (entry == null) {
                        throw new RuntimeException();
                    }
                    String strTrim = getInputStreamContent(zipFile.getInputStream(entry)).trim();
                    CloseUtils.close(zipFile);
                    return strTrim;
                } catch (IOException e) {
                    e = e;
                    e.printStackTrace();
                    CloseUtils.close(zipFile);
                    return null;
                }
            } catch (Throwable th) {
                th = th;
                closeable2 = closeable;
                CloseUtils.close(closeable2);
                throw th;
            }
        } catch (IOException e2) {
            e = e2;
            zipFile = null;
        } catch (Throwable th2) {
            th = th2;
            CloseUtils.close(closeable2);
            throw th;
        }
    }

    private static String getInputStreamContent(InputStream inputStream) throws Throwable {
        StringBuilder sb = new StringBuilder();
        BufferedReader bufferedReader = null;
        try {
            try {
                BufferedReader bufferedReader2 = new BufferedReader(new InputStreamReader(inputStream));
                while (true) {
                    try {
                        String line = bufferedReader2.readLine();
                        if (line == null) {
                            break;
                        }
                        if (!line.startsWith("#")) {
                            sb.append(line).append(ShellUtils.COMMAND_LINE_END);
                        }
                    } catch (Exception e) {
                        e = e;
                        bufferedReader = bufferedReader2;
                        e.printStackTrace();
                        CloseUtils.close(bufferedReader);
                    } catch (Throwable th) {
                        th = th;
                        bufferedReader = bufferedReader2;
                        CloseUtils.close(bufferedReader);
                        throw th;
                    }
                }
                CloseUtils.close(bufferedReader2);
            } catch (Exception e2) {
                e = e2;
            }
            return sb.toString();
        } catch (Throwable th2) {
            th = th2;
        }
    }
}
