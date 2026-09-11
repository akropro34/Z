package top.niunaijun.blackbox.fake.service.vivo;

import android.os.Process;
import black.android.os.BRServiceManager;
import black.model.vivo.BRIVivoPermissionServiceStub;
import top.niunaijun.blackbox.BlackBoxCore;
import top.niunaijun.blackbox.fake.hook.BinderInvocationStub;
import top.niunaijun.blackbox.fake.hook.MethodHook;
import top.niunaijun.blackbox.fake.hook.ProxyMethod;
import top.niunaijun.blackbox.utils.MethodParameterUtils;
import java.lang.reflect.Method;

/* JADX INFO: loaded from: classes3.dex */
public class IVivoPermissionServiceProxy extends BinderInvocationStub {
    @Override // top.niunaijun.blackbox.fake.hook.IInjectHook
    public boolean isBadEnv() {
        return false;
    }

    public IVivoPermissionServiceProxy() {
        super(BRServiceManager.get().getService("vivo_permission_service"));
    }

    @Override // top.niunaijun.blackbox.fake.hook.ClassInvocationStub
    protected Object getWho() {
        return BRIVivoPermissionServiceStub.get().asInterface(BRServiceManager.get().getService("vivo_permission_service"));
    }

    @Override // top.niunaijun.blackbox.fake.hook.ClassInvocationStub
    protected void inject(Object obj, Object obj2) {
        replaceSystemService("vivo_permission_service");
    }

    @ProxyMethod("checkPermission")
    public static class checkPermission extends MethodHook {
        @Override // top.niunaijun.blackbox.fake.hook.MethodHook
        protected Object hook(Object obj, Method method, Object[] objArr) throws Throwable {
            if (((Integer) objArr[2]).intValue() == Process.myUid()) {
                objArr[2] = Integer.valueOf(BlackBoxCore.getHostUid());
            }
            return method.invoke(obj, objArr);
        }
    }

    @ProxyMethod("getAppPermission")
    public static class getAppPermission extends MethodHook {
        @Override // top.niunaijun.blackbox.fake.hook.MethodHook
        protected Object hook(Object obj, Method method, Object[] objArr) throws Throwable {
            MethodParameterUtils.replaceFirstAppPkg(objArr);
            return method.invoke(obj, objArr);
        }
    }

    @ProxyMethod("setAppPermission")
    public static class setAppPermission extends MethodHook {
        @Override // top.niunaijun.blackbox.fake.hook.MethodHook
        protected Object hook(Object obj, Method method, Object[] objArr) throws Throwable {
            MethodParameterUtils.replaceFirstAppPkg(objArr);
            return method.invoke(obj, objArr);
        }
    }

    @ProxyMethod("setWhiteListApp")
    public static class setWhiteListApp extends MethodHook {
        @Override // top.niunaijun.blackbox.fake.hook.MethodHook
        protected Object hook(Object obj, Method method, Object[] objArr) throws Throwable {
            MethodParameterUtils.replaceFirstAppPkg(objArr);
            return method.invoke(obj, objArr);
        }
    }

    @ProxyMethod("setBlackListApp")
    public static class setBlackListApp extends MethodHook {
        @Override // top.niunaijun.blackbox.fake.hook.MethodHook
        protected Object hook(Object obj, Method method, Object[] objArr) throws Throwable {
            MethodParameterUtils.replaceFirstAppPkg(objArr);
            return method.invoke(obj, objArr);
        }
    }

    @ProxyMethod("noteStartActivityProcess")
    public static class noteStartActivityProcess extends MethodHook {
        @Override // top.niunaijun.blackbox.fake.hook.MethodHook
        protected Object hook(Object obj, Method method, Object[] objArr) throws Throwable {
            MethodParameterUtils.replaceFirstAppPkg(objArr);
            return method.invoke(obj, objArr);
        }
    }

    @ProxyMethod("isBuildInThirdPartApp")
    public static class isBuildInThirdPartApp extends MethodHook {
        @Override // top.niunaijun.blackbox.fake.hook.MethodHook
        protected Object hook(Object obj, Method method, Object[] objArr) throws Throwable {
            MethodParameterUtils.replaceFirstAppPkg(objArr);
            return method.invoke(obj, objArr);
        }
    }

    @ProxyMethod("checkDelete")
    public static class checkDelete extends MethodHook {
        @Override // top.niunaijun.blackbox.fake.hook.MethodHook
        protected Object hook(Object obj, Method method, Object[] objArr) throws Throwable {
            if (objArr[1] instanceof String) {
                objArr[1] = BlackBoxCore.getHostPkg();
            }
            MethodParameterUtils.replaceLastUserId(objArr);
            return method.invoke(obj, objArr);
        }
    }

    @ProxyMethod("setOnePermission")
    public static class setOnePermission extends MethodHook {
        @Override // top.niunaijun.blackbox.fake.hook.MethodHook
        protected Object hook(Object obj, Method method, Object[] objArr) throws Throwable {
            MethodParameterUtils.replaceLastUserId(objArr);
            MethodParameterUtils.replaceFirstAppPkg(objArr);
            return method.invoke(obj, objArr);
        }
    }

    @ProxyMethod("setOnePermissionExt")
    public static class setOnePermissionExt extends MethodHook {
        @Override // top.niunaijun.blackbox.fake.hook.MethodHook
        protected Object hook(Object obj, Method method, Object[] objArr) throws Throwable {
            MethodParameterUtils.replaceLastUserId(objArr);
            MethodParameterUtils.replaceFirstAppPkg(objArr);
            return method.invoke(obj, objArr);
        }
    }

    @ProxyMethod("isVivoImeiPkg")
    public static class isVivoImeiPkg extends MethodHook {
        @Override // top.niunaijun.blackbox.fake.hook.MethodHook
        protected Object hook(Object obj, Method method, Object[] objArr) throws Throwable {
            MethodParameterUtils.replaceFirstAppPkg(objArr);
            return method.invoke(obj, objArr);
        }
    }
}
