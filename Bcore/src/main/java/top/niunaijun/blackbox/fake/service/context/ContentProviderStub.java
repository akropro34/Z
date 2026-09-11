package top.niunaijun.blackbox.fake.service.context.providers;

import android.os.IInterface;
import black.android.content.BRAttributionSource;
import top.niunaijun.blackbox.BlackBoxCore;
import top.niunaijun.blackbox.fake.hook.ClassInvocationStub;
import top.niunaijun.blackbox.utils.compat.ContextCompat;
import java.lang.reflect.Method;

/* JADX INFO: loaded from: classes3.dex */
public class ContentProviderStub extends ClassInvocationStub implements BContentProvider {
    public static final String TAG = "ContentProviderStub";
    private String mAppPkg;
    private IInterface mBase;

    @Override // top.niunaijun.blackbox.fake.hook.ClassInvocationStub
    protected void inject(Object obj, Object obj2) {
    }

    @Override // top.niunaijun.blackbox.fake.hook.IInjectHook
    public boolean isBadEnv() {
        return false;
    }

    @Override // top.niunaijun.blackbox.fake.hook.ClassInvocationStub
    protected void onBindMethod() {
    }

    @Override // top.niunaijun.blackbox.fake.service.context.providers.BContentProvider
    public IInterface wrapper(IInterface iInterface, String str) {
        this.mBase = iInterface;
        this.mAppPkg = str;
        injectHook();
        return (IInterface) getProxyInvocation();
    }

    @Override // top.niunaijun.blackbox.fake.hook.ClassInvocationStub
    protected Object getWho() {
        return this.mBase;
    }

    @Override // top.niunaijun.blackbox.fake.hook.ClassInvocationStub, java.lang.reflect.InvocationHandler
    public Object invoke(Object obj, Method method, Object[] objArr) throws Throwable {
        if ("asBinder".equals(method.getName())) {
            return method.invoke(this.mBase, objArr);
        }
        if (objArr != null && objArr.length > 0) {
            Object obj2 = objArr[0];
            if (obj2 instanceof String) {
                objArr[0] = this.mAppPkg;
            } else if (obj2.getClass().getName().equals(BRAttributionSource.getRealClass().getName())) {
                ContextCompat.fixAttributionSourceState(obj2, BlackBoxCore.getHostUid());
            }
        }
        try {
            return method.invoke(this.mBase, objArr);
        } catch (Throwable th) {
            throw th.getCause();
        }
    }
}
