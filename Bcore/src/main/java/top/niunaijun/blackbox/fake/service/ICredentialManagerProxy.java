package top.niunaijun.blackbox.fake.service;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Binder;
import android.os.IBinder;
import black.android.os.BRServiceManager;
import top.niunaijun.blackbox.BlackBoxCore;
import top.niunaijun.blackbox.app.GoogleSignInHelper;
import top.niunaijun.blackbox.fake.hook.BinderInvocationStub;
import top.niunaijun.blackbox.fake.hook.MethodHook;
import top.niunaijun.blackbox.fake.hook.ProxyMethod;
import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;

/* JADX INFO: loaded from: classes3.dex */
public class ICredentialManagerProxy extends BinderInvocationStub {
    private static final String SERVICE_NAME = "credential";
    public static final String TAG = "PlayIntegrity";
    private static volatile Object sPendingCallback;
    private static volatile BroadcastReceiver sTokenReceiver;

    @Override // top.niunaijun.blackbox.fake.hook.IInjectHook
    public boolean isBadEnv() {
        return false;
    }

    @ProxyMethod("executeGetCredential")
    public static class ExecuteGetCredential extends MethodHook {
        @Override // top.niunaijun.blackbox.fake.hook.MethodHook
        public Object hook(Object obj, Method method, Object[] objArr) throws Throwable {
            Object obj2 = objArr[1];
            String cachedIdToken = GoogleSignInHelper.getCachedIdToken(BlackBoxCore.getContext());
            if (cachedIdToken != null) {
                try {
                    GoogleSignInHelper.deliverTokenViaCallback(obj2, cachedIdToken);
                    return ICredentialManagerProxy.createDummyCancellationSignal();
                } catch (Exception unused) {
                }
            }
            Object unused2 = ICredentialManagerProxy.sPendingCallback = obj2;
            ICredentialManagerProxy.registerTokenReceiver();
            GoogleSignInHelper.launchWebViewSignIn(BlackBoxCore.getContext());
            return ICredentialManagerProxy.createDummyCancellationSignal();
        }
    }

    public ICredentialManagerProxy() {
        super(BRServiceManager.get().getService(SERVICE_NAME));
    }

    /* JADX INFO: Access modifiers changed from: private */
    public static Object createDummyCancellationSignal() {
        try {
            Class<?> cls = Class.forName("android.os.ICancellationSignal");
            return Proxy.newProxyInstance(cls.getClassLoader(), new Class[]{cls}, new InvocationHandler() { // from class: top.niunaijun.blackbox.fake.service.ICredentialManagerProxy.1
                /* JADX WARN: Failed to restore switch over string. Please report as a decompilation issue */
                @Override // java.lang.reflect.InvocationHandler
                public Object invoke(Object obj, Method method, Object[] objArr) {
                    String name = method.getName();
                    name.hashCode();
                    byte b = -1;
                    switch (name.hashCode()) {
                        case -1776922004:
                            if (name.equals("toString")) {
                                b = 0;
                            }
                            break;
                        case -1772511108:
                            if (name.equals("asBinder")) {
                                b = 1;
                            }
                            break;
                        case -1295482945:
                            if (name.equals("equals")) {
                                b = 2;
                            }
                            break;
                        case 147696667:
                            if (name.equals("hashCode")) {
                                b = 3;
                            }
                            break;
                    }
                    Object obj2 = null;
                    switch (b) {
                        case 0:
                            return "DummyCancellationSignal";
                        case 1:
                            return new Binder();
                        case 2:
                            if (objArr != null && objArr.length > 0) {
                                obj2 = objArr[0];
                            }
                            return Boolean.valueOf(obj == obj2);
                        case 3:
                            return Integer.valueOf(System.identityHashCode(obj));
                        default:
                            return null;
                    }
                }
            });
        } catch (Exception unused) {
            return null;
        }
    }

    /* JADX INFO: Access modifiers changed from: private */
    public static void registerTokenReceiver() {
        Context context = BlackBoxCore.getContext();
        BroadcastReceiver broadcastReceiver = sTokenReceiver;
        if (broadcastReceiver != null) {
            try {
                context.unregisterReceiver(broadcastReceiver);
            } catch (Exception unused) {
            }
        }
        sTokenReceiver = new BroadcastReceiver() { // from class: top.niunaijun.blackbox.fake.service.ICredentialManagerProxy.2
            @Override // android.content.BroadcastReceiver
            public void onReceive(Context context2, Intent intent) {
                Object obj;
                String action = intent.getAction();
                if (GoogleSignInHelper.ACTION_GSI_TOKEN.equals(action)) {
                    String stringExtra = intent.getStringExtra("id_token");
                    Object obj2 = ICredentialManagerProxy.sPendingCallback;
                    if (stringExtra != null && obj2 != null) {
                        try {
                            GoogleSignInHelper.deliverTokenViaCallback(obj2, stringExtra);
                        } catch (Exception unused2) {
                        }
                    }
                } else if (GoogleSignInHelper.ACTION_GSI_CANCEL.equals(action) && (obj = ICredentialManagerProxy.sPendingCallback) != null) {
                    GoogleSignInHelper.deliverError(obj, "USER_CANCELED", "Cancelled");
                }
                Object unused3 = ICredentialManagerProxy.sPendingCallback = null;
                try {
                    context2.unregisterReceiver(this);
                } catch (Exception unused4) {
                }
                BroadcastReceiver unused5 = ICredentialManagerProxy.sTokenReceiver = null;
            }
        };
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(GoogleSignInHelper.ACTION_GSI_TOKEN);
        intentFilter.addAction(GoogleSignInHelper.ACTION_GSI_CANCEL);
        context.registerReceiver(sTokenReceiver, intentFilter, 4);
    }

    @Override // top.niunaijun.blackbox.fake.hook.ClassInvocationStub
    public Object getWho() {
        IBinder service = BRServiceManager.get().getService(SERVICE_NAME);
        if (service == null) {
            return null;
        }
        try {
            return Class.forName("android.credentials.ICredentialManager$Stub").getMethod("asInterface", IBinder.class).invoke(null, service);
        } catch (Exception unused) {
            return null;
        }
    }

    @Override // top.niunaijun.blackbox.fake.hook.ClassInvocationStub
    public void inject(Object obj, Object obj2) {
        replaceSystemService(SERVICE_NAME);
    }
}
