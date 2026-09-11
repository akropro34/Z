package top.niunaijun.blackbox.fake.hook;

import black.android.content.BRAttributionSource;
import top.niunaijun.blackbox.BlackBoxCore;
import top.niunaijun.blackbox.utils.compat.BuildCompat;
import top.niunaijun.blackbox.utils.compat.ContextCompat;
import java.lang.reflect.Method;

/* JADX INFO: loaded from: classes3.dex */
public class ReplacePackageNameMethodHook extends MethodHook {
    private int packageNameIndex;

    public ReplacePackageNameMethodHook(int i) {
        this.packageNameIndex = i;
    }

    @Override // top.niunaijun.blackbox.fake.hook.MethodHook
    public Object hook(Object obj, Method method, Object[] objArr) throws Throwable {
        Object obj2;
        Class realClass;
        if (objArr != null) {
            int i = this.packageNameIndex;
            if (i < 0) {
                this.packageNameIndex = i + objArr.length;
            }
            int i2 = this.packageNameIndex;
            if (i2 >= 0 && i2 < objArr.length && (obj2 = objArr[i2]) != null) {
                if (obj2 instanceof String) {
                    objArr[i2] = BlackBoxCore.getHostPkg();
                } else if (BuildCompat.isS() && (realClass = BRAttributionSource.getRealClass()) != null && realClass.isInstance(objArr[this.packageNameIndex])) {
                    ContextCompat.fixAttributionSourceState(objArr[this.packageNameIndex], BlackBoxCore.getHostUid());
                }
            }
        }
        return method.invoke(obj, objArr);
    }
}
