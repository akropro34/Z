package top.niunaijun.blackbox.fake.service;

import black.android.app.BRILocaleManagerStub;
import black.android.os.BRServiceManager;
import top.niunaijun.blackbox.fake.hook.BinderInvocationStub;
import top.niunaijun.blackbox.fake.service.base.PkgMethodProxy;

/* JADX INFO: loaded from: classes3.dex */
public class ILocaleManagerProxy extends BinderInvocationStub {
    public static final String TAG = "ILocaleManagerProxy";

    @Override // top.niunaijun.blackbox.fake.hook.IInjectHook
    public boolean isBadEnv() {
        return false;
    }

    public ILocaleManagerProxy() {
        super(BRServiceManager.get().getService("locale"));
    }

    @Override // top.niunaijun.blackbox.fake.hook.BinderInvocationStub, top.niunaijun.blackbox.fake.hook.ClassInvocationStub
    protected void onBindMethod() {
        super.onBindMethod();
        addMethodHook(new PkgMethodProxy("setApplicationLocales"));
        addMethodHook(new PkgMethodProxy("getApplicationLocales"));
    }

    @Override // top.niunaijun.blackbox.fake.hook.ClassInvocationStub
    protected Object getWho() {
        return BRILocaleManagerStub.get().asInterface(BRServiceManager.get().getService("locale"));
    }

    @Override // top.niunaijun.blackbox.fake.hook.ClassInvocationStub
    protected void inject(Object obj, Object obj2) {
        replaceSystemService("locale");
    }
}
