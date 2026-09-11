package top.niunaijun.blackbox.entity.pm;

import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.os.Parcel;
import android.os.Parcelable;
import top.niunaijun.blackbox.BlackBoxCore;

/* JADX INFO: loaded from: classes3.dex */
public class InstalledModule implements Parcelable {
    public static final Parcelable.Creator<InstalledModule> CREATOR = new Parcelable.Creator<InstalledModule>() { // from class: top.niunaijun.blackbox.entity.pm.InstalledModule.1
        /* JADX WARN: Can't rename method to resolve collision */
        @Override // android.os.Parcelable.Creator
        public InstalledModule createFromParcel(Parcel parcel) {
            return new InstalledModule(parcel);
        }

        /* JADX WARN: Can't rename method to resolve collision */
        @Override // android.os.Parcelable.Creator
        public InstalledModule[] newArray(int i) {
            return new InstalledModule[i];
        }
    };
    public String desc;
    public boolean enable;
    public String main;
    public String name;
    public String packageName;

    @Override // android.os.Parcelable
    public int describeContents() {
        return 0;
    }

    public InstalledModule() {
    }

    public ApplicationInfo getApplication() {
        return BlackBoxCore.getBPackageManager().getApplicationInfo(this.packageName, 128, -4);
    }

    public PackageInfo getPackageInfo() {
        return BlackBoxCore.getBPackageManager().getPackageInfo(this.packageName, 128, -4);
    }

    @Override // android.os.Parcelable
    public void writeToParcel(Parcel parcel, int i) {
        parcel.writeString(this.packageName);
        parcel.writeString(this.name);
        parcel.writeString(this.desc);
        parcel.writeString(this.main);
        parcel.writeByte(this.enable ? (byte) 1 : (byte) 0);
    }

    protected InstalledModule(Parcel parcel) {
        this.packageName = parcel.readString();
        this.name = parcel.readString();
        this.desc = parcel.readString();
        this.main = parcel.readString();
        this.enable = parcel.readByte() != 0;
    }
}
