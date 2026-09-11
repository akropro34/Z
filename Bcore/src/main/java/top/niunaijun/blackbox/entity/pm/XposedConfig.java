package top.niunaijun.blackbox.entity.pm;

import android.os.Parcel;
import android.os.Parcelable;
import java.util.HashMap;
import java.util.Map;

/* JADX INFO: loaded from: classes3.dex */
public class XposedConfig implements Parcelable {
    public static final Parcelable.Creator<XposedConfig> CREATOR = new Parcelable.Creator<XposedConfig>() { // from class: top.niunaijun.blackbox.entity.pm.XposedConfig.1
        /* JADX WARN: Can't rename method to resolve collision */
        @Override // android.os.Parcelable.Creator
        public XposedConfig createFromParcel(Parcel parcel) {
            return new XposedConfig(parcel);
        }

        /* JADX WARN: Can't rename method to resolve collision */
        @Override // android.os.Parcelable.Creator
        public XposedConfig[] newArray(int i) {
            return new XposedConfig[i];
        }
    };
    public boolean enable;
    public Map<String, Boolean> moduleState;

    @Override // android.os.Parcelable
    public int describeContents() {
        return 0;
    }

    @Override // android.os.Parcelable
    public void writeToParcel(Parcel parcel, int i) {
        parcel.writeByte(this.enable ? (byte) 1 : (byte) 0);
        parcel.writeInt(this.moduleState.size());
        for (Map.Entry<String, Boolean> entry : this.moduleState.entrySet()) {
            parcel.writeString(entry.getKey());
            parcel.writeValue(entry.getValue());
        }
    }

    public XposedConfig() {
        this.moduleState = new HashMap();
    }

    public XposedConfig(Parcel parcel) {
        this.moduleState = new HashMap();
        this.enable = parcel.readByte() != 0;
        int i = parcel.readInt();
        this.moduleState = new HashMap(i);
        for (int i2 = 0; i2 < i; i2++) {
            this.moduleState.put(parcel.readString(), (Boolean) parcel.readValue(Boolean.class.getClassLoader()));
        }
    }
}
