// ILangSuInterface.aidl
package com.langsu.manager;

import android.content.pm.PackageInfo;
import rikka.parcelablelist.ParcelableListSlice;

interface ILangSuInterface {
    ParcelableListSlice<PackageInfo> getPackages(int flags);

    int[] getUserIds();
}